// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_AchievementsSubsystem.h"

#include "eos_achievements.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

FEOK_NotificationId UEOK_AchievementsSubsystem::EOK_Achievements_AddNotifyAchievementsUnlockedV2(FOnAchievementsUnlockedV2Callback Callback)
{
	OnAchievementsUnlockedV2 = Callback;
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_AddNotifyAchievementsUnlockedV2Options Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_ADDNOTIFYACHIEVEMENTSUNLOCKEDV2_API_LATEST;
			return EOS_Achievements_AddNotifyAchievementsUnlockedV2(EOSRef->AchievementsHandle, &Options, this,[](const EOS_Achievements_OnAchievementsUnlockedCallbackV2Info* Data)
			{
				if (UEOK_AchievementsSubsystem* AchievementsSubsystem = static_cast<UEOK_AchievementsSubsystem*>(Data->ClientData))
				{
					AchievementsSubsystem->OnAchievementsUnlockedV2.ExecuteIfBound(Data->UserId, Data->AchievementId, Data->UnlockTime);
				}
			});
		}
	}
	return FEOK_NotificationId();
}

TEnumAsByte<EEOK_Result> UEOK_AchievementsSubsystem::EOK_Achievements_CopyAchievementDefinitionV2ByIndex(int32 Index,
	FEOK_Achievements_DefinitionV2& OutAchievementDefinition)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_COPYACHIEVEMENTDEFINITIONV2BYINDEX_API_LATEST;
			Options.AchievementIndex = Index;
			EOS_Achievements_DefinitionV2* OutAchievementDefinition1 = nullptr;
			EOS_EResult Result = EOS_Achievements_CopyAchievementDefinitionV2ByIndex(EOSRef->AchievementsHandle, &Options, &OutAchievementDefinition1);
			OutAchievementDefinition = *OutAchievementDefinition1;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_AchievementsSubsystem::EOK_Achievements_CopyPlayerAchievementByIndex(
	FEOK_ProductUserId TargetUserId, int32 Index, FEOK_ProductUserId LocalUserId,
	FEOK_Achievements_PlayerAchievement& OutPlayerAchievement)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_CopyPlayerAchievementByIndexOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYINDEX_API_LATEST;
			Options.AchievementIndex = Index;
			EOS_Achievements_PlayerAchievement* OutPlayerAchievement1 = nullptr;
			EOS_EResult Result = EOS_Achievements_CopyPlayerAchievementByIndex(EOSRef->AchievementsHandle, &Options, &OutPlayerAchievement1);
			OutPlayerAchievement = *OutPlayerAchievement1;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_ServiceFailure;
}

void UEOK_AchievementsSubsystem::EOK_Achievements_DefinitionV2_Release(
	FEOK_Achievements_DefinitionV2& AchievementDefinition)
{
	EOS_Achievements_DefinitionV2_Release(AchievementDefinition.GetValueAsEosType());
}

int32 UEOK_AchievementsSubsystem::EOK_Achievements_GetAchievementDefinitionCount()
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_GetAchievementDefinitionCountOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_GETACHIEVEMENTDEFINITIONCOUNT_API_LATEST;
			return EOS_Achievements_GetAchievementDefinitionCount(EOSRef->AchievementsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return 0;
}

int32 UEOK_AchievementsSubsystem::EOK_Achievements_GetPlayerAchievementCount(FEOK_ProductUserId UserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_GetPlayerAchievementCountOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_GETPLAYERACHIEVEMENTCOUNT_API_LATEST;
			Options.UserId = UserId.GetValueAsEosType();
			return EOS_Achievements_GetPlayerAchievementCount(EOSRef->AchievementsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return 0;
}

void UEOK_AchievementsSubsystem::EOK_Achievements_PlayerAchievement_Release(
	FEOK_Achievements_PlayerAchievement& PlayerAchievement)
{
	EOS_Achievements_PlayerAchievement_Release(PlayerAchievement.GetValueAsEosType());
}

TEnumAsByte<EEOK_Result> UEOK_AchievementsSubsystem::EOK_Achievements_QueryDefinitions(FEOK_ProductUserId UserId)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_QueryDefinitionsOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_LATEST;
			Options.LocalUserId = UserId.GetValueAsEosType();
			EOS_Achievements_QueryDefinitions(EOSRef->AchievementsHandle, &Options, this, nullptr);
			return static_cast<EEOK_Result>(EOS_EResult::EOS_Success);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_ServiceFailure;
}

void UEOK_AchievementsSubsystem::EOK_Achievements_RemoveNotifyAchievementsUnlocked(FEOK_NotificationId Id)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_RemoveNotifyAchievementsUnlocked(EOSRef->AchievementsHandle, Id.GetValueAsEosType());
		}
	}
}

TEnumAsByte<EEOK_Result> UEOK_AchievementsSubsystem::EOK_Achievements_CopyPlayerAchievementByAchievementId(FEOK_ProductUserId TargetUserId, FString AchievementId, FEOK_ProductUserId LocalUserId, FEOK_Achievements_PlayerAchievement& OutPlayerAchievement)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYACHIEVEMENTID_API_LATEST;
			if(AchievementId.IsEmpty())
			{
				UE_LOG(LogEOK, Error, TEXT("AchievementId is empty"));
				return EEOK_Result::EOS_ServiceFailure;
			}
			Options.AchievementId = TCHAR_TO_ANSI(*AchievementId);
			EOS_Achievements_PlayerAchievement* OutPlayerAchievement1 = nullptr;
			EOS_EResult Result = EOS_Achievements_CopyPlayerAchievementByAchievementId(EOSRef->AchievementsHandle, &Options, &OutPlayerAchievement1);
			OutPlayerAchievement = *OutPlayerAchievement1;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_ServiceFailure;
}

TEnumAsByte<EEOK_Result> UEOK_AchievementsSubsystem::EOK_Achievements_CopyAchievementDefinitionV2ByAchievementId(FString AchievementId, FEOK_Achievements_DefinitionV2& OutAchievementDefinition)
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_COPYACHIEVEMENTDEFINITIONV2BYACHIEVEMENTID_API_LATEST;
			if(AchievementId.IsEmpty())
			{
				UE_LOG(LogEOK, Error, TEXT("AchievementId is empty"));
				return EEOK_Result::EOS_ServiceFailure;
			}
			Options.AchievementId = TCHAR_TO_ANSI(*AchievementId);
			EOS_Achievements_DefinitionV2* OutAchievementDefinition1 = nullptr;
			EOS_EResult Result = EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(EOSRef->AchievementsHandle, &Options, &OutAchievementDefinition1);
			OutAchievementDefinition = *OutAchievementDefinition1;
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	return EEOK_Result::EOS_ServiceFailure;
}
