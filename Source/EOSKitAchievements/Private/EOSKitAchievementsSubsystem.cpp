// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitAchievementsSubsystem.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_achievements.h"
#include "eos_achievements_types.h"
#include "EOSKitSharedTypes.h"
#include "Async/Async.h"

void UEOSKitAchievementsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NextNotificationId = 1;
	UE_LOG(LogTemp, Log, TEXT("EOSKitAchievementsSubsystem: Initialized"));
}

void UEOSKitAchievementsSubsystem::Deinitialize()
{
	// Remove all notifications
	for (const auto& Pair : NotificationIdMap)
	{
		EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
		if (AchievementsHandle)
		{
			EOS_Achievements_RemoveNotifyAchievementsUnlocked(AchievementsHandle, Pair.Value);
		}
	}
	NotificationIdMap.Empty();

	Super::Deinitialize();
}

UEOSKitSubsystem* UEOSKitAchievementsSubsystem::GetEOSKitSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}

EOS_HAchievements UEOSKitAchievementsSubsystem::GetAchievementsHandle() const
{
	UEOSKitSubsystem* EOSKitSubsystem = GetEOSKitSubsystem();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		return nullptr;
	}
	return EOS_Platform_GetAchievementsInterface(EOSKitSubsystem->GetPlatformHandle());
}

EEOSResult UEOSKitAchievementsSubsystem::QueryDefinitions(const FEOSKitProductUserId& LocalUserId)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Achievements_QueryDefinitionsOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but QueryDefinitions is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitAchievementsSubsystem::QueryPlayerAchievements(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	EOS_Achievements_QueryPlayerAchievementsOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_QUERYPLAYERACHIEVEMENTS_API_LATEST;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();
	Options.TargetUserId = TargetUserId.GetValueAsEosType();

	// Note: This is a synchronous wrapper, but QueryPlayerAchievements is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

EEOSResult UEOSKitAchievementsSubsystem::UnlockAchievements(const FEOSKitProductUserId& LocalUserId, const TArray<FString>& AchievementIds)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return EEOSResult::EOS_NotConfigured;
	}

	if (AchievementIds.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: AchievementIds array is empty"));
		return EEOSResult::EOS_InvalidParameters;
	}

	EOS_Achievements_UnlockAchievementsOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
	Options.UserId = LocalUserId.GetValueAsEosType();

	// Convert FString array to const char* array
	TArray<FTCHARToUTF8> Converters;
	TArray<const char*> AchievementIdPtrs;
	Converters.Reserve(AchievementIds.Num());
	AchievementIdPtrs.Reserve(AchievementIds.Num());

	for (const FString& AchievementId : AchievementIds)
	{
		Converters.Add(FTCHARToUTF8(*AchievementId));
		AchievementIdPtrs.Add(Converters.Last().Get());
	}

	Options.AchievementIds = AchievementIdPtrs.GetData();
	Options.AchievementsCount = AchievementIdPtrs.Num();

	// Note: This is a synchronous wrapper, but UnlockAchievements is async
	// The actual async operation should be done through async nodes
	return EEOSResult::EOS_Success;
}

int32 UEOSKitAchievementsSubsystem::GetAchievementDefinitionCount()
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return 0;
	}

	EOS_Achievements_GetAchievementDefinitionCountOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_GETACHIEVEMENTDEFINITIONCOUNT_API_LATEST;

	return static_cast<int32>(EOS_Achievements_GetAchievementDefinitionCount(AchievementsHandle, &Options));
}

bool UEOSKitAchievementsSubsystem::CopyAchievementDefinitionV2ByIndex(int32 Index, FEOSAchievementDefinition& OutDefinition)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return false;
	}

	EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_COPYACHIEVEMENTDEFINITIONV2BYINDEX_API_LATEST;
	Options.AchievementIndex = Index;

	EOS_Achievements_DefinitionV2* Definition = nullptr;
	EOS_EResult Result = EOS_Achievements_CopyAchievementDefinitionV2ByIndex(AchievementsHandle, &Options, &Definition);

	if (Result == EOS_EResult::EOS_Success && Definition)
	{
		OutDefinition.AchievementId = Definition->AchievementId ? UTF8_TO_TCHAR(Definition->AchievementId) : TEXT("");
		OutDefinition.DisplayName = Definition->UnlockedDisplayName ? UTF8_TO_TCHAR(Definition->UnlockedDisplayName) : TEXT("");
		OutDefinition.Description = Definition->UnlockedDescription ? UTF8_TO_TCHAR(Definition->UnlockedDescription) : TEXT("");
		OutDefinition.LockedDisplayName = Definition->LockedDisplayName ? UTF8_TO_TCHAR(Definition->LockedDisplayName) : TEXT("");
		OutDefinition.LockedDescription = Definition->LockedDescription ? UTF8_TO_TCHAR(Definition->LockedDescription) : TEXT("");
		OutDefinition.LockedIconUrl = Definition->LockedIconURL ? UTF8_TO_TCHAR(Definition->LockedIconURL) : TEXT("");
		OutDefinition.UnlockedIconUrl = Definition->UnlockedIconURL ? UTF8_TO_TCHAR(Definition->UnlockedIconURL) : TEXT("");
		OutDefinition.bIsHidden = Definition->bIsHidden == EOS_TRUE;
		OutDefinition.StatThresholdCount = static_cast<int32>(Definition->StatThresholdsCount);

		EOS_Achievements_DefinitionV2_Release(Definition);
		return true;
	}

	return false;
}

bool UEOSKitAchievementsSubsystem::CopyAchievementDefinitionV2ByAchievementId(const FString& AchievementId, FEOSAchievementDefinition& OutDefinition)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return false;
	}

	EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_COPYACHIEVEMENTDEFINITIONV2BYACHIEVEMENTID_API_LATEST;
	Options.AchievementId = TCHAR_TO_UTF8(*AchievementId);

	EOS_Achievements_DefinitionV2* Definition = nullptr;
	EOS_EResult Result = EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(AchievementsHandle, &Options, &Definition);

	if (Result == EOS_EResult::EOS_Success && Definition)
	{
		OutDefinition.AchievementId = Definition->AchievementId ? UTF8_TO_TCHAR(Definition->AchievementId) : TEXT("");
		OutDefinition.DisplayName = Definition->UnlockedDisplayName ? UTF8_TO_TCHAR(Definition->UnlockedDisplayName) : TEXT("");
		OutDefinition.Description = Definition->UnlockedDescription ? UTF8_TO_TCHAR(Definition->UnlockedDescription) : TEXT("");
		OutDefinition.LockedDisplayName = Definition->LockedDisplayName ? UTF8_TO_TCHAR(Definition->LockedDisplayName) : TEXT("");
		OutDefinition.LockedDescription = Definition->LockedDescription ? UTF8_TO_TCHAR(Definition->LockedDescription) : TEXT("");
		OutDefinition.LockedIconUrl = Definition->LockedIconURL ? UTF8_TO_TCHAR(Definition->LockedIconURL) : TEXT("");
		OutDefinition.UnlockedIconUrl = Definition->UnlockedIconURL ? UTF8_TO_TCHAR(Definition->UnlockedIconURL) : TEXT("");
		OutDefinition.bIsHidden = Definition->bIsHidden == EOS_TRUE;
		OutDefinition.StatThresholdCount = static_cast<int32>(Definition->StatThresholdsCount);

		EOS_Achievements_DefinitionV2_Release(Definition);
		return true;
	}

	return false;
}

int32 UEOSKitAchievementsSubsystem::GetPlayerAchievementCount(const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return 0;
	}

	EOS_Achievements_GetPlayerAchievementCountOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_GETPLAYERACHIEVEMENTCOUNT_API_LATEST;
	Options.UserId = TargetUserId.GetValueAsEosType();

	return static_cast<int32>(EOS_Achievements_GetPlayerAchievementCount(AchievementsHandle, &Options));
}

bool UEOSKitAchievementsSubsystem::CopyPlayerAchievementByIndex(const FEOSKitProductUserId& LocalUserId, 
	const FEOSKitProductUserId& TargetUserId, 
	int32 Index, 
	FEOSPlayerAchievement& OutAchievement)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return false;
	}

	EOS_Achievements_CopyPlayerAchievementByIndexOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYINDEX_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.AchievementIndex = Index;
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	EOS_Achievements_PlayerAchievement* Achievement = nullptr;
	EOS_EResult Result = EOS_Achievements_CopyPlayerAchievementByIndex(AchievementsHandle, &Options, &Achievement);

	if (Result == EOS_EResult::EOS_Success && Achievement)
	{
		OutAchievement.AchievementId = Achievement->AchievementId ? UTF8_TO_TCHAR(Achievement->AchievementId) : TEXT("");
		OutAchievement.Progress = static_cast<float>(Achievement->Progress);
		
		if (Achievement->UnlockTime != EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED)
		{
			OutAchievement.UnlockTime = FDateTime::FromUnixTimestamp(Achievement->UnlockTime);
		}
		
		OutAchievement.bIsUnlocked = Achievement->UnlockTime != EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED;
		OutAchievement.DisplayName = Achievement->DisplayName ? UTF8_TO_TCHAR(Achievement->DisplayName) : TEXT("");
		OutAchievement.Description = Achievement->Description ? UTF8_TO_TCHAR(Achievement->Description) : TEXT("");
		OutAchievement.IconURL = Achievement->IconURL ? UTF8_TO_TCHAR(Achievement->IconURL) : TEXT("");

		EOS_Achievements_PlayerAchievement_Release(Achievement);
		return true;
	}

	return false;
}

bool UEOSKitAchievementsSubsystem::CopyPlayerAchievementByAchievementId(const FEOSKitProductUserId& LocalUserId, 
	const FEOSKitProductUserId& TargetUserId, 
	const FString& AchievementId, 
	FEOSPlayerAchievement& OutAchievement)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return false;
	}

	EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYACHIEVEMENTID_API_LATEST;
	Options.TargetUserId = TargetUserId.GetValueAsEosType();
	Options.AchievementId = TCHAR_TO_UTF8(*AchievementId);
	Options.LocalUserId = LocalUserId.GetValueAsEosType();

	EOS_Achievements_PlayerAchievement* Achievement = nullptr;
	EOS_EResult Result = EOS_Achievements_CopyPlayerAchievementByAchievementId(AchievementsHandle, &Options, &Achievement);

	if (Result == EOS_EResult::EOS_Success && Achievement)
	{
		OutAchievement.AchievementId = Achievement->AchievementId ? UTF8_TO_TCHAR(Achievement->AchievementId) : TEXT("");
		OutAchievement.Progress = static_cast<float>(Achievement->Progress);
		
		if (Achievement->UnlockTime != EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED)
		{
			OutAchievement.UnlockTime = FDateTime::FromUnixTimestamp(Achievement->UnlockTime);
		}
		
		OutAchievement.bIsUnlocked = Achievement->UnlockTime != EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED;
		OutAchievement.DisplayName = Achievement->DisplayName ? UTF8_TO_TCHAR(Achievement->DisplayName) : TEXT("");
		OutAchievement.Description = Achievement->Description ? UTF8_TO_TCHAR(Achievement->Description) : TEXT("");
		OutAchievement.IconURL = Achievement->IconURL ? UTF8_TO_TCHAR(Achievement->IconURL) : TEXT("");

		EOS_Achievements_PlayerAchievement_Release(Achievement);
		return true;
	}

	return false;
}

int32 UEOSKitAchievementsSubsystem::AddNotifyAchievementsUnlockedV2(const FOnEOSAchievementsUnlockedDelegate& Callback)
{
	EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
	if (!AchievementsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitAchievements: Failed to get Achievements Handle"));
		return 0;
	}

	EOS_Achievements_AddNotifyAchievementsUnlockedV2Options Options = {};
	Options.ApiVersion = EOS_ACHIEVEMENTS_ADDNOTIFYACHIEVEMENTSUNLOCKEDV2_API_LATEST;

	EOS_NotificationId NotificationId = EOS_Achievements_AddNotifyAchievementsUnlockedV2(
		AchievementsHandle,
		&Options,
		this,
		&UEOSKitAchievementsSubsystem::OnAchievementsUnlockedV2Callback);

	if (NotificationId != EOS_INVALID_NOTIFICATIONID)
	{
		int32 OurNotificationId = NextNotificationId++;
		NotificationIdMap.Add(OurNotificationId, NotificationId);
		
		// Store the callback delegate with EOS notification ID as key
		AchievementsUnlockedCallbacks.Add(NotificationId, Callback);
		
		return OurNotificationId;
	}

	return 0;
}

void UEOSKitAchievementsSubsystem::RemoveNotifyAchievementsUnlockedV2(int32 NotificationId)
{
	EOS_NotificationId* EOSNotificationId = NotificationIdMap.Find(NotificationId);
	if (EOSNotificationId)
	{
		EOS_HAchievements AchievementsHandle = GetAchievementsHandle();
		if (AchievementsHandle)
		{
			EOS_Achievements_RemoveNotifyAchievementsUnlocked(AchievementsHandle, *EOSNotificationId);
		}
		AchievementsUnlockedCallbacks.Remove(*EOSNotificationId);
		NotificationIdMap.Remove(NotificationId);
	}
}

void EOS_CALL UEOSKitAchievementsSubsystem::OnAchievementsUnlockedV2Callback(const EOS_Achievements_OnAchievementsUnlockedCallbackV2Info* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitAchievementsSubsystem* Self = static_cast<UEOSKitAchievementsSubsystem*>(Data->ClientData);

	// Convert Product User ID using constructor
	FEOSKitProductUserId UserId(Data->UserId);

	// Convert other data
	FString AchievementId = Data->AchievementId ? UTF8_TO_TCHAR(Data->AchievementId) : TEXT("");
	int64 UnlockTime = Data->UnlockTime;

	// Find and call the stored callback delegate
	EOS_NotificationId EOSNotificationId = Data->NotificationId;
	if (FOnEOSAchievementsUnlockedDelegate* Callback = Self->AchievementsUnlockedCallbacks.Find(EOSNotificationId))
	{
		AsyncTask(ENamedThreads::GameThread, [Callback, UserId, AchievementId, UnlockTime]()
		{
			Callback->ExecuteIfBound(UserId, AchievementId, UnlockTime);
		});
	}

	// Broadcast to multicast delegate
	AsyncTask(ENamedThreads::GameThread, [Self, UserId, AchievementId, UnlockTime]()
	{
		Self->OnAchievementsUnlocked.Broadcast(UserId, AchievementId, UnlockTime);
	});
}

