// Copyright (C) 2024, All Rights Reserved.

#include "EOSLoginAsync.h"
#include "EOSKitSubsystem.h"
#include "IEOSSDKManager.h"
#include "Kismet/GameplayStatics.h"
#include "eos_auth.h"
#include "eos_connect.h"

//////////////////////////////////////////////////////////////////////////
// UEOSLoginUsingAuthInterface
//////////////////////////////////////////////////////////////////////////

UEOSLoginUsingAuthInterface* UEOSLoginUsingAuthInterface::LoginUsingAuthInterface(
	UObject* WorldContextObject,
	EEOSCredentialType CredentialType,
	EEOSExternalCredentialType ExternalCredentialType,
	FString Id,
	FString Token)
{
	UEOSLoginUsingAuthInterface* Node = NewObject<UEOSLoginUsingAuthInterface>();
	Node->WorldContextObject = WorldContextObject;
	Node->CredentialType = CredentialType;
	Node->ExternalCredentialType = ExternalCredentialType;
	Node->Id = Id;
	Node->Token = Token;
	return Node;
}

void UEOSLoginUsingAuthInterface::Activate()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("EOS Platform not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(EOSSubsystem->GetPlatformHandle());
	if (!AuthHandle)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Failed to get Auth Interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert credential type
	EOS_ELoginCredentialType LoginCredType;
	switch (CredentialType)
	{
	case EEOSCredentialType::AccountPortal:
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
		break;
	case EEOSCredentialType::PersistentAuth:
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_PersistentAuth;
		break;
	case EEOSCredentialType::Developer:
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_Developer;
		break;
	case EEOSCredentialType::DeviceCode:
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_DeviceCode;
		break;
	case EEOSCredentialType::ExchangeCode:
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_ExchangeCode;
		break;
	default:
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
		break;
	}

	// Setup credentials
	EOS_Auth_Credentials Credentials = {};
	Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	Credentials.Type = LoginCredType;

	FTCHARToUTF8 IdConverter(*Id);
	FTCHARToUTF8 TokenConverter(*Token);

	Credentials.Id = Id.IsEmpty() ? nullptr : IdConverter.Get();
	Credentials.Token = Token.IsEmpty() ? nullptr : TokenConverter.Get();

	// Setup login options
	EOS_Auth_LoginOptions LoginOptions = {};
	LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	LoginOptions.Credentials = &Credentials;

	// Start Auth login
	EOS_Auth_Login(AuthHandle, &LoginOptions, this, &UEOSLoginUsingAuthInterface::OnAuthLoginComplete);
}

void EOS_CALL UEOSLoginUsingAuthInterface::OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSLoginUsingAuthInterface* Self = static_cast<UEOSLoginUsingAuthInterface*>(Data->ClientData);
	
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->CachedEpicAccountId = Data->LocalUserId;
		Self->StartConnectLogin(Data->LocalUserId);
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Auth Login Failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFail.Broadcast(TEXT(""), TEXT(""), ErrorMessage);
		Self->SetReadyToDestroy();
	}
}

void UEOSLoginUsingAuthInterface::StartConnectLogin(EOS_EpicAccountId EpicAccountId)
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("EOS Platform not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(EOSSubsystem->GetPlatformHandle());
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());

	if (!AuthHandle || !ConnectHandle)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Failed to get EOS Interfaces"));
		SetReadyToDestroy();
		return;
	}

	// Copy user auth token
	EOS_Auth_Token* AuthToken = nullptr;
	EOS_Auth_CopyUserAuthTokenOptions CopyTokenOptions = {};
	CopyTokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

	EOS_EResult CopyResult = EOS_Auth_CopyUserAuthToken(AuthHandle, &CopyTokenOptions, EpicAccountId, &AuthToken);
	
	if (CopyResult != EOS_EResult::EOS_Success)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Failed to copy auth token"));
		SetReadyToDestroy();
		return;
	}

	// Setup Connect credentials
	EOS_Connect_Credentials ConnectCredentials = {};
	ConnectCredentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;
	ConnectCredentials.Token = AuthToken->AccessToken;

	// Setup Connect login options
	EOS_Connect_LoginOptions ConnectLoginOptions = {};
	ConnectLoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	ConnectLoginOptions.Credentials = &ConnectCredentials;

	// Start Connect login
	EOS_Connect_Login(ConnectHandle, &ConnectLoginOptions, this, &UEOSLoginUsingAuthInterface::OnConnectLoginComplete);

	// Release auth token
	EOS_Auth_Token_Release(AuthToken);
}

void EOS_CALL UEOSLoginUsingAuthInterface::OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSLoginUsingAuthInterface* Self = static_cast<UEOSLoginUsingAuthInterface*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Convert IDs to strings
		char EpicAccountIdStr[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
		int32 EpicAccountIdStrSize = sizeof(EpicAccountIdStr);
		
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);

		EOS_EpicAccountId_ToString(Self->CachedEpicAccountId, EpicAccountIdStr, &EpicAccountIdStrSize);
		EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);

		FString EpicId = UTF8_TO_TCHAR(EpicAccountIdStr);
		FString ProductId = UTF8_TO_TCHAR(ProductUserIdStr);

		Self->OnSuccess.Broadcast(EpicId, ProductId, TEXT(""));
		Self->SetReadyToDestroy();
	}
	else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
	{
		// User doesn't have a product user ID, need to create one
		if (!Self->WorldContextObject)
		{
			Self->OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid World Context"));
			Self->SetReadyToDestroy();
			return;
		}

		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject);
		if (!GameInstance)
		{
			Self->OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid Game Instance"));
			Self->SetReadyToDestroy();
			return;
		}

		UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
		{
			Self->OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("EOS Platform not initialized"));
			Self->SetReadyToDestroy();
			return;
		}

		EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());

		EOS_Connect_CreateUserOptions CreateUserOptions = {};
		CreateUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
		CreateUserOptions.ContinuanceToken = Data->ContinuanceToken;

		EOS_Connect_CreateUser(ConnectHandle, &CreateUserOptions, Self, &UEOSLoginUsingAuthInterface::OnCreateUserComplete);
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Connect Login Failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFail.Broadcast(TEXT(""), TEXT(""), ErrorMessage);
		Self->SetReadyToDestroy();
	}
}

void EOS_CALL UEOSLoginUsingAuthInterface::OnCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSLoginUsingAuthInterface* Self = static_cast<UEOSLoginUsingAuthInterface*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Convert IDs to strings
		char EpicAccountIdStr[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
		int32 EpicAccountIdStrSize = sizeof(EpicAccountIdStr);
		
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);

		EOS_EpicAccountId_ToString(Self->CachedEpicAccountId, EpicAccountIdStr, &EpicAccountIdStrSize);
		EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);

		FString EpicId = UTF8_TO_TCHAR(EpicAccountIdStr);
		FString ProductId = UTF8_TO_TCHAR(ProductUserIdStr);

		Self->OnSuccess.Broadcast(EpicId, ProductId, TEXT(""));
		Self->SetReadyToDestroy();
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Create User Failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFail.Broadcast(TEXT(""), TEXT(""), ErrorMessage);
		Self->SetReadyToDestroy();
	}
}

//////////////////////////////////////////////////////////////////////////
// UEOSLoginUsingConnectInterface
//////////////////////////////////////////////////////////////////////////

UEOSLoginUsingConnectInterface* UEOSLoginUsingConnectInterface::LoginUsingConnectInterface(
	UObject* WorldContextObject,
	FString LoginMethod,
	FString DisplayName,
	FString Token)
{
	UEOSLoginUsingConnectInterface* Node = NewObject<UEOSLoginUsingConnectInterface>();
	Node->WorldContextObject = WorldContextObject;
	Node->LoginMethod = LoginMethod;
	Node->DisplayName = DisplayName;
	Node->Token = Token;
	return Node;
}

void UEOSLoginUsingConnectInterface::Activate()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("EOS Platform not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());
	if (!ConnectHandle)
	{
		OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Failed to get Connect Interface"));
		SetReadyToDestroy();
		return;
	}

	// Setup Connect credentials (default to Device ID)
	EOS_Connect_Credentials ConnectCredentials = {};
	ConnectCredentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;

	FTCHARToUTF8 TokenConverter(*Token);
	ConnectCredentials.Token = Token.IsEmpty() ? nullptr : TokenConverter.Get();

	// Setup Connect login options
	EOS_Connect_LoginOptions ConnectLoginOptions = {};
	ConnectLoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	ConnectLoginOptions.Credentials = &ConnectCredentials;

	// Start Connect login
	EOS_Connect_Login(ConnectHandle, &ConnectLoginOptions, this, &UEOSLoginUsingConnectInterface::OnConnectLoginComplete);
}

void EOS_CALL UEOSLoginUsingConnectInterface::OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSLoginUsingConnectInterface* Self = static_cast<UEOSLoginUsingConnectInterface*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Convert Product User ID to string
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
		EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);

		FString ProductId = UTF8_TO_TCHAR(ProductUserIdStr);

		Self->OnSuccess.Broadcast(TEXT(""), ProductId, TEXT(""));
		Self->SetReadyToDestroy();
	}
	else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
	{
		// Need to create a device ID first
		if (!Self->WorldContextObject)
		{
			Self->OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid World Context"));
			Self->SetReadyToDestroy();
			return;
		}

		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject);
		if (!GameInstance)
		{
			Self->OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("Invalid Game Instance"));
			Self->SetReadyToDestroy();
			return;
		}

		UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
		if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
		{
			Self->OnFail.Broadcast(TEXT(""), TEXT(""), TEXT("EOS Platform not initialized"));
			Self->SetReadyToDestroy();
			return;
		}

		EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());

		// Create device ID
		EOS_Connect_CreateDeviceIdOptions CreateDeviceIdOptions = {};
		CreateDeviceIdOptions.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
		
		FTCHARToUTF8 DeviceModelConverter(TEXT("PC"));
		CreateDeviceIdOptions.DeviceModel = DeviceModelConverter.Get();

		EOS_Connect_CreateDeviceId(ConnectHandle, &CreateDeviceIdOptions, Self, &UEOSLoginUsingConnectInterface::OnCreateDeviceIdComplete);
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Connect Login Failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFail.Broadcast(TEXT(""), TEXT(""), ErrorMessage);
		Self->SetReadyToDestroy();
	}
}

void EOS_CALL UEOSLoginUsingConnectInterface::OnCreateDeviceIdComplete(const EOS_Connect_CreateDeviceIdCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSLoginUsingConnectInterface* Self = static_cast<UEOSLoginUsingConnectInterface*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success || Data->ResultCode == EOS_EResult::EOS_DuplicateNotAllowed)
	{
		// Device ID created, try login again by reactivating
		Self->Activate();
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Create Device ID Failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFail.Broadcast(TEXT(""), TEXT(""), ErrorMessage);
		Self->SetReadyToDestroy();
	}
}

void EOS_CALL UEOSLoginUsingConnectInterface::OnCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSLoginUsingConnectInterface* Self = static_cast<UEOSLoginUsingConnectInterface*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Convert Product User ID to string
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
		EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);

		FString ProductId = UTF8_TO_TCHAR(ProductUserIdStr);

		Self->OnSuccess.Broadcast(TEXT(""), ProductId, TEXT(""));
		Self->SetReadyToDestroy();
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Create User Failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFail.Broadcast(TEXT(""), TEXT(""), ErrorMessage);
		Self->SetReadyToDestroy();
	}
}
