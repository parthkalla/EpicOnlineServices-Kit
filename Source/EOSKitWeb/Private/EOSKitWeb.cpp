// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitWeb.h"

#define LOCTEXT_NAMESPACE "FEOSKitWebModule"

void FEOSKitWebModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Module started"));
}

void FEOSKitWebModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Module shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSKitWebModule, EOSKitWeb)
