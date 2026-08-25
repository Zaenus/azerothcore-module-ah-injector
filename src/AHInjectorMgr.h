#ifndef MOD_AH_INJECTOR_MGR_H_
#define MOD_AH_INJECTOR_MGR_H_

#include "AHInjectorConfig.h"
#include "Common.h"
#include "AuctionHouseMgr.h"
#include "ObjectGuid.h"
#include <mutex>

class AuctionHouseObject;
class AuctionEntry;
class ItemTemplate;

class AHInjectorMgr
{
public:
    static AHInjectorMgr& Instance()
    {
        static AHInjectorMgr instance;
        return instance;
    }

    void Initialize();
    void ProcessCycle();

private:
    AHInjectorMgr() = default;
    ~AHInjectorMgr() = default;

    AHInjectorMgr(const AHInjectorMgr&) = delete;
    AHInjectorMgr& operator=(const AHInjectorMgr&) = delete;

    uint32 CountListings(uint32 itemEntry, ObjectGuid ownerGuid) const;
    bool CreateAuction(InjectedItem const& item, AuctionHouseId houseId);
    uint64 CalculateDeposit(uint32 itemEntry, uint32 count, uint32 durationHours) const;

    ObjectGuid _injectorGuid;
    bool _initialized = false;
    mutable std::mutex _mutex;
};

#define sAHInjectorMgr AHInjectorMgr::Instance()

#endif