// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitShared.h"

#define LOCTEXT_NAMESPACE "FEOSKitSharedModule"

void FEOSKitSharedModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitShared: Module started"));
}

void FEOSKitSharedModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitShared: Module shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSKitSharedModule, EOSKitShared)
