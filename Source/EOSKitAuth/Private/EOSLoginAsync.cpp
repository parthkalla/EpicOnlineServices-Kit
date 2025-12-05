// Copyright (C) 2024, All Rights Reserved.

#include "EOSLoginAsync.h"
#include "EOSKitSubsystem.h"
#include "IEOSSDKManager.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemTypes.h"
#include "Misc/DateTime.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_auth.h"
#include "eos_connect.h"
#include "eos_userinfo.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

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

		// Query UserInfo to cache display name for GetPlayerNickname
		// IMPORTANT: Wait for this to complete before triggering OnLoginComplete
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject))
		{
			if (UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>())
			{
				EOS_HUserInfo UserInfoHandle = EOSKitSubsystem->GetUserInfoHandle();
				if (UserInfoHandle && Self->CachedEpicAccountId)
				{
					EOS_UserInfo_QueryUserInfoOptions QueryOptions = {};
					QueryOptions.ApiVersion = EOS_USERINFO_QUERYUSERINFO_API_LATEST;
					QueryOptions.LocalUserId = Self->CachedEpicAccountId;
					QueryOptions.TargetUserId = Self->CachedEpicAccountId;
					
					// Capture context for the callback
					struct FQueryUserInfoContext
					{
						UEOSLoginUsingAuthInterface* LoginNode;
						FString EpicIdStr;
						FString ProductIdStr;
						EOS_HUserInfo UserInfoHandle;
						EOS_EpicAccountId EpicAccountId;
					};
					
					FQueryUserInfoContext* Context = new FQueryUserInfoContext();
					Context->LoginNode = Self;
					Context->EpicIdStr = EpicId;
					Context->ProductIdStr = ProductId;
					Context->UserInfoHandle = UserInfoHandle;
					Context->EpicAccountId = Self->CachedEpicAccountId;
					
					// Add to root to prevent GC
					Self->AddToRoot();
					
					EOS_UserInfo_QueryUserInfo(UserInfoHandle, &QueryOptions, Context,
						[](const EOS_UserInfo_QueryUserInfoCallbackInfo* QueryData)
						{
							FQueryUserInfoContext* Ctx = static_cast<FQueryUserInfoContext*>(QueryData->ClientData);
							FString DisplayName;
							
							if (QueryData->ResultCode == EOS_EResult::EOS_Success && Ctx)
							{
								// Copy UserInfo to get DisplayName
								EOS_UserInfo_CopyUserInfoOptions CopyOptions = {};
								CopyOptions.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
								CopyOptions.LocalUserId = Ctx->EpicAccountId;
								CopyOptions.TargetUserId = Ctx->EpicAccountId;
								
								EOS_UserInfo* UserInfo = nullptr;
								EOS_EResult CopyResult = EOS_UserInfo_CopyUserInfo(Ctx->UserInfoHandle, &CopyOptions, &UserInfo);
								
								if (CopyResult == EOS_EResult::EOS_Success && UserInfo)
								{
									DisplayName = UTF8_TO_TCHAR(UserInfo->DisplayName);
									EOS_UserInfo_Release(UserInfo);
									
									UE_LOG(LogTemp, Log, TEXT("EOSKit: ✅ UserInfo retrieved - DisplayName: %s"), *DisplayName);
									
									// Store DisplayName in OnlineIdentity for GetPlayerNickname
									if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
									{
										if (IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
										{
											// Trigger with DisplayName as the "Error" field temporarily to pass it through
											// We'll extract it in OnLoginCompleteInternal
											FString TempData = FString::Printf(TEXT("DISPLAYNAME:%s"), *DisplayName);
											Identity->TriggerOnLoginCompleteDelegates(0, true, *FUniqueNetIdString::Create(Ctx->ProductIdStr, FName(TEXT("EOS"))), TempData);
										}
									}
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to copy UserInfo: %s"), 
										UTF8_TO_TCHAR(EOS_EResult_ToString(CopyResult)));
								}
							}
							else
							{
								UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to query UserInfo: %s"), 
									UTF8_TO_TCHAR(EOS_EResult_ToString(QueryData->ResultCode)));
							}
							
							// Now trigger login complete (UserInfo is cached)
							if (Ctx && Ctx->LoginNode)
							{
								// If we didn't get DisplayName via Identity delegate, trigger it normally
								if (DisplayName.IsEmpty())
								{
									if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
									{
										if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
										{
											Identity->TriggerOnLoginCompleteDelegates(0, true, *FUniqueNetIdString::Create(Ctx->ProductIdStr, FName(TEXT("EOS"))), TEXT(""));
										}
									}
								}
								
								Ctx->LoginNode->OnSuccess.Broadcast(Ctx->EpicIdStr, Ctx->ProductIdStr, TEXT(""));
								Ctx->LoginNode->RemoveFromRoot();
								Ctx->LoginNode->SetReadyToDestroy();
							}
							
							delete Ctx;
						});
					
					// Return early - callback will handle completion
					return;
				}
			}
		}

		// Fallback if UserInfo query couldn't be started
		if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				Identity->TriggerOnLoginCompleteDelegates(0, true, *FUniqueNetIdString::Create(ProductId, FName(TEXT("EOS"))), TEXT(""));
			}
		}

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

		// Query UserInfo to cache display name for GetPlayerNickname
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject))
		{
			if (UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>())
			{
				EOS_HUserInfo UserInfoHandle = EOSKitSubsystem->GetUserInfoHandle();
				if (UserInfoHandle && Self->CachedEpicAccountId)
				{
					EOS_UserInfo_QueryUserInfoOptions QueryOptions = {};
					QueryOptions.ApiVersion = EOS_USERINFO_QUERYUSERINFO_API_LATEST;
					QueryOptions.LocalUserId = Self->CachedEpicAccountId;
					QueryOptions.TargetUserId = Self->CachedEpicAccountId;
					
					EOS_UserInfo_QueryUserInfo(UserInfoHandle, &QueryOptions, nullptr,
						[](const EOS_UserInfo_QueryUserInfoCallbackInfo* QueryData)
						{
							if (QueryData->ResultCode == EOS_EResult::EOS_Success)
							{
								UE_LOG(LogTemp, Log, TEXT("EOSKit: ✅ UserInfo queried - display name now cached"));
							}
						});
				}
			}
		}

		// Notify OnlineSubsystem that login succeeded
		if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface())
			{
				// Trigger OnLoginComplete delegate so OnlineIdentity knows user is logged in
				Identity->TriggerOnLoginCompleteDelegates(0, true, *FUniqueNetIdString::Create(ProductId, FName(TEXT("EOS"))), TEXT(""));
			}
		}

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
	EEOSKitExternalCredentialType LoginMethod,
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

	// Setup Connect credentials based on login method
	EOS_Connect_Credentials ConnectCredentials = {};
	ConnectCredentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	
	// Convert enum to EOS credential type
	switch (LoginMethod)
	{
	case EEOSKitExternalCredentialType::DeviceID:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
		ConnectCredentials.Token = nullptr; // Device ID doesn't use token
		break;
	case EEOSKitExternalCredentialType::Epic:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;
		break;
	case EEOSKitExternalCredentialType::Steam:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_STEAM_APP_TICKET;
		break;
	case EEOSKitExternalCredentialType::PSN:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_PSN_ID_TOKEN;
		break;
	case EEOSKitExternalCredentialType::XBL:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_XBL_XSTS_TOKEN;
		break;
	case EEOSKitExternalCredentialType::Discord:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_DISCORD_ACCESS_TOKEN;
		break;
	case EEOSKitExternalCredentialType::Nintendo:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_NINTENDO_ID_TOKEN;
		break;
	case EEOSKitExternalCredentialType::Apple:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_APPLE_ID_TOKEN;
		break;
	case EEOSKitExternalCredentialType::Google:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_GOOGLE_ID_TOKEN;
		break;
	case EEOSKitExternalCredentialType::Oculus:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_OCULUS_USERID_NONCE;
		break;
	case EEOSKitExternalCredentialType::OpenID:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN;
		break;
	default:
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
		ConnectCredentials.Token = nullptr;
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("EOSKit: Connect Login - Method: %d, DisplayName: %s"), (int32)LoginMethod, *DisplayName);

	// Setup UserLoginInfo with persistent buffer for DisplayName
	EOS_Connect_UserLoginInfo UserLoginInfo = {};
	UserLoginInfo.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
	
	// Store DisplayName in persistent buffer
	FString DisplayNameToUse = DisplayName.IsEmpty() ? TEXT("Player") : DisplayName;
	DisplayNameAnsi.SetNumUninitialized(DisplayNameToUse.Len() + 1);
	FCStringAnsi::Strcpy((char*)DisplayNameAnsi.GetData(), DisplayNameAnsi.Num(), TCHAR_TO_UTF8(*DisplayNameToUse));
	UserLoginInfo.DisplayName = (char*)DisplayNameAnsi.GetData();

	// Setup Connect login options
	EOS_Connect_LoginOptions ConnectLoginOptions = {};
	ConnectLoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	ConnectLoginOptions.Credentials = &ConnectCredentials;
	ConnectLoginOptions.UserLoginInfo = &UserLoginInfo;

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
	else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser || Data->ResultCode == EOS_EResult::EOS_NotFound)
	{
		// Need to create a device ID first
		// EOS_InvalidUser: Device ID exists but is invalid
		// EOS_NotFound: Device ID doesn't exist (common in packaged builds on new PCs)
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Device ID not found or invalid (%s), creating new one"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
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

		// CRITICAL: Delete existing Device ID first to ensure unique ProductUserId per login
		// This is necessary for testing multiple players on the same PC
		EOS_Connect_DeleteDeviceIdOptions DeleteOptions = {};
		DeleteOptions.ApiVersion = EOS_CONNECT_DELETEDEVICEID_API_LATEST;
		
		// Delete first, then create in the callback
		EOS_Connect_DeleteDeviceId(ConnectHandle, &DeleteOptions, Self,
			[](const EOS_Connect_DeleteDeviceIdCallbackInfo* DeleteData)
			{
				if (!DeleteData || !DeleteData->ClientData) return;
				
				UEOSLoginUsingConnectInterface* LoginNode = static_cast<UEOSLoginUsingConnectInterface*>(DeleteData->ClientData);
				
				// Log result (success or "not found" both mean we can proceed)
				if (DeleteData->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Deleted existing Device ID"));
				}
				else if (DeleteData->ResultCode == EOS_EResult::EOS_NotFound)
				{
					UE_LOG(LogTemp, Log, TEXT("EOSKit: No existing Device ID to delete"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: DeleteDeviceId returned: %s"), 
						UTF8_TO_TCHAR(EOS_EResult_ToString(DeleteData->ResultCode)));
				}
				
				// Now create a new Device ID with unique DeviceModel
				UGameInstance* GI = UGameplayStatics::GetGameInstance(LoginNode->WorldContextObject);
				if (!GI) return;
				
				UEOSKitSubsystem* Subsystem = GI->GetSubsystem<UEOSKitSubsystem>();
				if (!Subsystem) return;
				
				EOS_HConnect Connect = EOS_Platform_GetConnectInterface(Subsystem->GetPlatformHandle());
				if (!Connect) return;
				
				// Create device ID with persistent buffer
				EOS_Connect_CreateDeviceIdOptions CreateDeviceIdOptions = {};
				CreateDeviceIdOptions.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
				
				// Use a timestamp + random to ensure uniqueness
				FString UniqueId = FString::Printf(TEXT("%lld_%d"), FDateTime::Now().GetTicks(), FMath::Rand());
				FString DeviceModel = LoginNode->DisplayName.IsEmpty() 
					? FString::Printf(TEXT("PC_Player_%s"), *UniqueId)
					: FString::Printf(TEXT("PC_%s_%s"), *LoginNode->DisplayName, *UniqueId);
				
				LoginNode->DeviceModelAnsi.SetNumUninitialized(DeviceModel.Len() + 1);
				FCStringAnsi::Strcpy((char*)LoginNode->DeviceModelAnsi.GetData(), LoginNode->DeviceModelAnsi.Num(), TCHAR_TO_UTF8(*DeviceModel));
				CreateDeviceIdOptions.DeviceModel = (char*)LoginNode->DeviceModelAnsi.GetData();
				
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: Creating NEW Device ID with unique DeviceModel: %s"), *DeviceModel);
				
				EOS_Connect_CreateDeviceId(Connect, &CreateDeviceIdOptions, LoginNode, &UEOSLoginUsingConnectInterface::OnCreateDeviceIdComplete);
			});
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
