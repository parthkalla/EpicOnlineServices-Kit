// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_Login_AsyncFunction.h"

#include "EOKSettings.h"
#include "Online.h"
#include "Misc/CommandLine.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"


UEOK_Login_AsyncFunction* UEOK_Login_AsyncFunction::LoginUsingConnectInterface(TEnumAsByte<EEOK_EExternalCredentialType> LoginMethod, FString DisplayName, FString Token)
{
	UEOK_Login_AsyncFunction* UEOK_LoginObject= NewObject<UEOK_Login_AsyncFunction>();
	UEOK_LoginObject->DisplayName = DisplayName;
	UEOK_LoginObject->Token = Token;
	UEOK_LoginObject->LoginMethod = LoginMethod;
	return UEOK_LoginObject;
}

void UEOK_Login_AsyncFunction::Activate()
{
	Super::Activate();
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			UE_LOG(LogEOK, Verbose, TEXT("LoginUsingConnectInterface: Subsystem and Identity Interface is valid and proceeding with login."));
			FString EGS_Token;
			FOnlineAccountCredentials AccountDetails;
			AccountDetails.Id = DisplayName;
			AccountDetails.Token = Token;
			AccountDetails.Type = "noeas_+_" + UEnum::GetValueAsString(LoginMethod);
			UE_LOG(LogEOK, Log, TEXT("LoginUsingConnectInterface: Login Method: %s"), *AccountDetails.Type);
			IdentityPointerRef->OnLoginCompleteDelegates->AddUObject(this,&UEOK_Login_AsyncFunction::LoginCallback);
			IdentityPointerRef->Login(0,AccountDetails);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("LoginUsingConnectInterface: Subsystem or Identity Interface is not valid"));
	OnFail.Broadcast(FEOK_ProductUserId(), "Subsystem or Identity Interface is not valid");
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_Login_AsyncFunction::LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId,
	const FString& Error)
{
	if(bWasSuccess)
	{
		if(UserId.IsValid())
		{
			UE_LOG(LogEOK, Verbose, TEXT("LoginUsingConnectInterface: Login was successful. UserID: %s"), *UserId.ToString());
			OnSuccess.Broadcast(EOS_ProductUserId_FromString(TCHAR_TO_ANSI(*UserId.ToString())), "");
		}
		else
		{
			UE_LOG(LogEOK, Error, TEXT("LoginUsingConnectInterface: Login was successful but UserID is not valid"));
			OnFail.Broadcast(FEOK_ProductUserId(), "UserID is not valid");
		}
	}
	else
	{
		UE_LOG(LogEOK, Error, TEXT("LoginUsingConnectInterface: Login failed. Error: %s"), *Error);
		OnFail.Broadcast(FEOK_ProductUserId(), Error);
	}
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
