// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitShared.h"
#include "IEOSSDKManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitSharedModule"

void FEOSKitSharedModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitShared: Module started"));

#if WITH_EOS_SDK
	if (!IEOSSDKManager::Get())
	{
		if (IEOSSDKManager* Manager = EOSKitSDK::CreateSDKManager())
		{
			IEOSSDKManager::Set(Manager);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKitShared: Failed to create EOS SDK manager. EOS platform functionality will be limited."));
		}
	}
#endif
}

void FEOSKitSharedModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitShared: Module shut down"));

#if WITH_EOS_SDK
	if (IEOSSDKManager::Get())
	{
		IEOSSDKManager::Set(nullptr);
	}
	EOSKitSDK::DestroySDKManager();
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSKitSharedModule, EOSKitShared)
