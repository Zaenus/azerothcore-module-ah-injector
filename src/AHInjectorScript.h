#ifndef MOD_AH_INJECTOR_SCRIPT_H_
#define MOD_AH_INJECTOR_SCRIPT_H_

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

#endif