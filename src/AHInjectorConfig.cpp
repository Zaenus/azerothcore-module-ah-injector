#include "AHInjectorConfig.h"
#include "Config.h"
#include "Logging/Log.h"
#include "Utilities/StringFormat.h"
#include <sstream>
#include <algorithm>

void AHInjectorConfig::Initialize(bool /*reload*/)
{
    _enabled = sConfigMgr->GetOption<bool>("AHInjector.Enabled", true);
    _updateInterval = sConfigMgr->GetOption<uint32>("AHInjector.UpdateInterval", 300000);
    _itemsPerCycle = sConfigMgr->GetOption<uint32>("AHInjector.ItemsPerCycle", 10);
    _injectorAccount = sConfigMgr->GetOption<std::string>("AHInjector.InjectorAccount", "AHInjector");
    _injectorGuid = sConfigMgr->GetOption<uint32>("AHInjector.InjectorGuid", 1002000);
    _injectorName = sConfigMgr->GetOption<std::string>("AHInjector.InjectorName", "AHInjector");

    std::string itemListStr = sConfigMgr->GetOption<std::string>("AHInjector.ItemList", "");
    _items = ParseItemList(itemListStr);

    LOG_INFO("modules.ahinjector", "AH Injector config loaded: Enabled={}, Interval={}ms, ItemsPerCycle={}, ItemCount={}",
        _enabled, _updateInterval, _itemsPerCycle, _items.size());
}

std::vector<InjectedItem> AHInjectorConfig::ParseItemList(const std::string& listStr) const
{
    std::vector<InjectedItem> items;

    if (listStr.empty())
        return items;

    std::stringstream ss(listStr);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);

        if (token.empty())
            continue;

        // Parse: entry:count:minBid:buyout:durationHours
        std::vector<std::string> parts;
        std::stringstream partStream(token);
        std::string part;

        while (std::getline(partStream, part, ':'))
        {
            parts.push_back(part);
        }

        if (parts.size() != 5)
        {
            LOG_WARN("modules.ahinjector", "Invalid item format in AHInjector.ItemList: '{}' (expected 5 parts)", token);
            continue;
        }

        InjectedItem item;
        try
        {
            item.itemEntry = std::stoul(parts[0]);
            item.count = std::stoul(parts[1]);
            item.minBid = std::stoull(parts[2]);
            item.buyout = std::stoull(parts[3]);
            item.durationHours = std::stoul(parts[4]);

            if (item.itemEntry == 0 || item.count == 0 || item.minBid == 0 || item.buyout == 0 || item.durationHours == 0)
            {
                LOG_WARN("modules.ahinjector", "Invalid values in AHInjector.ItemList: '{}' (all values must be > 0)", token);
                continue;
            }

            items.push_back(item);
        }
        catch (const std::exception& e)
        {
            LOG_WARN("modules.ahinjector", "Failed to parse item in AHInjector.ItemList: '{}' - {}", token, e.what());
            continue;
        }
    }

    return items;
}