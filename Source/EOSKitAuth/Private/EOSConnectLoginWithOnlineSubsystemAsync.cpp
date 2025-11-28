// Copyright (C) 2024, All Rights Reserved.

#include "EOSConnectLoginWithOnlineSubsystemAsync.h"
#include "OnlineSubsystemUtils.h"

UEOSConnectLoginWithOnlineSubsystemAsync* UEOSConnectLoginWithOnlineSubsystemAsync::LoginUsingConnectInterfaceOSS(
	EEOSKitExternalCredentialType LoginMethod,
	FString DisplayName,
	FString Token)
{
	UEOSConnectLoginWithOnlineSubsystemAsync* LoginObject = NewObject<UEOSConnectLoginWithOnlineSubsystemAsync>();
	LoginObject->DisplayName = DisplayName;
	LoginObject->Token = Token;
	LoginObject->LoginMethod = LoginMethod;
	return LoginObject;
}

void UEOSConnectLoginWithOnlineSubsystemAsync::Activate()
{
	Super::Activate();
	
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get())
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			UE_LOG(LogTemp, Verbose, TEXT("EOSKit: Subsystem and Identity Interface is valid and proceeding with login."));
			
			FOnlineAccountCredentials AccountDetails;
			AccountDetails.Id = DisplayName;
			AccountDetails.Token = Token;
			
			// Build the credential type string based on the login method
			FString CredentialTypeString;
			switch (LoginMethod)
			{
			case EEOSKitExternalCredentialType::DeviceID:
				CredentialTypeString = TEXT("DeviceID");
				break;
			case EEOSKitExternalCredentialType::Epic:
				CredentialTypeString = TEXT("Epic");
				break;
			case EEOSKitExternalCredentialType::Steam:
				CredentialTypeString = TEXT("Steam");
				break;
			case EEOSKitExternalCredentialType::PSN:
				CredentialTypeString = TEXT("PSN");
				break;
			case EEOSKitExternalCredentialType::XBL:
				CredentialTypeString = TEXT("XBL");
				break;
			case EEOSKitExternalCredentialType::Discord:
				CredentialTypeString = TEXT("Discord");
				break;
			case EEOSKitExternalCredentialType::Nintendo:
				CredentialTypeString = TEXT("Nintendo");
				break;
			case EEOSKitExternalCredentialType::Apple:
				CredentialTypeString = TEXT("Apple");
				break;
			case EEOSKitExternalCredentialType::Google:
				CredentialTypeString = TEXT("Google");
				break;
			case EEOSKitExternalCredentialType::Oculus:
				CredentialTypeString = TEXT("Oculus");
				break;
			case EEOSKitExternalCredentialType::OpenID:
				CredentialTypeString = TEXT("OpenID");
				break;
			default:
				CredentialTypeString = TEXT("DeviceID");
				break;
			}
			
			// Use a format similar to EIK: "noeas_+_CredentialType"
			AccountDetails.Type = TEXT("noeas_+_") + CredentialTypeString;
			
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Login Method: %s"), *AccountDetails.Type);
			
			IdentityPointerRef->OnLoginCompleteDelegates->AddUObject(this, &UEOSConnectLoginWithOnlineSubsystemAsync::LoginCallback);
			IdentityPointerRef->Login(0, AccountDetails);
			return;
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("EOSKit: Subsystem or Identity Interface is not valid"));
	OnFail.Broadcast("", "Subsystem or Identity Interface is not valid");
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOSConnectLoginWithOnlineSubsystemAsync::LoginCallback(
	int32 LocalUserNum,
	bool bWasSuccess,
	const FUniqueNetId& UserId,
	const FString& Error)
{
	if(bWasSuccess)
	{
		if(UserId.IsValid())
		{
			UE_LOG(LogTemp, Verbose, TEXT("EOSKit: Login was successful. UserID: %s"), *UserId.ToString());
			OnSuccess.Broadcast(UserId.ToString(), "");
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Login was successful but UserID is not valid"));
			OnFail.Broadcast("", "UserID is not valid");
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Login failed. Error: %s"), *Error);
		OnFail.Broadcast("", Error);
	}
	
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
