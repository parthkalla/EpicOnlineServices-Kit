// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEpicOnlineServicesKitV2, Log, All);
class FEpicOnlineServicesKitV2Module : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    void ConfigureOnlineSubsystemEOK() const;
    static void ConfigureDedicatedServerConfigEOK();
    virtual void ShutdownModule() override;
};
