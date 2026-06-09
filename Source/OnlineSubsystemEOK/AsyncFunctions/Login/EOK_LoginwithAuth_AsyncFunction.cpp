// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_LoginwithAuth_AsyncFunction.h"

#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_LoginwithAuth_AsyncFunction* UEOK_LoginwithAuth_AsyncFunction::LoginWithAuth(TEnumAsByte<EEOK_ELoginCredentialType> CredentialType, TEnumAsByte<EEOK_EExternalCredentialType> ExternalCredentialType, FString Id, FString Token)
{
	UEOK_LoginwithAuth_AsyncFunction* UEOK_LoginObject = NewObject<UEOK_LoginwithAuth_AsyncFunction>();
	UEOK_LoginObject->Var_CredentialType = CredentialType;
	UEOK_LoginObject->Var_Id = Id;
	UEOK_LoginObject->Var_Token = Token;
	UEOK_LoginObject->Var_ExternalCredentialType = ExternalCredentialType;
	return UEOK_LoginObject;
}

void UEOK_LoginwithAuth_AsyncFunction::Activate()
{
	Super::Activate();
	if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(GetWorld()))
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			UE_LOG(LogEOK, Verbose, TEXT("LoginUsingAuthInterface: Subsystem and Identity Interface is valid and proceeding with login."));
			FString EGS_Token;
			FOnlineAccountCredentials AccountDetails;
			AccountDetails.Id = Var_Id;
			AccountDetails.Token = Var_Token;
			AccountDetails.Type = "eas_+_" + UEnum::GetValueAsString(Var_CredentialType) + "_+_" + UEnum::GetValueAsString(Var_ExternalCredentialType);
			IdentityPointerRef->OnLoginCompleteDelegates->AddUObject(this,&UEOK_LoginwithAuth_AsyncFunction::LoginCallback);
			IdentityPointerRef->Login(0,AccountDetails);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("LoginUsingAuthInterface: Subsystem or Identity Interface is not valid"));
	OnFailure.Broadcast(FEOK_EpicAccountId(),FEOK_ProductUserId(), "Subsystem or Identity Interface is not valid");
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_LoginwithAuth_AsyncFunction::LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId,
	const FString& Error)
{
	if(bWasSuccess)
	{
		if(UserId.IsValid())
		{
			FString UserIdString = UserId.ToString();
			UE_LOG(LogEOK, Verbose, TEXT("LoginUsingAuthInterface: Login was successful. UserID: %s"), *UserIdString);
			
			FString EpicId, ProductId;
			if(UserIdString.Split(TEXT("|"), &EpicId, &ProductId))
			{
				// Full Epic Account login with EpicID|ProductID format
				UE_LOG(LogEOK, Verbose, TEXT("LoginUsingAuthInterface: Login was successful. EpicID: %s, ProductID: %s"), *EpicId, *ProductId);
				OnSuccess.Broadcast(EOS_EpicAccountId_FromString(TCHAR_TO_ANSI(*EpicId)), EOS_ProductUserId_FromString(TCHAR_TO_ANSI(*ProductId)), "");
			}
			else
			{
				// Device ID or other login type - only Product User ID is available
				// Check if it's a valid EOS Product User ID format (starts with specific prefix or is a valid hex string)
				// For Device ID logins, the UserID is the Product User ID itself
				UE_LOG(LogEOK, Verbose, TEXT("LoginUsingAuthInterface: Device ID or Connect login detected. ProductID: %s"), *UserIdString);
				// Use empty Epic Account ID for Device ID logins
				OnSuccess.Broadcast(FEOK_EpicAccountId(), EOS_ProductUserId_FromString(TCHAR_TO_ANSI(*UserIdString)), "");
			}
		}
		else
		{
			UE_LOG(LogEOK, Error, TEXT("LoginUsingAuthInterface: Login was successful but UserID is not valid"));
			OnFailure.Broadcast(FEOK_EpicAccountId(), FEOK_ProductUserId(), "UserID is not valid");
		}
	}
	else
	{
		UE_LOG(LogEOK, Error, TEXT("LoginUsingAuthInterface: Login failed. Error: %s"), *Error);
		OnFailure.Broadcast(FEOK_EpicAccountId(), FEOK_ProductUserId(), Error);
	}
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
