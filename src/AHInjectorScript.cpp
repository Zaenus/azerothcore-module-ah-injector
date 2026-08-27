#include "AHInjectorScript.h"
#include "AHInjectorConfig.h"
#include "AHInjectorMgr.h"
#include "AuctionHouseMgr.h"
#include "CharacterDatabase.h"
#include "Item.h"
#include "Logging/Log.h"
#include "ObjectAccessor.h"
#include "Utilities/StringFormat.h"

AHInjectorWorldScript::AHInjectorWorldScript() : WorldScript("AHInjectorWorldScript",
    std::vector<uint16>{WORLDHOOK_ON_UPDATE})
{
}

AHInjectorConfigScript::AHInjectorConfigScript() : WorldScript("AHInjectorConfigScript",
    std::vector<uint16>{WORLDHOOK_ON_BEFORE_CONFIG_LOAD})
{
}

void AHInjectorConfigScript::OnBeforeConfigLoad(bool reload)
{
    sAHInjectorConfig.Initialize(reload);
}

void AHInjectorWorldScript::OnUpdate(uint32 diff)
{
    if (!sAHInjectorConfig.IsEnabled())
        return;

    static uint32 updateTimer = 0;
    updateTimer += diff;

    if (updateTimer >= sAHInjectorConfig.GetUpdateInterval())
    {
        updateTimer = 0;
        sAHInjectorMgr.ProcessCycle();
    }
}

AHInjectorAuctionScript::AHInjectorAuctionScript() : AuctionHouseScript("AHInjectorAuctionScript",
    std::vector<uint16>{AUCTIONHOUSEHOOK_ON_AUCTION_EXPIRE, AUCTIONHOUSEHOOK_ON_BEFORE_AUCTIONHOUSEMGR_SEND_AUCTION_EXPIRED_MAIL})
{
}

void AHInjectorAuctionScript::OnAuctionExpire(AuctionHouseObject* /*ah*/, AuctionEntry* entry)
{
    if (!sAHInjectorConfig.IsEnabled() || !entry)
        return;

    ObjectGuid injGuid = ObjectGuid::Create<HighGuid::Player>(sAHInjectorConfig.GetInjectorGuid());
    if (entry->owner != injGuid)
        return;

    LOG_DEBUG("modules.ahinjector", "AH Injector: Auction #{} (item {} x{}) expired, will be replenished next cycle",
        entry->Id, entry->item_template, entry->itemCount);
}

void AHInjectorAuctionScript::OnBeforeAuctionHouseMgrSendAuctionExpiredMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* auction, Player* /*owner*/, uint32& /*owner_accId*/, bool& /*sendNotification*/, bool& sendMail)
{
    if (!sAHInjectorConfig.IsEnabled() || !auction)
        return;

    ObjectGuid injGuid = ObjectGuid::Create<HighGuid::Player>(sAHInjectorConfig.GetInjectorGuid());
    if (auction->owner != injGuid)
        return;

    // Prevent mail flood for the virtual injector character; items are virtual and will be re-injected via ProcessCycle
    sendMail = false;

    if (Item* item = sAuctionMgr->GetAItem(auction->item_guid))
    {
        sAuctionMgr->RemoveAItem(auction->item_guid);
        delete item;
        CharacterDatabase.Execute(Acore::StringFormat("DELETE FROM item_instance WHERE guid = {}", auction->item_guid.GetCounter()));
    }

    LOG_INFO("modules.ahinjector", "AH Injector: Suppressed expired mail for auction #{} (item {}), cleaned virtual item {}", auction->Id, auction->item_template, auction->item_guid.GetCounter());
}