// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_GetAppleAuthToken_AsyncFunction.h"


UEOK_GetAppleAuthToken_AsyncFunction* UEOK_GetAppleAuthToken_AsyncFunction::GetAppleAuthToken()
{
	UEOK_GetAppleAuthToken_AsyncFunction* Node = NewObject<UEOK_GetAppleAuthToken_AsyncFunction>();
	return Node;
}

void UEOK_GetAppleAuthToken_AsyncFunction::OnLoginComplete(int LocalUserNum, bool bWasSuccess,
                                                           const FUniqueNetId& UniqueNetId, const FString& Error)
{
	if (bWasSuccess)
	{
		if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(APPLE_SUBSYSTEM))
		{
			if (IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface())
			{
				if (!Identity->GetAuthToken(0).IsEmpty())
				{
					UE_LOG(LogEOK, Display, TEXT("Apple Auth Token found"));
					OnSuccess.Broadcast(Identity->GetAuthToken(0));
					SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					MarkAsGarbage();
#else
					MarkPendingKill();
#endif
					return;
				}
			}
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get Apple Auth Token due to login failure : %s"), *Error);
	OnFailure.Broadcast(TEXT(""));
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_GetAppleAuthToken_AsyncFunction::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(APPLE_SUBSYSTEM))
	{
		if (IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface())
		{
			if(Identity->GetAuthToken(0).IsEmpty())
			{
				UE_LOG(LogEOK, Display, TEXT("No Apple Auth Token found, attempting to login"));
				Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &UEOK_GetAppleAuthToken_AsyncFunction::OnLoginComplete));
				Identity->AutoLogin(0);
				return;
			}
			UE_LOG(LogEOK, Display, TEXT("Apple Auth Token found"));
			OnSuccess.Broadcast(Identity->GetAuthToken(0));
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to get Apple Auth Token due to missing OnlineSubsystem or IdentityInterface"));
	OnFailure.Broadcast(TEXT(""));
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
