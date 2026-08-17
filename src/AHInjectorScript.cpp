#include "AHInjectorScript.h"
#include "AHInjectorConfig.h"
#include "AHInjectorMgr.h"
#include "Logging/Log.h"

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