// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSKitSanctionsTypes.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSubsystem.h"

#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sanctions.h"
#include "eos_sanctions_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// .generated.h must always be the last include
#include "EOSKitSanctionsSubsystem.generated.h"

/**
 * Delegate for query active player sanctions complete
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEOSQueryActivePlayerSanctionsComplete,
	const FEOSKitProductUserId&, LocalUserId,
	const FEOSKitProductUserId&, TargetUserId,
	EEOSResult, Result);

/**
 * Sanctions subsystem for managing EOS Sanctions interface
 */
UCLASS()
class EOSKITSANCTIONS_API UEOSKitSanctionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// SDK Function Wrappers
	// ========================================

	/**
	 * Query active player sanctions (async)
	 * @param LocalUserId - Product User ID of the local user initiating the request (can be null for dedicated servers)
	 * @param TargetUserId - Product User ID of the user whose active sanctions are to be retrieved
	 * @param Callback - Optional callback delegate (Note: This is async)
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sanctions")
	EEOSResult QueryActivePlayerSanctions(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId);

	/**
	 * Get player sanction count
	 * You must call QueryActivePlayerSanctions first to retrieve the data from the service backend.
	 * @param TargetUserId - Product User ID of the user whose sanction count should be returned
	 * @return Number of available sanctions for this player, or -1 on error
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sanctions")
	int32 GetPlayerSanctionCount(const FEOSKitProductUserId& TargetUserId) const;

	/**
	 * Copy player sanction by index
	 * You must call QueryActivePlayerSanctions first to retrieve the data from the service backend.
	 * @param TargetUserId - Product User ID of the user whose active sanctions are to be copied
	 * @param SanctionIndex - Index of the sanction to retrieve from the cache
	 * @param OutSanction - The player sanction data for the given index, if it exists and is valid
	 * @return Result code
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sanctions")
	EEOSResult CopyPlayerSanctionByIndex(const FEOSKitProductUserId& TargetUserId, int32 SanctionIndex, FEOSKitPlayerSanction& OutSanction) const;

	/**
	 * Create a player sanction appeal
	 * @param LocalUserId - Product User ID of the local user sending their own sanction appeal
	 * @param ReferenceId - A unique identifier for the specific sanction (from CopyPlayerSanctionByIndex)
	 * @param Reason - Reason code for the appeal
	 * @param Callback - Optional callback delegate (Note: This is async)
	 * @return Result code (Note: This is async, returns immediately)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Sanctions")
	EEOSResult CreatePlayerSanctionAppeal(const FEOSKitProductUserId& LocalUserId, 
		const FString& ReferenceId, 
		EEOSKitSanctionAppealReason Reason);

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Sanctions")
	FOnEOSQueryActivePlayerSanctionsComplete OnQueryActivePlayerSanctionsComplete;

private:
	UEOSKitSubsystem* GetEOSKitSubsystem() const;
	EOS_HSanctions GetSanctionsHandle() const;

	static void EOS_CALL OnQueryActivePlayerSanctionsCallback(const EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo* Data);
	static void EOS_CALL OnCreatePlayerSanctionAppealCallback(const EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfo* Data);
};

