// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitSockets.h"
#include "SocketSubsystemEOS.h"
#include "SocketSubsystemModule.h"

#define LOCTEXT_NAMESPACE "FEOSKitSocketsModule"

void FEOSKitSocketsModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitSockets: Module starting up"));

	// Create and register the EOS socket subsystem
	FSocketSubsystemEOS* EOSSocketSubsystem = FSocketSubsystemEOS::Create();
	
	// Register with the socket subsystem module
	FString Error;
	if (EOSSocketSubsystem && EOSSocketSubsystem->Init(Error))
	{
		// Register the subsystem with Unreal's socket system
		FSocketSubsystemModule& SocketSubsystemModule = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
		SocketSubsystemModule.RegisterSocketSubsystem(FName("EOS"), EOSSocketSubsystem, false);
		
		UE_LOG(LogTemp, Log, TEXT("EOSKitSockets: Socket subsystem registered successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitSockets: Failed to initialize socket subsystem - %s"), *Error);
	}
}

void FEOSKitSocketsModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitSockets: Module shutting down"));

	// Unregister the socket subsystem
	FSocketSubsystemModule& SocketSubsystemModule = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
	SocketSubsystemModule.UnregisterSocketSubsystem(FName("EOS"));
	
	// Destroy the socket subsystem
	FSocketSubsystemEOS::Destroy();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSKitSocketsModule, EOSKitSockets)
