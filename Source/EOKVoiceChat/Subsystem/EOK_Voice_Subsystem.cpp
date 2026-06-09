// Copyright 2023 Betide Studio. All rights reserved.


#include "EOK_Voice_Subsystem.h"
#include "Net/UnrealNetwork.h"


bool UEOK_Voice_Subsystem::EVOK_Local_Initialize()
{
	EVOK_VoiceChat = IVoiceChat::Get();
	if(EVOK_VoiceChat)
	{
		return EVOK_VoiceChat->Initialize();
	}
	return false;
}

IVoiceChat* UEOK_Voice_Subsystem::EVOK_Local_GetVoiceChat()
{
	if (!EVOK_VoiceChat)
	{
		EVOK_VoiceChat = IVoiceChat::Get();
		if(EVOK_VoiceChat)
		{
			EVOK_VoiceChat->Initialize();
		}
	}
	return EVOK_VoiceChat;
}

void UEOK_Voice_Subsystem::EVOK_Local_Connect(const FEOKResultDelegate& ResultDelegate)
{
	if(EVOK_VoiceChat)
	{
		EVOK_VoiceChat->Connect(FOnVoiceChatConnectCompleteDelegate::CreateLambda([this, ResultDelegate](const FVoiceChatResult& Result)
		{
			if(Result.IsSuccess())
			{
				ResultDelegate.ExecuteIfBound(true, EEVIKResultCodes::Success);
			}
			else
			{
				ResultDelegate.ExecuteIfBound(false, EEVIKResultCodes::Failed);
			}
		}
		));
	}	
}


void UEOK_Voice_Subsystem::PlayerListUpdated()
{
	if(UpdatePositionalVoiceChatTimerHandle.IsValid())
	{
		UpdatePositionalVoiceChatTimerHandle.Invalidate();
	}
	//GetWorld()->GetTimerManager().SetTimer(UpdatePositionalVoiceChatTimerHandle, this, &UEOK_Voice_Subsystem::UpdatePositionalVoiceChat, 0.2f, true);
}