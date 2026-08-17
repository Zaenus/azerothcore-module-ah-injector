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
#include "Utilities/StringFormat.h"

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
    if (items.empty())
    {
        LOG_DEBUG("modules.ahinjector", "AH Injector: No items configured to inject");
        return;
    }

    uint32 itemsPerCycle = sAHInjectorConfig.GetItemsPerCycle();
    uint32 injectedCount = 0;

    for (const auto& item : items)
    {
        if (injectedCount >= itemsPerCycle)
            break;

        if (IsItemListed(item.itemEntry, _injectorGuid))
        {
            LOG_DEBUG("modules.ahinjector", "AH Injector: Item {} already listed by injector, skipping", item.itemEntry);
            continue;
        }

        if (CreateAuction(item))
        {
            ++injectedCount;
            LOG_INFO("modules.ahinjector", "AH Injector: Injected item {} x{} (bid={}, buyout={}, duration={}h)",
                item.itemEntry, item.count, item.minBid, item.buyout, item.durationHours);
        }
    }

    if (injectedCount > 0)
    {
        LOG_INFO("modules.ahinjector", "AH Injector: Injected {} items this cycle", injectedCount);
    }
}

bool AHInjectorMgr::IsItemListed(uint32 itemEntry, ObjectGuid ownerGuid) const
{
    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(AuctionHouseId::Neutral);
    if (!ah)
        return false;

    for (auto const& [id, auction] : ah->GetAuctions())
    {
        if (auction->item_template == itemEntry && auction->owner == ownerGuid)
            return true;
    }
    return false;
}
bool AHInjectorMgr::CreateAuction(const InjectedItem& item)
{
    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(AuctionHouseId::Neutral);
    if (!ah)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Failed to get neutral auction house");
        return false;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemEntry);
    if (!proto)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Item template not found for entry {}", item.itemEntry);
        return false;
    }

    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntryFromHouse(AuctionHouseId::Neutral);
    if (!ahEntry)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Auction house entry not found for neutral AH");
        return false;
    }

    // Create a virtual item for the auction (temp=false to get a proper GUID)
    Item* virtualItem = Item::CreateItem(item.itemEntry, item.count, nullptr, false, 0, false);
    if (!virtualItem)
    {
        LOG_ERROR("modules.ahinjector", "AH Injector: Failed to create virtual item for entry {}", item.itemEntry);
        return false;
    }

    // Set owner GUID so it's saved correctly in item_instance
    virtualItem->SetOwnerGUID(_injectorGuid);

    uint32 durationSeconds = item.durationHours * 3600;
    uint64 deposit = CalculateDeposit(item.itemEntry, item.count, item.durationHours);

    AuctionEntry* auction = new AuctionEntry();
    auction->Id = sObjectMgr->GenerateAuctionID();
    auction->houseId = AuctionHouseId::Neutral;
    auction->item_guid = virtualItem->GetGUID();
    auction->item_template = item.itemEntry;
    auction->itemCount = item.count;
    auction->owner = _injectorGuid;
    auction->startbid = static_cast<uint32>(item.minBid);
    auction->bidder = ObjectGuid::Empty;
    auction->bid = 0;
    auction->buyout = static_cast<uint32>(item.buyout);
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

    LOG_INFO("modules.ahinjector", "AH Injector: Injected item {} x{} (bid={}, buyout={}, duration={}h)",
        item.itemEntry, item.count, item.minBid, item.buyout, item.durationHours);

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