#include "EOSKit.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitModule"

void FEOSKitModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("EOSKit Module Started"));
}

void FEOSKitModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("EOSKit Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitModule, EOSKit)
