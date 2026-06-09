// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Achievements_QueryPlayerAchievements.h"

#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"


UEOK_Achievements_QueryPlayerAchievements* UEOK_Achievements_QueryPlayerAchievements::
EOK_Achievements_QueryPlayerAchievements(FEOK_ProductUserId LocalUserId, FEOK_ProductUserId TargetUserId)
{
	UEOK_Achievements_QueryPlayerAchievements* Node = NewObject<UEOK_Achievements_QueryPlayerAchievements>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Achievements_QueryPlayerAchievements::Activate()
{
	Super::Activate();
	
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_QueryPlayerAchievementsOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_QUERYPLAYERACHIEVEMENTS_API_LATEST;
			Options.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			Options.TargetUserId = Var_TargetUserId.GetValueAsEosType();
			EOS_Achievements_QueryPlayerAchievements(EOSRef->AchievementsHandle, &Options, this, [](const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo* Data)
			{
				if (UEOK_Achievements_QueryPlayerAchievements* QueryPlayerAchievements = static_cast<UEOK_Achievements_QueryPlayerAchievements*>(Data->ClientData))
				{
					QueryPlayerAchievements->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), Data->LocalUserId, Data->TargetUserId);
					QueryPlayerAchievements->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
QueryPlayerAchievements->MarkAsGarbage();
#else
QueryPlayerAchievements->MarkPendingKill();
#endif
				}
			});
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	OnCallback.Broadcast(EEOK_Result::EOS_ServiceFailure, FEOK_ProductUserId(), FEOK_ProductUserId());
}
