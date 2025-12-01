// Copyright (C) 2024, All Rights Reserved.

#include "Functions/EOSUnlockAchievementAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_achievements.h"
#include "eos_achievements_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSUnlockAchievementAsync* UEOSUnlockAchievementAsync::UnlockAchievements(
	UObject* WorldContextObject,
	const FString& LocalUserId,
	const TArray<FString>& AchievementIds)
{
	UEOSUnlockAchievementAsync* Node = NewObject<UEOSUnlockAchievementAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->UserId = LocalUserId;
	Node->AchievementIdsToUnlock = AchievementIds;
	return Node;
}

void UEOSUnlockAchievementAsync::Activate()
{
	PerformUnlock();
}

void UEOSUnlockAchievementAsync::PerformUnlock()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	if (AchievementIdsToUnlock.Num() == 0)
	{
		OnFail.Broadcast(TEXT("No achievement IDs provided"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT("Failed to get Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT("EOSKit Subsystem not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HAchievements AchievementsHandle = EOS_Platform_GetAchievementsInterface(PlatformHandle);

	if (!AchievementsHandle)
	{
		OnFail.Broadcast(TEXT("Failed to get Achievements interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert user ID
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserId));
	if (!LocalPUID)
	{
		OnFail.Broadcast(TEXT("Invalid Product User ID"));
		SetReadyToDestroy();
		return;
	}

	// Convert achievement IDs to ANSI
	TArray<FTCHARToUTF8> AchievementIdConverters;
	TArray<const char*> AchievementIdPtrs;
	
	for (const FString& Id : AchievementIdsToUnlock)
	{
		AchievementIdConverters.Emplace(*Id);
	}
	
	for (const FTCHARToUTF8& Converter : AchievementIdConverters)
	{
		AchievementIdPtrs.Add(Converter.Get());
	}

	// Setup unlock options
	EOS_Achievements_UnlockAchievementsOptions UnlockOptions = {};
	UnlockOptions.ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
	UnlockOptions.UserId = LocalPUID;
	UnlockOptions.AchievementIds = AchievementIdPtrs.GetData();
	UnlockOptions.AchievementsCount = AchievementIdPtrs.Num();

	UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Unlocking %d achievements"), AchievementIdsToUnlock.Num());

	// Perform unlock - use lambda wrapper
	EOS_Achievements_UnlockAchievements(AchievementsHandle, &UnlockOptions, this, [](const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSUnlockAchievementAsync* Self = static_cast<UEOSUnlockAchievementAsync*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Achievements unlocked successfully"));
				Self->OnSuccess.Broadcast();
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Unlock achievements failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
			}

			Self->SetReadyToDestroy();
		});
	});
}
