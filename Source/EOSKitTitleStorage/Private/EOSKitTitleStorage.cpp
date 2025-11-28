// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitTitleStorage.h"

#define LOCTEXT_NAMESPACE "FEOSKitTitleStorageModule"

void FEOSKitTitleStorageModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitTitleStorage: Module started"));
}

void FEOSKitTitleStorageModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitTitleStorage: Module shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSKitTitleStorageModule, EOSKitTitleStorage)
