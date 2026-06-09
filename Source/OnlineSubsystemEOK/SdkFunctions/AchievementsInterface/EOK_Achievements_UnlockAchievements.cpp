// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Achievements_UnlockAchievements.h"

#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Achievements_UnlockAchievements* UEOK_Achievements_UnlockAchievements::EOK_Achievements_UnlockAchievements(
	FEOK_ProductUserId UserId, TArray<FString> AchievementIds)
{
	UEOK_Achievements_UnlockAchievements* Node = NewObject<UEOK_Achievements_UnlockAchievements>();
	Node->Var_UserId = UserId;
	Node->Var_AchievementIds = AchievementIds;
	return Node;
}

void UEOK_Achievements_UnlockAchievements::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_UnlockAchievementsOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
			Options.UserId = Var_UserId.GetValueAsEosType();
			Options.AchievementsCount = Var_AchievementIds.Num();
			Options.AchievementIds = new const char*[Var_AchievementIds.Num()];
			for (int i = 0; i < Var_AchievementIds.Num(); i++)
			{
				Options.AchievementIds[i] = TCHAR_TO_ANSI(*Var_AchievementIds[i]);
			}
			EOS_Achievements_UnlockAchievements(EOSRef->AchievementsHandle, &Options, this, [](const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo* Data)
			{
				if (UEOK_Achievements_UnlockAchievements* UnlockAchievements = static_cast<UEOK_Achievements_UnlockAchievements*>(Data->ClientData))
				{
					UnlockAchievements->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->UserId, Data->AchievementsCount);
					UnlockAchievements->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					UnlockAchievements->MarkAsGarbage();
#else
					UnlockAchievements->MarkPendingKill();
#endif
				}
			});
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	OnCallback.Broadcast(EEOK_Result::EOS_ServiceFailure, FEOK_ProductUserId(), 0);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
