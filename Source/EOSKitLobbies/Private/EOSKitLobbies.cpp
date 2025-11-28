#include "EOSKitLobbies.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitLobbiesModule"

void FEOSKitLobbiesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("EOSKitLobbies Module Started"));
}

void FEOSKitLobbiesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("EOSKitLobbies Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitLobbiesModule, EOSKitLobbies)
