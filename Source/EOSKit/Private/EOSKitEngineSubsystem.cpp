// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitEngineSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Engine/Engine.h"

#if WITH_EDITOR
	#include "Editor.h"
#endif

void UEOSKitEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Engine Subsystem Initialized"));
	
#if WITH_EDITOR
	// Register PIE delegates to handle EOS initialization/cleanup
	if (GEditor)
	{
		FEditorDelegates::BeginPIE.AddUObject(this, &UEOSKitEngineSubsystem::BeginPIE);
		FEditorDelegates::EndPIE.AddUObject(this, &UEOSKitEngineSubsystem::EndPIE);
		
		UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE delegates registered"));
	}
#endif
}

void UEOSKitEngineSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Engine Subsystem Deinitialized"));
	
#if WITH_EDITOR
	// Unregister PIE delegates
	if (GEditor)
	{
		FEditorDelegates::BeginPIE.RemoveAll(this);
		FEditorDelegates::EndPIE.RemoveAll(this);
		
		UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE delegates unregistered"));
	}
#endif
	
	Super::Deinitialize();
}

void UEOSKitEngineSubsystem::BeginPIE(const bool bIsSimulating)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: PIE Session Starting"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Simulate Mode: %s"), bIsSimulating ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Preparing EOS Platform for PIE..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	bIsPIEActive = true;
	
	// Note: The EOSKit subsystem will be automatically initialized when the PIE world's game instance
	// gets created. We don't need to manually reload it here - the subsystem system handles this.
	
	// However, if you need to do any pre-PIE setup for EOS, you can do it here.
	// For example, you might want to ensure certain EOS services are ready, etc.
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE session started - EOSKit will initialize with game instance"));
#endif
}

void UEOSKitEngineSubsystem::EndPIE(const bool bIsSimulating)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: PIE Session Ending"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Simulate Mode: %s"), bIsSimulating ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Cleaning up EOS Platform from PIE..."));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	bIsPIEActive = false;
	
	// Note: The EOSKit subsystem will be automatically deinitialized when the PIE world's game instance
	// gets destroyed. We don't need to manually clean it up here.
	
	// However, if you have any PIE-specific cleanup that needs to happen before the subsystem
	// is destroyed, you can do it here.
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: PIE session ended - EOSKit subsystem will deinitialize with game instance"));
	
	// Optional: Force garbage collection to ensure clean state between PIE sessions
	// Uncomment if you experience issues with EOS state persisting between PIE sessions
	// CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, true);
#endif
}
