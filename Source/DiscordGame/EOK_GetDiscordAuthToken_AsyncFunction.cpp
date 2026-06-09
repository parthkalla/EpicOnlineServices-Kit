// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_GetDiscordAuthToken_AsyncFunction.h"

#include "DiscordGameSubsystem.h"

UEOK_GetDiscordAuthToken_AsyncFunction* UEOK_GetDiscordAuthToken_AsyncFunction::GetDiscordAuthToken()
{
	UEOK_GetDiscordAuthToken_AsyncFunction* Node = NewObject<UEOK_GetDiscordAuthToken_AsyncFunction>();
	return Node;
}

void UEOK_GetDiscordAuthToken_AsyncFunction::Activate()
{
	Super::Activate();
#if EOKDISCORDACTIVE
	if(GEngine)
	{
		if(UDiscordGameSubsystem* DiscordSubsystem = GEngine->GetEngineSubsystem<UDiscordGameSubsystem>())
		{
			if(DiscordSubsystem->IsDiscordRunning())
			{
				if(DiscordSubsystem->IsDiscordSDKLoaded())
				{
					if(DiscordSubsystem->IsClientIdValid())
					{
						DiscordSubsystem->DiscordCore().ApplicationManager().GetOAuth2Token([this](discord::Result result, discord::OAuth2Token const& token)
						{
							if(result == discord::Result::Ok)
							{
								OnSuccess.Broadcast(ANSI_TO_TCHAR(token.GetAccessToken()), "");
							}
							else
							{
								UE_LOG(LogDiscord, Error, TEXT("Failed to get Discord Auth Token due to error: %d"), static_cast<int32>(result));
								OnFailure.Broadcast("",TEXT("Failed to get Discord Auth Token"));
							}
							SetReadyToDestroy();
							#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
						});
						return;
					}
					else
					{
						OnFailure.Broadcast("", "Client ID is not valid");
					}
				}
				else
				{
					OnFailure.Broadcast("", "Discord SDK is not loaded");
				}
			}
			else
			{
				OnFailure.Broadcast("", "DiscordCore is null");
			}
		}
		else
		{
			OnFailure.Broadcast("", "DiscordSubsystem is null");
		}
	}
	else
	{
		OnFailure.Broadcast("", "GEngine is null");
	}
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
#else
	OnFailure.Broadcast("", "Discord setup files are not included in the project");
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
#endif
}
