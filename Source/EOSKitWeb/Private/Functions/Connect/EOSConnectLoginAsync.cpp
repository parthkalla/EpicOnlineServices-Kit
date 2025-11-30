// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Connect/EOSConnectLoginAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_connect.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSConnectLoginAsync* UEOSConnectLoginAsync::ConnectLogin(
	UObject* WorldContextObject,
	const FString& ExternalToken,
	EEOSWebCredentialType Type,
	const FString& DisplayName)
{
	UEOSConnectLoginAsync* Node = NewObject<UEOSConnectLoginAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->Token = ExternalToken;
	Node->CredentialType = Type;
	Node->UserDisplayName = DisplayName;
	return Node;
}

void UEOSConnectLoginAsync::Activate()
{
	PerformConnectLogin();
}

void UEOSConnectLoginAsync::PerformConnectLogin()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT("Failed to get Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT("EOSKit Subsystem not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);

	if (!ConnectHandle)
	{
		OnFail.Broadcast(TEXT("Failed to get Connect interface"));
		SetReadyToDestroy();
		return;
	}

	// Setup credentials
	EOS_Connect_Credentials Credentials = {};
	Credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	Credentials.Type = ConvertCredentialType(CredentialType);

	FTCHARToUTF8 TokenConverter(*Token);
	Credentials.Token = TokenConverter.Get();

	// Setup login options
	EOS_Connect_LoginOptions LoginOptions = {};
	LoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	LoginOptions.Credentials = &Credentials;

	if (!UserDisplayName.IsEmpty())
	{
		// Setup user login info if display name is provided
		EOS_Connect_UserLoginInfo UserLoginInfo = {};
		UserLoginInfo.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
		
		FTCHARToUTF8 DisplayNameConverter(*UserDisplayName);
		UserLoginInfo.DisplayName = DisplayNameConverter.Get();
		
		LoginOptions.UserLoginInfo = &UserLoginInfo;
	}

	UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Attempting Connect login with external credentials"));

	// Perform Connect login
	EOS_Connect_Login(ConnectHandle, &LoginOptions, this, &UEOSConnectLoginAsync::OnConnectLoginCallback);
}

void EOS_CALL UEOSConnectLoginAsync::OnConnectLoginCallback(const EOS_Connect_LoginCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSConnectLoginAsync* Self = static_cast<UEOSConnectLoginAsync*>(Data->ClientData);

	AsyncTask(ENamedThreads::GameThread, [Self, Data]()
	{
		if (Data->ResultCode == EOS_EResult::EOS_Success)
		{
			// Convert Product User ID to string
			char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
			int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
			if (EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
			{
				FString ProductUserId = UTF8_TO_TCHAR(ProductUserIdStr);
				UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Connect login successful. Product User ID: %s"), *ProductUserId);
				Self->OnSuccess.Broadcast(ProductUserId);
			}
			else
			{
				Self->OnFail.Broadcast(TEXT("Failed to convert Product User ID"));
			}
			
			Self->SetReadyToDestroy();
		}
		else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
		{
			// User needs to be created - continue with Create User flow
			UE_LOG(LogTemp, Warning, TEXT("EOSKitWeb: User not found, creating new user..."));

			UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject);
			if (GameInstance)
			{
				UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
				if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
				{
					EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());
					if (ConnectHandle)
					{
						EOS_Connect_CreateUserOptions CreateUserOptions = {};
						CreateUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
						CreateUserOptions.ContinuanceToken = Data->ContinuanceToken;

						EOS_Connect_CreateUser(ConnectHandle, &CreateUserOptions, Self, &UEOSConnectLoginAsync::OnCreateUserCallback);
						return; // Don't destroy yet, waiting for create user callback
					}
				}
			}

			Self->OnFail.Broadcast(TEXT("Failed to create user"));
			Self->SetReadyToDestroy();
		}
		else
		{
			FString ErrorMessage = FString::Printf(TEXT("Connect login failed: %s"), 
				UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
			UE_LOG(LogTemp, Error, TEXT("EOSKitWeb: %s"), *ErrorMessage);
			Self->OnFail.Broadcast(ErrorMessage);
			Self->SetReadyToDestroy();
		}
	});
}

void EOS_CALL UEOSConnectLoginAsync::OnCreateUserCallback(const EOS_Connect_CreateUserCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSConnectLoginAsync* Self = static_cast<UEOSConnectLoginAsync*>(Data->ClientData);

	AsyncTask(ENamedThreads::GameThread, [Self, Data]()
	{
		if (Data->ResultCode == EOS_EResult::EOS_Success)
		{
			// Convert Product User ID to string
			char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
			int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
			if (EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
			{
				FString ProductUserId = UTF8_TO_TCHAR(ProductUserIdStr);
				UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: User created successfully. Product User ID: %s"), *ProductUserId);
				Self->OnSuccess.Broadcast(ProductUserId);
			}
			else
			{
				Self->OnFail.Broadcast(TEXT("Failed to convert Product User ID"));
			}
		}
		else
		{
			FString ErrorMessage = FString::Printf(TEXT("Create user failed: %s"), 
				UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
			UE_LOG(LogTemp, Error, TEXT("EOSKitWeb: %s"), *ErrorMessage);
			Self->OnFail.Broadcast(ErrorMessage);
		}

		Self->SetReadyToDestroy();
	});
}

EOS_EExternalCredentialType UEOSConnectLoginAsync::ConvertCredentialType(EEOSWebCredentialType Type)
{
	switch (Type)
	{
	case EEOSWebCredentialType::Epic:
		return EOS_EExternalCredentialType::EOS_ECT_EPIC;
	case EEOSWebCredentialType::Steam:
		return EOS_EExternalCredentialType::EOS_ECT_STEAM_APP_TICKET;
	case EEOSWebCredentialType::PSN:
		return EOS_EExternalCredentialType::EOS_ECT_PSN_ID_TOKEN;
	case EEOSWebCredentialType::XBL:
		return EOS_EExternalCredentialType::EOS_ECT_XBL_XSTS_TOKEN;
	case EEOSWebCredentialType::Discord:
		return EOS_EExternalCredentialType::EOS_ECT_DISCORD_ACCESS_TOKEN;
	case EEOSWebCredentialType::GOG:
		return EOS_EExternalCredentialType::EOS_ECT_GOG_SESSION_TICKET;
	case EEOSWebCredentialType::Nintendo:
		return EOS_EExternalCredentialType::EOS_ECT_NINTENDO_ID_TOKEN;
	case EEOSWebCredentialType::AppleIdToken:
		return EOS_EExternalCredentialType::EOS_ECT_APPLE_ID_TOKEN;
	case EEOSWebCredentialType::GoogleIdToken:
		return EOS_EExternalCredentialType::EOS_ECT_GOOGLE_ID_TOKEN;
	case EEOSWebCredentialType::OculusUserIdNonce:
		return EOS_EExternalCredentialType::EOS_ECT_OCULUS_USERID_NONCE;
	case EEOSWebCredentialType::ItchioJwt:
		return EOS_EExternalCredentialType::EOS_ECT_ITCHIO_JWT;
	case EEOSWebCredentialType::ItchioKey:
		return EOS_EExternalCredentialType::EOS_ECT_ITCHIO_KEY;
	case EEOSWebCredentialType::EpicIdToken:
		return EOS_EExternalCredentialType::EOS_ECT_EPIC_ID_TOKEN;
	case EEOSWebCredentialType::AmazonAccessToken:
		return EOS_EExternalCredentialType::EOS_ECT_AMAZON_ACCESS_TOKEN;
	case EEOSWebCredentialType::OpenIdAccessToken:
		return EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN;
	default:
		return EOS_EExternalCredentialType::EOS_ECT_EPIC;
	}
}
