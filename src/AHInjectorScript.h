#ifndef MOD_AH_INJECTOR_SCRIPT_H_
#define MOD_AH_INJECTOR_SCRIPT_H_

#include "ScriptDefines/AuctionHouseScript.h"
#include "ScriptDefines/WorldScript.h"
#include "AHInjectorConfig.h"

class AHInjectorWorldScript : public WorldScript
{
public:
    AHInjectorWorldScript();

    void OnUpdate(uint32 diff) override;
};

class AHInjectorConfigScript : public WorldScript
{
public:
    AHInjectorConfigScript();

    void OnBeforeConfigLoad(bool reload) override;
};

class AHInjectorAuctionScript : public AuctionHouseScript
{
public:
    AHInjectorAuctionScript();

    void OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnBeforeAuctionHouseMgrSendAuctionExpiredMail(AuctionHouseMgr* auctionHouseMgr, AuctionEntry* auction, Player* owner, uint32& owner_accId, bool& sendNotification, bool& sendMail) override;
};

#endif