// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Achievements_QueryDefinitions.h"
#include "eos_achievements.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Achievements_QueryDefinitions* UEOK_Achievements_QueryDefinitions::EOK_Achievements_QueryDefinitions(
	FEOK_ProductUserId UserId)
{
	UEOK_Achievements_QueryDefinitions* Node = NewObject<UEOK_Achievements_QueryDefinitions>();
	Node->Var_UserId = UserId;
	return Node;
}

void UEOK_Achievements_QueryDefinitions::Activate()
{
	Super::Activate();
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Achievements_QueryDefinitionsOptions Options;
			Options.ApiVersion = EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_LATEST;
			Options.LocalUserId = Var_UserId.GetValueAsEosType();
			EOS_Achievements_QueryDefinitions(EOSRef->AchievementsHandle, &Options, this, [](const EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo* Data)
			{
				if (UEOK_Achievements_QueryDefinitions* QueryDefinitions = static_cast<UEOK_Achievements_QueryDefinitions*>(Data->ClientData))
				{
					QueryDefinitions->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode));
					QueryDefinitions->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					QueryDefinitions->MarkAsGarbage();
#else
					QueryDefinitions->MarkPendingKill();
#endif
				}
			});
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get EOS subsystem"));
	OnCallback.Broadcast(EEOK_Result::EOS_ServiceFailure);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
