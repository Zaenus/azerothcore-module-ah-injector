#ifndef MOD_AH_INJECTOR_CONFIG_H_
#define MOD_AH_INJECTOR_CONFIG_H_

#include "Common.h"
#include <string>
#include <vector>

struct InjectedItem
{
    uint32 itemEntry = 0;
    uint32 count = 1;
    uint64 minBid = 0;
    uint64 buyout = 0;
    uint32 durationHours = 48;
};

class AHInjectorConfig
{
public:
    static AHInjectorConfig& Instance()
    {
        static AHInjectorConfig instance;
        return instance;
    }

    void Initialize(bool reload = false);

    bool IsEnabled() const { return _enabled; }
    uint32 GetUpdateInterval() const { return _updateInterval; }
    uint32 GetItemsPerCycle() const { return _itemsPerCycle; }
    const std::vector<InjectedItem>& GetItems() const { return _items; }
    const std::string& GetInjectorAccount() const { return _injectorAccount; }
    uint32 GetInjectorGuid() const { return _injectorGuid; }
    const std::string& GetInjectorName() const { return _injectorName; }

private:
    AHInjectorConfig() = default;
    ~AHInjectorConfig() = default;

    AHInjectorConfig(const AHInjectorConfig&) = delete;
    AHInjectorConfig& operator=(const AHInjectorConfig&) = delete;

    std::vector<InjectedItem> ParseItemList(const std::string& listStr) const;

    // Config values
    bool _enabled = true;
    uint32 _updateInterval = 300000;
    uint32 _itemsPerCycle = 10;
    std::string _injectorAccount = "AHInjector";
    uint32 _injectorGuid = 1002000;
    std::string _injectorName = "AHInjector";
    std::vector<InjectedItem> _items;
};

#define sAHInjectorConfig AHInjectorConfig::Instance()

#endif