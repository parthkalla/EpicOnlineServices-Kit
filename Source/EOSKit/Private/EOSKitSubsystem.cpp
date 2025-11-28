// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitSubsystem.h"
#include "EOSKitSettings.h"
#include "IEOSSDKManager.h"
#include "EOSShared.h"
#include "Kismet/GameplayStatics.h"

void UEOSKitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PlatformHandle = nullptr;
	ProductUserId = nullptr;
	LoginNotificationId = 0;
	CreateEOSPlatform();

	// Register for Connect login status changes
	if (PlatformHandle)
	{
		RegisterForConnectLoginStatusChanges();
	}

	// Auto-login if configured
	const UEOSKitSettings* Settings = GetDefault<UEOSKitSettings>();
	if (Settings && Settings->bAutomaticallySetupEIK && Settings->bEnableAutoLogin && PlatformHandle)
	{
		PerformAutoLogin();
	}
}

void UEOSKitSubsystem::Deinitialize()
{
	// Unregister notifications
	if (PlatformHandle && LoginNotificationId != 0)
	{
		EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
		if (ConnectHandle)
		{
			EOS_Connect_RemoveNotifyAuthExpiration(ConnectHandle, LoginNotificationId);
			LoginNotificationId = 0;
		}
	}

	if (PlatformHandle)
	{
		EOS_Platform_Release(PlatformHandle);
		PlatformHandle = nullptr;
	}
	ProductUserId = nullptr;
	Super::Deinitialize();
}

void UEOSKitSubsystem::Tick(float DeltaTime)
{
	if (PlatformHandle)
	{
		EOS_Platform_Tick(PlatformHandle);
	}
}

TStatId UEOSKitSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UEOSKitSubsystem, STATGROUP_Tickables);
}

EOS_ProductUserId UEOSKitSubsystem::GetProductUserId() const
{
	return ProductUserId;
}

void UEOSKitSubsystem::RegisterForConnectLoginStatusChanges()
{
	if (!PlatformHandle)
	{
		return;
	}

	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	if (!ConnectHandle)
	{
		return;
	}

	// Add notification for login status changes
	EOS_Connect_AddNotifyLoginStatusChangedOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST;

	LoginNotificationId = EOS_Connect_AddNotifyLoginStatusChanged(ConnectHandle, &Options, this, &UEOSKitSubsystem::OnConnectLoginStatusChanged);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Registered for Connect login status changes"));
}

void EOS_CALL UEOSKitSubsystem::OnConnectLoginStatusChanged(const EOS_Connect_LoginStatusChangedCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSubsystem* Self = static_cast<UEOSKitSubsystem*>(Data->ClientData);

	if (Data->CurrentStatus == EOS_ELoginStatus::EOS_LS_LoggedIn)
	{
		// User logged in - cache the ProductUserId
		Self->ProductUserId = Data->LocalUserId;

		// Log for debugging
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
		EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);
		
		UE_LOG(LogTemp, Log, TEXT("? EOSKit: User logged in! Product User ID: %s"), UTF8_TO_TCHAR(ProductUserIdStr));
	}
	else if (Data->CurrentStatus == EOS_ELoginStatus::EOS_LS_NotLoggedIn)
	{
		// User logged out
		Self->ProductUserId = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("?? EOSKit: User logged out"));
	}
}

void UEOSKitSubsystem::CreateEOSPlatform()
{
	const UEOSKitSettings* Settings = GetDefault<UEOSKitSettings>();
	if (!Settings) 
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit Settings not found"));
		return;
	}

	// Get the active artifact for the current platform
	FEOSArtifact ActiveArtifact = Settings->GetActiveArtifact();

	// EOS SDK is already initialized by EOSShared plugin, so we don't call EOS_Initialize
	// We just need to create the platform handle
	
	EOS_Platform_Options PlatformOptions = {};
	PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	
	// Use active artifact credentials
	FTCHARToUTF8 ProductId(*ActiveArtifact.ProductId);
	FTCHARToUTF8 SandboxId(*ActiveArtifact.SandboxId);
	FTCHARToUTF8 DeploymentId(*ActiveArtifact.DeploymentId);
	FTCHARToUTF8 ClientId(*ActiveArtifact.ClientId);
	FTCHARToUTF8 ClientSecret(*ActiveArtifact.ClientSecret);
	FTCHARToUTF8 EncKey(*ActiveArtifact.EncryptionKey);

	PlatformOptions.ProductId = ProductId.Get();
	PlatformOptions.SandboxId = SandboxId.Get();
	PlatformOptions.DeploymentId = DeploymentId.Get();
	PlatformOptions.ClientCredentials.ClientId = ClientId.Get();
	PlatformOptions.ClientCredentials.ClientSecret = ClientSecret.Get();
	PlatformOptions.EncryptionKey = EncKey.Get();
	PlatformOptions.bIsServer = false;
	PlatformOptions.Flags = 0; // Initialize flags

	// Apply tick budget if configured
	if (Settings->TickBudgetInMilliseconds > 0)
	{
		PlatformOptions.TickBudgetInMilliseconds = Settings->TickBudgetInMilliseconds;
	}

	// Handle overlay settings
	// Mobile platforms (Android/iOS) do not support the PC overlay, so force system browser
#if PLATFORM_ANDROID || PLATFORM_IOS
	// Force disable overlay on mobile - mobile uses system browser for login
	PlatformOptions.Flags |= EOS_PF_DISABLE_OVERLAY;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Mobile platform detected - forcing system browser login (overlay disabled)"));
#else
	// PC/Mac/Console: Use the setting from EOSKitSettings
	if (!Settings->bEnableOverlay)
	{
		PlatformOptions.Flags |= EOS_PF_DISABLE_OVERLAY;
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Overlay disabled by settings"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Overlay enabled"));
	}
#endif

	// Check if we have valid IDs before creating
	if (PlatformOptions.ProductId && strlen(PlatformOptions.ProductId) > 0 && 
		PlatformOptions.SandboxId && strlen(PlatformOptions.SandboxId) > 0 && 
		PlatformOptions.DeploymentId && strlen(PlatformOptions.DeploymentId) > 0)
	{
		PlatformHandle = EOS_Platform_Create(&PlatformOptions);
		
		if (PlatformHandle)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Platform created successfully using artifact: %s"), *ActiveArtifact.ArtifactName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create EOS Platform"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Platform credentials are incomplete. Please configure them in Project Settings > Plugins > EOS Kit"));
		UE_LOG(LogTemp, Warning, TEXT("  ProductId: %s"), *ActiveArtifact.ProductId);
		UE_LOG(LogTemp, Warning, TEXT("  SandboxId: %s"), *ActiveArtifact.SandboxId);
		UE_LOG(LogTemp, Warning, TEXT("  DeploymentId: %s"), *ActiveArtifact.DeploymentId);
	}
}

void UEOSKitSubsystem::PerformAutoLogin()
{
	const UEOSKitSettings* Settings = GetDefault<UEOSKitSettings>();
	if (!Settings || !PlatformHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Cannot perform auto-login - Settings or Platform not available"));
		return;
	}

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(PlatformHandle);
	if (!AuthHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Auth Interface for auto-login"));
		return;
	}

	// Convert credential type string to EOS enum
	EOS_ELoginCredentialType CredentialType = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
	
	if (Settings->AutoLoginCredentialType == TEXT("AccountPortal"))
	{
		CredentialType = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
	}
	else if (Settings->AutoLoginCredentialType == TEXT("PersistentAuth"))
	{
		CredentialType = EOS_ELoginCredentialType::EOS_LCT_PersistentAuth;
	}
	else if (Settings->AutoLoginCredentialType == TEXT("Developer"))
	{
		CredentialType = EOS_ELoginCredentialType::EOS_LCT_Developer;
	}
	else if (Settings->AutoLoginCredentialType == TEXT("DeviceCode"))
	{
		CredentialType = EOS_ELoginCredentialType::EOS_LCT_DeviceCode;
	}
	else if (Settings->AutoLoginCredentialType == TEXT("ExchangeCode"))
	{
		CredentialType = EOS_ELoginCredentialType::EOS_LCT_ExchangeCode;
	}

	// Setup credentials
	EOS_Auth_Credentials Credentials = {};
	Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	Credentials.Type = CredentialType;

	FTCHARToUTF8 IdConverter(*Settings->AutoLoginCredentialId);
	FTCHARToUTF8 TokenConverter(*Settings->AutoLoginCredentialToken);

	Credentials.Id = Settings->AutoLoginCredentialId.IsEmpty() ? nullptr : IdConverter.Get();
	Credentials.Token = Settings->AutoLoginCredentialToken.IsEmpty() ? nullptr : TokenConverter.Get();

	// Setup login options
	EOS_Auth_LoginOptions LoginOptions = {};
	LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	LoginOptions.Credentials = &Credentials;

	UE_LOG(LogTemp, Log, TEXT("EOSKit: Starting auto-login with credential type: %s"), *Settings->AutoLoginCredentialType);

	// Start Auth login
	EOS_Auth_Login(AuthHandle, &LoginOptions, this, &UEOSKitSubsystem::OnAutoLoginComplete);
}

void EOS_CALL UEOSKitSubsystem::OnAutoLoginComplete(const EOS_Auth_LoginCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSubsystem* Self = static_cast<UEOSKitSubsystem*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Auto-login successful"));
		
		// Convert Epic Account ID to string for logging
		char EpicAccountIdStr[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
		int32 EpicAccountIdStrSize = sizeof(EpicAccountIdStr);
		EOS_EpicAccountId_ToString(Data->LocalUserId, EpicAccountIdStr, &EpicAccountIdStrSize);
		
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Epic Account ID: %s"), UTF8_TO_TCHAR(EpicAccountIdStr));

		// Now login to Connect interface
		Self->StartAutoConnectLogin(Data->LocalUserId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Auto-login failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
	}
}

void UEOSKitSubsystem::StartAutoConnectLogin(EOS_EpicAccountId EpicAccountId)
{
	if (!PlatformHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Cannot start Connect login - Platform not available"));
		return;
	}

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(PlatformHandle);
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);

	if (!AuthHandle || !ConnectHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get EOS Interfaces for Connect login"));
		return;
	}

	// Copy user auth token
	EOS_Auth_Token* AuthToken = nullptr;
	EOS_Auth_CopyUserAuthTokenOptions CopyTokenOptions = {};
	CopyTokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

	EOS_EResult CopyResult = EOS_Auth_CopyUserAuthToken(AuthHandle, &CopyTokenOptions, EpicAccountId, &AuthToken);
	
	if (CopyResult != EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to copy auth token for Connect login"));
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
	EOS_Connect_Login(ConnectHandle, &ConnectLoginOptions, this, &UEOSKitSubsystem::OnAutoConnectLoginComplete);

	// Release auth token
	EOS_Auth_Token_Release(AuthToken);
}

void EOS_CALL UEOSKitSubsystem::OnAutoConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSubsystem* Self = static_cast<UEOSKitSubsystem*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->ProductUserId = Data->LocalUserId;

		// Convert Product User ID to string for logging
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
		EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);

		UE_LOG(LogTemp, Log, TEXT("EOSKit: Auto-login to Connect successful. Product User ID: %s"), UTF8_TO_TCHAR(ProductUserIdStr));
	}
	else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
	{
		// User doesn't have a product user ID, need to create one
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: User doesn't have a Product User ID, creating one..."));
		
		if (!Self->PlatformHandle)
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Cannot create user - Platform not available"));
			return;
		}

		EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(Self->PlatformHandle);

		EOS_Connect_CreateUserOptions CreateUserOptions = {};
		CreateUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
		CreateUserOptions.ContinuanceToken = Data->ContinuanceToken;

		EOS_Connect_CreateUser(ConnectHandle, &CreateUserOptions, Self, &UEOSKitSubsystem::OnAutoCreateUserComplete);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Auto-login to Connect failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
	}
}

void EOS_CALL UEOSKitSubsystem::OnAutoCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSKitSubsystem* Self = static_cast<UEOSKitSubsystem*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->ProductUserId = Data->LocalUserId;

		// Convert Product User ID to string for logging
		char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
		EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);

		UE_LOG(LogTemp, Log, TEXT("EOSKit: User created successfully. Product User ID: %s"), UTF8_TO_TCHAR(ProductUserIdStr));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create user: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
	}
}
