// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Online subsystem module class (EOSKit Implementation)
 * Code related to the loading of the EOSKit module
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineSubsystemEOSKitModule : public IModuleInterface
{
private:
	/** Class responsible for creating instance(s) of the subsystem */
	class FOnlineFactoryEOSKit* EOSKitFactory;

public:
	FOnlineSubsystemEOSKitModule() :
		EOSKitFactory(nullptr)
	{}

	virtual ~FOnlineSubsystemEOSKitModule() {}

#if WITH_EDITOR
	void OnPostEngineInit();
	void OnPreExit();
#endif

	// IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}

private:
	void RegisterMenus();
	void ConfigureOnlineSubsystemEOSKit();
};

