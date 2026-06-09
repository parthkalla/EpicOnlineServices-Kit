// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_Logout_AsyncFunction.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

UEOK_Logout_AsyncFunction* UEOK_Logout_AsyncFunction::LogoutUsingEOK()
{
	UEOK_Logout_AsyncFunction* BlueprintNode = NewObject<UEOK_Logout_AsyncFunction>();
	return BlueprintNode;
}

void UEOK_Logout_AsyncFunction::Activate()
{
	LogoutPlayer();
	Super::Activate();
}

void UEOK_Logout_AsyncFunction::LogoutPlayer()
{
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get())
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			IdentityPointerRef->OnLogoutCompleteDelegates->AddUObject(this,&UEOK_Logout_AsyncFunction::LogoutCallback);
			IdentityPointerRef->Logout(0);
		}
		else
		{
			OnFail.Broadcast();
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else
	{
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}

void UEOK_Logout_AsyncFunction::LogoutCallback(int32 LocalUserNum, bool bWasSuccess)
{
	if(bWasSuccess)
	{
		OnSuccess.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	else
	{
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
}
