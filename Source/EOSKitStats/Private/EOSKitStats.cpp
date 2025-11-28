#include "EOSKitStats.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitStatsModule"

void FEOSKitStatsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("EOSKitStats Module Started"));
}

void FEOSKitStatsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("EOSKitStats Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitStatsModule, EOSKitStats)
