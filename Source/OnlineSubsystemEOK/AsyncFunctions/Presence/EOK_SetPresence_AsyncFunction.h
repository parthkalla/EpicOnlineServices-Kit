// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "IOnlineSubsystemEOS.h"
#include "OnlineSubsystemUtils.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "EOK_SetPresence_AsyncFunction.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EPresenceStatus : uint8 {
	PR_Online       UMETA(DisplayName = "Online"),
	PR_Offline              UMETA(DisplayName = "Offline"),
	PR_Away        UMETA(DisplayName = "Away"),
	PR_ExtendedAway        UMETA(DisplayName = "ExtendedAway"),
	PR_DoNotDisturb              UMETA(DisplayName = "DoNotDisturb"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSetPresence_Delegate, const FString&, RichPresenceStr, const EPresenceStatus&, PresenceStatusEnum);


UCLASS()
class OnlineSubsystemEOK_API UEOK_SetPresence_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable, DisplayName = "Success")
	FSetPresence_Delegate OnSuccess;

	UPROPERTY(BlueprintAssignable, DisplayName = "Faliure")
	FSetPresence_Delegate OnFaliure;

	FString RichPresence;
	
	EPresenceStatus PresenceStatus;

	virtual void Activate() override;

	void SetPresence();

	void OnSetPresenceCompleted(const class FUniqueNetId& UserId, const bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, DisplayName = "Set EOK Presence", meta = (BlueprintInternalUseOnly = "true"), Category = "Epic Online Services-Kit V2 || Presence")
	static UEOK_SetPresence_AsyncFunction* SetEOSPresence(FString RichPresense, EPresenceStatus PresenceStatus);
};
