// Copyright (C) 2024, All Rights Reserved.

#include "EOSGetIdTokenAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_auth.h"
#include "eos_auth_types.h"

UEOSGetIdTokenAsync* UEOSGetIdTokenAsync::GetIdToken(UObject* WorldContextObject, const FString& EpicAccountId)
{
	UEOSGetIdTokenAsync* Node = NewObject<UEOSGetIdTokenAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->EpicAccountIdString = EpicAccountId;
	return Node;
}

void UEOSGetIdTokenAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(FEOSKitIdToken());
		SetReadyToDestroy();
		return;
	}

	GetIdTokenInternal();
}

void UEOSGetIdTokenAsync::GetIdTokenInternal()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(FEOSKitIdToken());
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(FEOSKitIdToken());
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(FEOSKitIdToken());
		SetReadyToDestroy();
		return;
	}

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(EOSSubsystem->GetPlatformHandle());
	if (!AuthHandle)
	{
		OnFailure.Broadcast(FEOSKitIdToken());
		SetReadyToDestroy();
		return;
	}

	// Convert Epic Account ID string to EOS_EpicAccountId
	EOS_EpicAccountId EpicAccountId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*EpicAccountIdString));
	if (!EOS_EpicAccountId_IsValid(EpicAccountId))
	{
		OnFailure.Broadcast(FEOSKitIdToken());
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_Auth_CopyIdTokenOptions Options = {};
	Options.ApiVersion = EOS_AUTH_COPYIDTOKEN_API_LATEST;
	Options.AccountId = EpicAccountId;

	// Copy ID token
	EOS_Auth_IdToken* IdToken = nullptr;
	EOS_EResult Result = EOS_Auth_CopyIdToken(AuthHandle, &Options, &IdToken);

	if (Result == EOS_EResult::EOS_Success && IdToken)
	{
		FEOSKitIdToken TokenData;
		TokenData.IdToken_JWT = FString(IdToken->JsonWebToken);

		// Convert Account ID to string
		char AccountIdBuffer[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
		int32_t BufferLength = sizeof(AccountIdBuffer);
		if (EOS_EpicAccountId_ToString(IdToken->AccountId, AccountIdBuffer, &BufferLength) == EOS_EResult::EOS_Success)
		{
			TokenData.EpicAccountId = UTF8_TO_TCHAR(AccountIdBuffer);
		}

		OnSuccess.Broadcast(TokenData);

		// Release the ID token
		EOS_Auth_IdToken_Release(IdToken);
		SetReadyToDestroy();
	}
	else
	{
		OnFailure.Broadcast(FEOSKitIdToken());
		SetReadyToDestroy();
	}
}

