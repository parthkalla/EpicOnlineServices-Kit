// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "EOSKitEngineSubsystem.generated.h"

/**
 * EOSKit Engine Subsystem - Manages EOSKit lifecycle at the engine level
 * 
 * This subsystem handles:
 * - PIE (Play In Editor) session initialization
 * - EOS Platform reloading when starting/stopping PIE
 * - Editor-specific cleanup and initialization
 */
UCLASS()
class EOSKIT_API UEOSKitEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Initialize the engine subsystem
	 * Sets up PIE delegates in editor builds
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Deinitialize the engine subsystem
	 * Cleans up PIE delegates in editor builds
	 */
	virtual void Deinitialize() override;

	/**
	 * Called when PIE session begins
	 * Reloads the EOSKit subsystem to ensure clean state
	 * 
	 * @param bIsSimulating - Whether PIE is running in simulate mode
	 */
	void BeginPIE(const bool bIsSimulating);

	/**
	 * Called when PIE session ends
	 * Cleans up any PIE-specific state
	 * 
	 * @param bIsSimulating - Whether PIE was running in simulate mode
	 */
	void EndPIE(const bool bIsSimulating);

private:
	/** Whether PIE is currently active */
	bool bIsPIEActive = false;
};
