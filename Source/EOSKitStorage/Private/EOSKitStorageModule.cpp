#include "EOSKitStorage.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitStorageModule"

void FEOSKitStorageModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitStorage: Module started"));
}

void FEOSKitStorageModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitStorage: Module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitStorageModule, EOSKitStorage)


