#include "AHInjectorMgr.h"
#include "AHInjectorConfig.h"
#include "AuctionHouseMgr.h"
#include "CharacterDatabase.h"
#include "GameTime.h"
#include "Item.h"
#include "Logging/Log.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Random.h"
#include "Utilities/StringFormat.h"
#include "World.h"
#include <algorithm>

namespace
{
    AuctionHouseId GetHouseFromIndex(uint8 index)
    {
        switch (index)
        {
            case 0:
                return AuctionHouseId::Alliance;
            case 1:
                return AuctionHouseId::Horde;
            default:
                return AuctionHouseId::Neutral;
        }
    }
}

void AHInjectorMgr::Initialize()
{
    std::lock_guard lock(_mutex);

    if (_initialized)
        return;

    _injectorGuid = ObjectGuid::Create<HighGuid::Player>(sAHInjectorConfig.GetInjectorGuid());
    _initialized = true;

    LOG_INFO("modules.ahinjector", "AH Injector initialized: Injector GUID={}, Name={}",
        _injectorGuid.GetCounter(), sAHInjectorConfig.GetInjectorName());
}

void AHInjectorMgr::ProcessCycle()
{
    if (!sAHInjectorConfig.IsEnabled())
        return;

    std::lock_guard lock(_mutex);

    if (!_initialized)
        Initialize();

    const auto& items = sAHInjectorConfig.GetItems();
    const auto& houses = sAHInjectorConfig.GetTargetHouses();
    if (items.empty() || houses.empty())
    {
        LOG_DEBUG("modules.ahinjector", "AH Injector: No items or target houses configured to inject");
        return;
    }

    uint32 itemsPerCycle = sAHInjectorConfig.GetItemsPerCycle();
    uint32 injectedCount = 0;

    for (const auto& item : items)
    {
        if (injectedCount >= itemsPerCycle)
            break;

        // Allow several concurrent listings of the same entry so the AH
        // doesn't look like a single static shelf of goods
        uint32 activeListings = CountListings(item.itemEntry, _injectorGuid);
        uint32 maxListings = sAHInjectorConfig.GetMaxListingsPerItem();
        if (activeListings >= maxListings)
        {
            LOG_DEBUG("modules.ahinjector", "AH Injector: Item {} already has {} listings, skipping",
                item.itemEntry, activeListings);
            continue;
        }

        for (uint32 i = activeListings; i < maxListings; ++i)
        {
            if (injectedCount >= itemsPerCycle)
                break;

            AuctionHouseId houseId = GetHouseFromIndex(houses[urand(0, houses.size() - 1)]);
            if (CreateAuction(item, houseId))
                ++injectedCount;
        }
    }

    if (injectedCount > 0)
    {
        LOG_INFO("modules.ahinjector", "AH Injector: Injected {} items this cycle", injectedCount);
    }
}

uint32 AHInjectorMgr::CountListings(uint32 itemEntry, ObjectGuid ownerGuid) const
{
    // When cross-faction AH is enabled all three house IDs map to the same neutral object, so counting three times would triple-count.
    if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(AuctionHouseId::Neutral);
        if (!ah)
            return 0;
        uint32 count = 0;
        for (auto const& [id, auction] : ah->GetAuctions())
            if (auction->item_template == itemEntry && auction->owner == ownerGuid)
                ++count;
        return count;
    }

    static const AuctionHouseId houses[3] = { AuctionHouseId::Alliance, AuctionHouseId::Horde, AuctionHouseId::Neutral };
    uint32 count = 0;
    // Deduplicate when GetAuctionsMapByHouseId returns same pointer (defensive)
    std::vector<AuctionHouseObject*> seen;
    for (AuctionHouseId house : houses)
    {
        AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(house);
        if (!ah)
            continue;
        if (std::find(seen.begin(), seen.end(), ah) != seen.end())
            continue;
        seen.push_back(ah);

        for (auto const& [id, auction] : ah->GetAuctions())
        {
            if (auction->item_template == itemEntry && auction->owner == ownerGuid)
                ++count;
        }
    }

    return count;
}
bool AHInjectorMgr::CreateAuction(InjectedItem const& item, AuctionHouseId houseId)
{
    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(houseId);
    if (!ah)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Failed to get auction house {}", static_cast<uint8>(houseId));
        return false;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemEntry);
    if (!proto)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Item template not found for entry {}", item.itemEntry);
        return false;
    }

    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntryFromHouse(houseId);
    if (!ahEntry)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Auction house entry not found for house {}", static_cast<uint8>(houseId));
        return false;
    }

    // Apply random price jitter so listings don't share identical prices
    float variance = sAHInjectorConfig.GetPriceVariancePercent() / 100.0f;
    uint64 minBid = item.minBid;
    uint64 buyout = item.buyout;
    if (variance > 0.0f)
    {
        float factor = frand(1.0f - variance, 1.0f + variance);
        minBid = std::max<uint64>(1, static_cast<uint64>(minBid * factor));
        buyout = std::max<uint64>(minBid, static_cast<uint64>(buyout * factor));
    }

    // Random duration across the standard 12/24/48h AH tiers
    uint32 durationHours = item.durationHours;
    if (sAHInjectorConfig.GetRandomizeDuration())
        durationHours = 12 * (1 << urand(0, 2));

    // Create a virtual item for the auction (temp=false to get a proper GUID)
    Item* virtualItem = Item::CreateItem(item.itemEntry, item.count, nullptr, false, 0, false);
    if (!virtualItem)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Failed to create virtual item for entry {}", item.itemEntry);
        return false;
    }

    // Set owner GUID so it's saved correctly in item_instance
    virtualItem->SetOwnerGUID(_injectorGuid);

    uint32 durationSeconds = durationHours * 3600;
    uint64 deposit = CalculateDeposit(item.itemEntry, item.count, durationHours);

    AuctionEntry* auction = new AuctionEntry();
    auction->Id = sObjectMgr->GenerateAuctionID();
    auction->houseId = houseId;
    auction->item_guid = virtualItem->GetGUID();
    auction->item_template = item.itemEntry;
    auction->itemCount = item.count;
    auction->owner = _injectorGuid;
    auction->startbid = static_cast<uint32>(minBid);
    auction->bidder = ObjectGuid::Empty;
    auction->bid = 0;
    auction->buyout = static_cast<uint32>(buyout);
    auction->expire_time = GameTime::GetGameTime().count() + durationSeconds;
    auction->deposit = static_cast<uint32>(deposit);
    auction->auctionHouseEntry = ahEntry;

    // Add virtual item to auction house manager
    sAuctionMgr->AddAItem(virtualItem);

    // Save virtual item to database so it loads into searcher
    CharacterDatabaseTransaction itemTrans = CharacterDatabase.BeginTransaction();
    virtualItem->SaveToDB(itemTrans);
    CharacterDatabase.CommitTransaction(itemTrans);

    // Add auction to auction house
    ah->AddAuction(auction);

    // Save auction to database
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    auction->SaveToDB(trans);
    CharacterDatabase.CommitTransaction(trans);

    LOG_INFO("modules.ahinjector", "AH Injector: Injected item {} x{} into house {} (bid={}, buyout={}, duration={}h)",
        item.itemEntry, item.count, static_cast<uint8>(houseId), minBid, buyout, durationHours);

    return true;
}

uint64 AHInjectorMgr::CalculateDeposit(uint32 itemEntry, uint32 count, uint32 durationHours) const
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
    if (!proto)
        return 0;

    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntryFromHouse(AuctionHouseId::Neutral);
    if (!ahEntry)
        return 0;

    // Create a temporary item for deposit calculation
    Item* tempItem = Item::CreateItem(itemEntry, count, nullptr, false, 0, true);
    if (!tempItem)
        return 0;

    uint32 duration = durationHours * 60 * 60;
    uint32 deposit = AuctionHouseMgr::GetAuctionDeposit(ahEntry, duration, tempItem, count);

    // Clean up temp item (not added to AH mgr)
    delete tempItem;

    return deposit;
}