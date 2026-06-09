// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Runtime/Launch/Resources/Version.h"

#include "EOSSDKManager.h"

class FEOKSharedModule: public IModuleInterface
{
public:
	FEOKSharedModule() = default;
	~FEOKSharedModule() = default;

private:
	// ~Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// ~End IModuleInterface

#if WITH_EOS_SDK
	TUniquePtr<FEOKSDKManager> SDKManager;
#endif
};