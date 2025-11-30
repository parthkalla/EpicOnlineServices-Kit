#include "EOSKitPlayerDataStorage.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitPlayerDataStorageModule"

void FEOSKitPlayerDataStorageModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("EOSKitPlayerDataStorage Module Started"));
}

void FEOSKitPlayerDataStorageModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("EOSKitPlayerDataStorage Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitPlayerDataStorageModule, EOSKitPlayerDataStorage)

