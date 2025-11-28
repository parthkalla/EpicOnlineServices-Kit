#include "EOSKitAuth.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitAuthModule"

void FEOSKitAuthModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("EOSKitAuth Module Started"));
}

void FEOSKitAuthModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("EOSKitAuth Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitAuthModule, EOSKitAuth)
