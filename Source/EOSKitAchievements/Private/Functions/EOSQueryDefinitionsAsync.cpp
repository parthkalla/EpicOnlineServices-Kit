// Copyright (C) 2024, All Rights Reserved.

#include "Functions/EOSQueryDefinitionsAsync.h"
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

UEOSQueryDefinitionsAsync* UEOSQueryDefinitionsAsync::QueryAchievementDefinitions(
	UObject* WorldContextObject,
	const FString& LocalUserId)
{
	UEOSQueryDefinitionsAsync* Node = NewObject<UEOSQueryDefinitionsAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->UserId = LocalUserId;
	return Node;
}

void UEOSQueryDefinitionsAsync::Activate()
{
	PerformQuery();
}

void UEOSQueryDefinitionsAsync::PerformQuery()
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

	// Convert user ID
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserId));
	if (!LocalPUID)
	{
		OnFail.Broadcast(TEXT("Invalid Product User ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup query options
	EOS_Achievements_QueryDefinitionsOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_LATEST;
	QueryOptions.LocalUserId = LocalPUID;

	UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Querying achievement definitions"));

	// Perform query - use lambda wrapper
	EOS_Achievements_QueryDefinitions(AchievementsHandle, &QueryOptions, this, [](const EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSQueryDefinitionsAsync* Self = static_cast<UEOSQueryDefinitionsAsync*>(Data->ClientData);

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
							// Get definition count
							EOS_Achievements_GetAchievementDefinitionCountOptions CountOptions = {};
							CountOptions.ApiVersion = EOS_ACHIEVEMENTS_GETACHIEVEMENTDEFINITIONCOUNT_API_LATEST;

							uint32_t DefinitionCount = EOS_Achievements_GetAchievementDefinitionCount(AchievementsHandle, &CountOptions);
							
							TArray<FEOSAchievementDefinition> Definitions;
							Definitions.Reserve(DefinitionCount);

							// Get each definition
							for (uint32_t i = 0; i < DefinitionCount; i++)
							{
								EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions CopyOptions = {};
								CopyOptions.ApiVersion = EOS_ACHIEVEMENTS_COPYACHIEVEMENTDEFINITIONV2BYINDEX_API_LATEST;
								CopyOptions.AchievementIndex = i;

								EOS_Achievements_DefinitionV2* Definition = nullptr;
								EOS_EResult CopyResult = EOS_Achievements_CopyAchievementDefinitionV2ByIndex(AchievementsHandle, &CopyOptions, &Definition);

								if (CopyResult == EOS_EResult::EOS_Success && Definition)
								{
									FEOSAchievementDefinition DefInfo;
									
									if (Definition->AchievementId)
									{
										DefInfo.AchievementId = UTF8_TO_TCHAR(Definition->AchievementId);
									}
									if (Definition->UnlockedDisplayName)
									{
										DefInfo.DisplayName = UTF8_TO_TCHAR(Definition->UnlockedDisplayName);
									}
									if (Definition->UnlockedDescription)
									{
										DefInfo.Description = UTF8_TO_TCHAR(Definition->UnlockedDescription);
									}
									if (Definition->LockedDisplayName)
									{
										DefInfo.LockedDisplayName = UTF8_TO_TCHAR(Definition->LockedDisplayName);
									}
									if (Definition->LockedDescription)
									{
										DefInfo.LockedDescription = UTF8_TO_TCHAR(Definition->LockedDescription);
									}
									if (Definition->LockedIconURL)
									{
										DefInfo.LockedIconUrl = UTF8_TO_TCHAR(Definition->LockedIconURL);
									}
									if (Definition->UnlockedIconURL)
									{
										DefInfo.UnlockedIconUrl = UTF8_TO_TCHAR(Definition->UnlockedIconURL);
									}
									
									DefInfo.bIsHidden = (Definition->bIsHidden == EOS_TRUE);
									DefInfo.StatThresholdCount = Definition->StatThresholdsCount;

									Definitions.Add(DefInfo);

									// Release the definition
									EOS_Achievements_DefinitionV2_Release(Definition);
								}
							}

							UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Query definitions successful. Found %d definitions"), Definitions.Num());
							Self->OnSuccess.Broadcast(Definitions);
							Self->SetReadyToDestroy();
							return;
						}
					}
				}

				Self->OnFail.Broadcast(TEXT("Failed to retrieve definitions"));
				Self->SetReadyToDestroy();
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Query definitions failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
				Self->SetReadyToDestroy();
			}
		});
	});
}
