// Copyright (C) 2024, All Rights Reserved.

#include "Functions/EOSQueryPlayerAchievementsAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_achievements.h"
#include "eos_achievements_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSQueryPlayerAchievementsAsync* UEOSQueryPlayerAchievementsAsync::QueryPlayerAchievements(
	UObject* WorldContextObject,
	const FString& LocalUserId,
	const FString& TargetUserId)
{
	UEOSQueryPlayerAchievementsAsync* Node = NewObject<UEOSQueryPlayerAchievementsAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->LocalId = LocalUserId;
	Node->TargetId = TargetUserId;
	return Node;
}

void UEOSQueryPlayerAchievementsAsync::Activate()
{
	PerformQuery();
}

void UEOSQueryPlayerAchievementsAsync::PerformQuery()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
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

	// Convert user IDs
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*LocalId));
	EOS_ProductUserId TargetPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*TargetId));
	
	if (!LocalPUID || !TargetPUID)
	{
		OnFail.Broadcast(TEXT("Invalid Product User ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup query options
	EOS_Achievements_QueryPlayerAchievementsOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_ACHIEVEMENTS_QUERYPLAYERACHIEVEMENTS_API_LATEST;
	QueryOptions.LocalUserId = LocalPUID;
	QueryOptions.TargetUserId = TargetPUID;

	UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Querying player achievements"));

	// Perform query - use lambda wrapper
	EOS_Achievements_QueryPlayerAchievements(AchievementsHandle, &QueryOptions, this, [](const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSQueryPlayerAchievementsAsync* Self = static_cast<UEOSQueryPlayerAchievementsAsync*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject);
				if (GameInstance)
				{
					UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
					if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
					{
						EOS_HAchievements AchievementsHandle = EOS_Platform_GetAchievementsInterface(EOSSubsystem->GetPlatformHandle());
						if (AchievementsHandle)
						{
							// Get achievement count
							EOS_Achievements_GetPlayerAchievementCountOptions CountOptions = {};
							CountOptions.ApiVersion = EOS_ACHIEVEMENTS_GETPLAYERACHIEVEMENTCOUNT_API_LATEST;
							CountOptions.UserId = Data->LocalUserId;

							uint32_t AchievementCount = EOS_Achievements_GetPlayerAchievementCount(AchievementsHandle, &CountOptions);
							
							TArray<FEOSPlayerAchievement> Achievements;
							Achievements.Reserve(AchievementCount);

							// Get each player achievement
							for (uint32_t i = 0; i < AchievementCount; i++)
							{
								EOS_Achievements_CopyPlayerAchievementByIndexOptions CopyOptions = {};
								CopyOptions.ApiVersion = EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYINDEX_API_LATEST;
								CopyOptions.LocalUserId = Data->LocalUserId;
								CopyOptions.TargetUserId = Data->TargetUserId;
								CopyOptions.AchievementIndex = i;

								EOS_Achievements_PlayerAchievement* Achievement = nullptr;
								EOS_EResult CopyResult = EOS_Achievements_CopyPlayerAchievementByIndex(AchievementsHandle, &CopyOptions, &Achievement);

								if (CopyResult == EOS_EResult::EOS_Success && Achievement)
								{
									FEOSPlayerAchievement AchievementInfo;
									
									if (Achievement->AchievementId)
									{
										AchievementInfo.AchievementId = UTF8_TO_TCHAR(Achievement->AchievementId);
									}
									if (Achievement->DisplayName)
									{
										AchievementInfo.DisplayName = UTF8_TO_TCHAR(Achievement->DisplayName);
									}
									if (Achievement->Description)
									{
										AchievementInfo.Description = UTF8_TO_TCHAR(Achievement->Description);
									}
									if (Achievement->IconURL)
									{
										AchievementInfo.IconURL = UTF8_TO_TCHAR(Achievement->IconURL);
									}
									
									AchievementInfo.Progress = Achievement->Progress;
									AchievementInfo.UnlockTime = FDateTime::FromUnixTimestamp(Achievement->UnlockTime);
									
									// Check if unlocked based on progress
									AchievementInfo.bIsUnlocked = (Achievement->Progress >= 1.0);

									Achievements.Add(AchievementInfo);

									// Release the achievement
									EOS_Achievements_PlayerAchievement_Release(Achievement);
								}
							}

							UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Query player achievements successful. Found %d achievements"), Achievements.Num());
							Self->OnSuccess.Broadcast(Achievements);
							Self->SetReadyToDestroy();
							return;
						}
					}
				}

				Self->OnFail.Broadcast(TEXT("Failed to retrieve player achievements"));
				Self->SetReadyToDestroy();
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Query player achievements failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
				Self->SetReadyToDestroy();
			}
		});
	});
}
