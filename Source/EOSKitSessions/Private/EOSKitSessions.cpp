#include "EOSKitSessions.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEOSKitSessionsModule"

void FEOSKitSessionsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("EOSKitSessions Module Started"));
}

void FEOSKitSessionsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("EOSKitSessions Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitSessionsModule, EOSKitSessions)
