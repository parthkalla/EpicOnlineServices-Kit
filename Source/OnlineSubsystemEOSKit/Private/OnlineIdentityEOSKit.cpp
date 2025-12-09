// Copyright (C) 2024, All Rights Reserved.

#include "OnlineIdentityEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitSubsystem.h"
#include "EOSLoginAsync.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "OnlineSubsystemTypes.h"
#include "Misc/DateTime.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"

#if WITH_EOS_SDK
	#include "eos_userinfo.h"
	#include "eos_userinfo_types.h"
#endif

#if WITH_EOS_SDK

FOnlineIdentityEOSKit::FOnlineIdentityEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, AuthHandle(nullptr)
	, ConnectHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		AuthHandle = EOSKitSubsystem->AuthHandle;
		ConnectHandle = EOSKitSubsystem->ConnectHandle;
	}

	// Listen to our own login complete delegate to populate UserAccounts
	OnLoginCompleteDelegates[0].AddRaw(this, &FOnlineIdentityEOSKit::OnLoginCompleteInternal);
}

FOnlineIdentityEOSKit::~FOnlineIdentityEOSKit()
{
	// Cleanup
	OnLoginCompleteDelegates[0].RemoveAll(this);
}

void FOnlineIdentityEOSKit::OnLoginCompleteInternal(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		// Extract DisplayName if passed through Error parameter (for Epic Auth)
		FString DisplayName;
		if (Error.StartsWith(TEXT("DISPLAYNAME:")))
		{
			DisplayName = Error.RightChop(12); // Remove "DISPLAYNAME:" prefix
			LocalUserNumToDisplayName.Add(LocalUserNum, DisplayName);
			UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: Extracted DisplayName from Epic Auth: %s"), *DisplayName);
		}
		else if (LocalUserNumToDisplayName.Contains(LocalUserNum))
		{
			DisplayName = LocalUserNumToDisplayName[LocalUserNum];
		}
		
		// Create a minimal user account and store it
		FScopeLock ScopeLock(&AccountLock);
		
		// Create a simple account wrapper - FUserOnlineAccount is an interface, so we use a basic implementation
		class FUserOnlineAccountEOSKit : public FUserOnlineAccount
		{
		public:
			explicit FUserOnlineAccountEOSKit(const TSharedRef<const FUniqueNetId>& InUserId, const FString& InDisplayName = FString())
				: UserId(InUserId)
				, DisplayName(InDisplayName.IsEmpty() ? InUserId->ToString() : InDisplayName)
			{
			}

			virtual TSharedRef<const FUniqueNetId> GetUserId() const override { return UserId; }
			virtual FString GetRealName() const override { return DisplayName; }
			virtual FString GetDisplayName(const FString& Platform = FString()) const override { return DisplayName; }
			virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override 
			{ 
				if (AttrName == TEXT("DisplayName"))
				{
					OutAttrValue = DisplayName;
					return true;
				}
				return false; 
			}
			virtual bool SetUserAttribute(const FString& AttrName, const FString& AttrValue) override 
			{ 
				if (AttrName == TEXT("DisplayName"))
				{
					DisplayName = AttrValue;
					return true;
				}
				return false; 
			}
			virtual FString GetAccessToken() const override { return FString(); }
			virtual bool GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const override { return false; }

		private:
			TSharedRef<const FUniqueNetId> UserId;
			FString DisplayName;
		};

		TSharedRef<FUserOnlineAccount> UserAccount = MakeShared<FUserOnlineAccountEOSKit>(UserId.AsShared(), DisplayName);
		UserAccounts.Add(UserId.AsShared(), UserAccount);
		
		UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: User logged in successfully, UserId=%s, DisplayName=%s"), 
			*UserId.ToString(), *DisplayName);
	}
}

bool FOnlineIdentityEOSKit::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit::Login: LocalUserNum=%d, Type=%s"), LocalUserNum, *AccountCredentials.Type);
	
	// Store DisplayName for GetPlayerNickname (only for Device ID where we have it immediately)
	// For Epic Auth, we'll get it from EOS UserInfo later
	if (AccountCredentials.Type.StartsWith(TEXT("noeas_+_")) && !AccountCredentials.Id.IsEmpty())
	{
		LocalUserNumToDisplayName.Add(LocalUserNum, AccountCredentials.Id);
	}
	
	// Check if this is a Connect login (noeas_+_CredentialType)
	if (AccountCredentials.Type.StartsWith(TEXT("noeas_+_")))
	{
		// Extract credential type
		FString CredentialType = AccountCredentials.Type.RightChop(8); // Remove "noeas_+_"
		
		if (!ConnectHandle)
		{
			UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit::Login: Connect handle is null"));
			TriggerOnLoginCompleteDelegates(LocalUserNum, false, *FUniqueNetIdString::EmptyId(), TEXT("Connect handle is null"));
			return false;
		}
		
		// Setup Connect credentials
		EOS_Connect_Credentials Credentials = {};
		Credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
		
		// Map credential type string to EOS enum
		if (CredentialType == TEXT("DeviceID"))
		{
			Credentials.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
			Credentials.Token = nullptr;
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("FOnlineIdentityEOSKit::Login: Unsupported credential type: %s, defaulting to DeviceID"), *CredentialType);
			Credentials.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
			Credentials.Token = nullptr;
		}
		
		// Setup UserLoginInfo
		EOS_Connect_UserLoginInfo UserLoginInfo = {};
		UserLoginInfo.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
		
		// Convert DisplayName to ANSI (use AccountCredentials.Id as DisplayName)
		FString DisplayNameStr = AccountCredentials.Id.IsEmpty() ? TEXT("Player") : AccountCredentials.Id;
		const auto DisplayNameAnsi = StringCast<ANSICHAR>(*DisplayNameStr);
		UserLoginInfo.DisplayName = DisplayNameAnsi.Get();
		
		// Setup login options
		EOS_Connect_LoginOptions LoginOptions = {};
		LoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
		LoginOptions.Credentials = &Credentials;
		LoginOptions.UserLoginInfo = &UserLoginInfo;
		
		// Create callback context
		struct FConnectLoginCallback
		{
			FOnlineIdentityEOSKit* Identity;
			int32 LocalUserNum;
			FString DisplayName;
			
			static void EOS_CALL HandleCallback(const EOS_Connect_LoginCallbackInfo* Data)
			{
				if (!Data || !Data->ClientData) return;
				
				FConnectLoginCallback* Context = (FConnectLoginCallback*)Data->ClientData;
				
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					// Convert ProductUserId to string
					char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
					int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
					EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);
					
					FString ProductId = UTF8_TO_TCHAR(ProductUserIdStr);
					UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: Connect login succeeded, ProductUserId=%s"), *ProductId);
					
					// Trigger login complete
					Context->Identity->TriggerOnLoginCompleteDelegates(Context->LocalUserNum, true, *FUniqueNetIdString::Create(ProductId, FName(TEXT("EOS"))), TEXT(""));
				}
				else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser || Data->ResultCode == EOS_EResult::EOS_NotFound)
				{
					// Need to create device ID first
					// EOS_InvalidUser: Device ID exists but is invalid
					// EOS_NotFound: Device ID doesn't exist (common in packaged builds on new PCs)
					UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: Device ID not found or invalid (%s), creating new one"), 
						UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
					Context->Identity->CreateDeviceIdAndRetryLogin(Context->LocalUserNum, Context->DisplayName);
				}
				else
				{
					FString ErrorMsg = FString::Printf(TEXT("Connect login failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
					UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit: %s"), *ErrorMsg);
					Context->Identity->TriggerOnLoginCompleteDelegates(Context->LocalUserNum, false, *FUniqueNetIdString::EmptyId(), ErrorMsg);
				}
				
				delete Context;
			}
		};
		
		FConnectLoginCallback* CallbackContext = new FConnectLoginCallback{ this, LocalUserNum, DisplayNameStr };
		EOS_Connect_Login(ConnectHandle, &LoginOptions, CallbackContext, &FConnectLoginCallback::HandleCallback);
		
		return true;
	}
	
	// Check if this is an Epic Account login (AccountPortal, PersistentAuth, etc.)
	// These require EOS_Auth_Login -> EOS_Connect_Login chain
	EOS_ELoginCredentialType LoginCredType = EOS_ELoginCredentialType::EOS_LCT_Password; // Initialize to a valid enum value
	bool bIsEpicAccountLogin = false;
	
	if (AccountCredentials.Type == TEXT("AccountPortal"))
	{
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
		bIsEpicAccountLogin = true;
	}
	else if (AccountCredentials.Type == TEXT("PersistentAuth") || AccountCredentials.Type == TEXT("persistentauth"))
	{
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_PersistentAuth;
		bIsEpicAccountLogin = true;
	}
	else if (AccountCredentials.Type == TEXT("Developer"))
	{
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_Developer;
		bIsEpicAccountLogin = true;
	}
	else if (AccountCredentials.Type == TEXT("ExchangeCode"))
	{
		LoginCredType = EOS_ELoginCredentialType::EOS_LCT_ExchangeCode;
		bIsEpicAccountLogin = true;
	}
	
	if (bIsEpicAccountLogin)
	{
		// Epic Account login - chain Auth -> Connect
		if (!AuthHandle || !ConnectHandle)
		{
			UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit::Login: Auth or Connect handle is null"));
			TriggerOnLoginCompleteDelegates(LocalUserNum, false, *FUniqueNetIdString::EmptyId(), TEXT("EOS handles not initialized"));
			return false;
		}
		
		// Setup Auth credentials
		EOS_Auth_Credentials AuthCredentials = {};
		AuthCredentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
		AuthCredentials.Type = LoginCredType;
		
		// Convert Id and Token to UTF8
		FTCHARToUTF8 IdConverter(*AccountCredentials.Id);
		FTCHARToUTF8 TokenConverter(*AccountCredentials.Token);
		AuthCredentials.Id = AccountCredentials.Id.IsEmpty() ? nullptr : IdConverter.Get();
		AuthCredentials.Token = AccountCredentials.Token.IsEmpty() ? nullptr : TokenConverter.Get();
		
		// Setup Auth login options
		EOS_Auth_LoginOptions AuthLoginOptions = {};
		AuthLoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
		AuthLoginOptions.Credentials = &AuthCredentials;
		
		// Create callback context for Auth -> Connect chain
		struct FEpicAuthLoginCallback
		{
			FOnlineIdentityEOSKit* Identity;
			int32 LocalUserNum;
			EOS_HAuth AuthHandle;
			EOS_HConnect ConnectHandle;
			
			static void EOS_CALL HandleAuthCallback(const EOS_Auth_LoginCallbackInfo* Data)
			{
				if (!Data || !Data->ClientData) return;
				
				FEpicAuthLoginCallback* Context = (FEpicAuthLoginCallback*)Data->ClientData;
				
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: Epic Auth login succeeded, starting Connect login"));
					
					// Copy auth token for Connect login
					EOS_Auth_Token* AuthToken = nullptr;
					EOS_Auth_CopyUserAuthTokenOptions CopyTokenOptions = {};
					CopyTokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;
					
					EOS_EResult CopyResult = EOS_Auth_CopyUserAuthToken(Context->AuthHandle, &CopyTokenOptions, Data->LocalUserId, &AuthToken);
					
					if (CopyResult == EOS_EResult::EOS_Success && AuthToken)
					{
						// Setup Connect credentials using Epic credential type
						EOS_Connect_Credentials ConnectCredentials = {};
						ConnectCredentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
						ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;
						ConnectCredentials.Token = AuthToken->AccessToken;
						
						// Setup Connect login options
						EOS_Connect_LoginOptions ConnectLoginOptions = {};
						ConnectLoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
						ConnectLoginOptions.Credentials = &ConnectCredentials;
						
						// Create callback context for Connect login
						struct FConnectLoginFromEpicCallback
						{
							FOnlineIdentityEOSKit* Identity;
							int32 LocalUserNum;
							EOS_EpicAccountId EpicAccountId;
							
							static void EOS_CALL HandleConnectCallback(const EOS_Connect_LoginCallbackInfo* Data)
							{
								if (!Data || !Data->ClientData) return;
								
								FConnectLoginFromEpicCallback* Ctx = (FConnectLoginFromEpicCallback*)Data->ClientData;
								
								if (Data->ResultCode == EOS_EResult::EOS_Success)
								{
									// Convert ProductUserId to string
									char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
									int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
									EOS_ProductUserId_ToString(Data->LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);
									
									FString ProductId = UTF8_TO_TCHAR(ProductUserIdStr);
									UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: ✅ Epic Auth -> Connect login chain succeeded, ProductUserId=%s"), *ProductId);
									
									// NOTE: ProductUserId is now available via EOS_Connect_GetLoggedInUserByIndex
									// GetUniquePlayerId will query it from the Connect interface
									
									// Trigger login complete with ProductUserId
									Ctx->Identity->TriggerOnLoginCompleteDelegates(Ctx->LocalUserNum, true, *FUniqueNetIdString::Create(ProductId, FName(TEXT("EOS"))), TEXT(""));
								}
								else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
								{
									// Need to create account mapping using continuance token
									UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: InvalidUser - account mapping needed, ContinuanceToken available: %d"), Data->ContinuanceToken != nullptr);
									
									// For now, fail - full implementation would use CreateConnectedLogin
									FString ErrorMsg = FString::Printf(TEXT("Account mapping required - ContinuanceToken handling not implemented"));
									UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit: %s"), *ErrorMsg);
									Ctx->Identity->TriggerOnLoginCompleteDelegates(Ctx->LocalUserNum, false, *FUniqueNetIdString::EmptyId(), ErrorMsg);
								}
								else
								{
									FString ErrorMsg = FString::Printf(TEXT("Connect login failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
									UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit: %s"), *ErrorMsg);
									Ctx->Identity->TriggerOnLoginCompleteDelegates(Ctx->LocalUserNum, false, *FUniqueNetIdString::EmptyId(), ErrorMsg);
								}
								
								delete Ctx;
							}
						};
						
						FConnectLoginFromEpicCallback* ConnectContext = new FConnectLoginFromEpicCallback{ Context->Identity, Context->LocalUserNum, Data->LocalUserId };
						EOS_Connect_Login(Context->ConnectHandle, &ConnectLoginOptions, ConnectContext, &FConnectLoginFromEpicCallback::HandleConnectCallback);
						
						// Release auth token
						EOS_Auth_Token_Release(AuthToken);
					}
					else
					{
						FString ErrorMsg = FString::Printf(TEXT("Failed to copy auth token: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(CopyResult)));
						UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit: %s"), *ErrorMsg);
						Context->Identity->TriggerOnLoginCompleteDelegates(Context->LocalUserNum, false, *FUniqueNetIdString::EmptyId(), ErrorMsg);
					}
				}
				else
				{
					FString ErrorMsg = FString::Printf(TEXT("Epic Auth login failed: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
					UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit: %s"), *ErrorMsg);
					Context->Identity->TriggerOnLoginCompleteDelegates(Context->LocalUserNum, false, *FUniqueNetIdString::EmptyId(), ErrorMsg);
				}
				
				delete Context;
			}
		};
		
		FEpicAuthLoginCallback* AuthContext = new FEpicAuthLoginCallback{ this, LocalUserNum, AuthHandle, ConnectHandle };
		EOS_Auth_Login(AuthHandle, &AuthLoginOptions, AuthContext, &FEpicAuthLoginCallback::HandleAuthCallback);
		
		return true;
	}
	
	// Not a supported login type
	UE_LOG_ONLINE(Warning, TEXT("FOnlineIdentityEOSKit::Login: Unsupported login type: %s"), *AccountCredentials.Type);
	TriggerOnLoginCompleteDelegates(LocalUserNum, false, *FUniqueNetIdString::EmptyId(), TEXT("Unsupported login type"));
	return false;
}

void FOnlineIdentityEOSKit::CreateDeviceIdAndRetryLogin(int32 LocalUserNum, const FString& DisplayName)
{
	if (!ConnectHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit::CreateDeviceIdAndRetryLogin: Connect handle is null"));
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, *FUniqueNetIdString::EmptyId(), TEXT("Connect handle is null"));
		return;
	}
	
	UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit::CreateDeviceIdAndRetryLogin: Creating Device ID for %s"), *DisplayName);
	
	// Define callback struct first
	struct FCreateDeviceIdCallback
	{
		FOnlineIdentityEOSKit* Identity;
		int32 LocalUserNum;
		FString DisplayName;
		
		static void EOS_CALL HandleCallback(const EOS_Connect_CreateDeviceIdCallbackInfo* Data)
		{
			if (!Data || !Data->ClientData) return;
			
			FCreateDeviceIdCallback* Context = (FCreateDeviceIdCallback*)Data->ClientData;
			
			if (Data->ResultCode == EOS_EResult::EOS_Success || Data->ResultCode == EOS_EResult::EOS_DuplicateNotAllowed)
			{
				UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: Device ID created/exists (Result: %s), retrying login after short delay"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				
				// EOS needs a small delay after creating Device ID before it can be used for login
				// Use AsyncTask to delay retry on game thread
				AsyncTask(ENamedThreads::GameThread, [Context]()
				{
					// Small delay to ensure Device ID is fully registered
					FPlatformProcess::Sleep(0.1f);
					
					UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: Retrying login with Device ID"));
					
					// Retry login with Device ID
					FOnlineAccountCredentials Credentials;
					Credentials.Type = TEXT("noeas_+_DeviceID");
					Credentials.Id = Context->DisplayName;
					Credentials.Token = TEXT("");
					
					Context->Identity->Login(Context->LocalUserNum, Credentials);
					
					delete Context;
				});
				return; // Don't delete Context here, it will be deleted in AsyncTask
			}
			else
			{
				FString ErrorMsg = FString::Printf(TEXT("Failed to create Device ID: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit: %s"), *ErrorMsg);
				
				// Execute on game thread
				AsyncTask(ENamedThreads::GameThread, [Context, ErrorMsg]()
				{
					Context->Identity->TriggerOnLoginCompleteDelegates(Context->LocalUserNum, false, *FUniqueNetIdString::EmptyId(), ErrorMsg);
					delete Context;
				});
				return; // Don't delete Context here, it will be deleted in AsyncTask
			}
		}
	};
	
	// CRITICAL: Delete existing Device ID first to ensure unique ProductUserId per login
	EOS_Connect_DeleteDeviceIdOptions DeleteOptions = {};
	DeleteOptions.ApiVersion = EOS_CONNECT_DELETEDEVICEID_API_LATEST;
	
	// Create callback context for delete (with persistent DeviceModel storage)
	struct FDeleteDeviceIdContext
	{
		FOnlineIdentityEOSKit* Identity;
		int32 LocalUserNum;
		FString DisplayName;
		EOS_HConnect ConnectHandle;
		TArray<char> DeviceModelAnsi; // Persistent buffer for DeviceModel
		
		FDeleteDeviceIdContext(FOnlineIdentityEOSKit* InIdentity, int32 InLocalUserNum, const FString& InDisplayName, EOS_HConnect InConnectHandle)
			: Identity(InIdentity)
			, LocalUserNum(InLocalUserNum)
			, DisplayName(InDisplayName)
			, ConnectHandle(InConnectHandle)
		{
			// Pre-generate DeviceModel and store in persistent buffer
			FString UniqueId = FString::Printf(TEXT("%lld_%d"), FDateTime::Now().GetTicks(), FMath::Rand());
			FString DeviceModel = DisplayName.IsEmpty() 
				? FString::Printf(TEXT("PC_Player_%s"), *UniqueId)
				: FString::Printf(TEXT("PC_%s_%s"), *DisplayName, *UniqueId);
			
			// Convert to ANSI and store in persistent buffer
			const auto DeviceModelAnsiTemp = StringCast<ANSICHAR>(*DeviceModel);
			DeviceModelAnsi.SetNumUninitialized(DeviceModelAnsiTemp.Length() + 1);
			FCStringAnsi::Strcpy(DeviceModelAnsi.GetData(), DeviceModelAnsiTemp.Length() + 1, DeviceModelAnsiTemp.Get());
			
			UE_LOG_ONLINE(Warning, TEXT("FOnlineIdentityEOSKit: Prepared Device ID: %s"), *DeviceModel);
		}
	};
	
	FDeleteDeviceIdContext* DeleteContext = new FDeleteDeviceIdContext(this, LocalUserNum, DisplayName, ConnectHandle);
	
	EOS_Connect_DeleteDeviceId(ConnectHandle, &DeleteOptions, DeleteContext,
		[](const EOS_Connect_DeleteDeviceIdCallbackInfo* DeleteData)
		{
			if (!DeleteData || !DeleteData->ClientData) return;
			
			FDeleteDeviceIdContext* Ctx = static_cast<FDeleteDeviceIdContext*>(DeleteData->ClientData);
			
			// Log result
			if (DeleteData->ResultCode == EOS_EResult::EOS_Success)
			{
				UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: Deleted existing Device ID"));
			}
			else if (DeleteData->ResultCode == EOS_EResult::EOS_NotFound)
			{
				UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit: No existing Device ID to delete (this is normal for first run on new PC)"));
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("FOnlineIdentityEOSKit: DeleteDeviceId returned: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(DeleteData->ResultCode)));
			}
			
			// Now create a new Device ID using the persistent buffer
			EOS_Connect_CreateDeviceIdOptions CreateDeviceIdOptions = {};
			CreateDeviceIdOptions.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
			CreateDeviceIdOptions.DeviceModel = Ctx->DeviceModelAnsi.GetData(); // Use persistent buffer
			
			UE_LOG_ONLINE(Warning, TEXT("FOnlineIdentityEOSKit: Creating NEW Device ID: %s"), UTF8_TO_TCHAR(Ctx->DeviceModelAnsi.GetData()));
			
			// Create new callback context for CreateDeviceId
			FCreateDeviceIdCallback* CreateContext = new FCreateDeviceIdCallback{ Ctx->Identity, Ctx->LocalUserNum, Ctx->DisplayName };
			
			EOS_Connect_CreateDeviceId(Ctx->ConnectHandle, &CreateDeviceIdOptions, CreateContext, &FCreateDeviceIdCallback::HandleCallback);
			
			delete Ctx;
		});
}

bool FOnlineIdentityEOSKit::Logout(int32 LocalUserNum)
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit::Logout: LocalUserNum=%d"), LocalUserNum);
	
	// Destroy any active sessions before logging out
	if (EOSKitSubsystem)
	{
		if (IOnlineSessionPtr SessionInterface = EOSKitSubsystem->GetSessionInterface())
		{
			// Destroy common session names
			TArray<FName> CommonSessionNames = { NAME_GameSession, NAME_PartySession, FName(TEXT("Modified_EOS_Session")), FName(TEXT("Modified_EOS_Lobby")) };
			for (const FName& SessionName : CommonSessionNames)
			{
				if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName))
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineIdentityEOSKit::Logout: Destroying session '%s' before logout"), *SessionName.ToString());
					SessionInterface->DestroySession(SessionName);
				}
			}
		}
	}
	
	// Remove user account
	FScopeLock ScopeLock(&AccountLock);
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		UserAccounts.Remove(UserId.ToSharedRef());
	}
	
	// Trigger logout complete delegate
	if (OnLogoutCompleteDelegates[LocalUserNum].IsBound())
	{
		OnLogoutCompleteDelegates[LocalUserNum].Broadcast(LocalUserNum, true);
	}
	
	return true;
}

bool FOnlineIdentityEOSKit::AutoLogin(int32 LocalUserNum)
{
	// Attempt automatic login
	return Login(LocalUserNum, FOnlineAccountCredentials(TEXT("persistentauth"), TEXT(""), TEXT("")));
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityEOSKit::GetUserAccount(const FUniqueNetId& UserId) const
{
	FScopeLock ScopeLock(&AccountLock);
	const TSharedRef<FUserOnlineAccount>* Account = UserAccounts.Find(UserId.AsShared());
	if (Account)
	{
		return *Account;
	}
	return nullptr;
}

TArray<TSharedPtr<FUserOnlineAccount>> FOnlineIdentityEOSKit::GetAllUserAccounts() const
{
	FScopeLock ScopeLock(&AccountLock);
	TArray<TSharedRef<FUserOnlineAccount, ESPMode::ThreadSafe>> Accounts;
	UserAccounts.GenerateValueArray(Accounts);
	// Convert TSharedRef to TSharedPtr for return
	TArray<TSharedPtr<FUserOnlineAccount>> Result;
	Result.Reserve(Accounts.Num());
	for (const TSharedRef<FUserOnlineAccount, ESPMode::ThreadSafe>& Account : Accounts)
	{
		Result.Add(Account);
	}
	return Result;
}

FUniqueNetIdPtr FOnlineIdentityEOSKit::GetUniquePlayerId(int32 LocalUserNum) const
{
	// CRITICAL: Get ProductUserId from EOSKitSubsystem
	// This must return a FUniqueNetIdString with type "EOS" for PreLogin validation
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitSubsystem* EOSKitSubsystemPtr = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
				if (EOSKitSubsystemPtr)
				{
					EOS_ProductUserId ProductUserId = EOSKitSubsystemPtr->GetProductUserId(LocalUserNum);
					if (EOS_ProductUserId_IsValid(ProductUserId) == EOS_TRUE)
					{
						// Convert ProductUserId to string
						char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32_t ProductUserIdStrSize = sizeof(ProductUserIdStr);
						if (EOS_ProductUserId_ToString(ProductUserId, ProductUserIdStr, &ProductUserIdStrSize) == EOS_EResult::EOS_Success)
						{
							FString UserIdString = UTF8_TO_TCHAR(ProductUserIdStr);
							
							// CRITICAL: Create FUniqueNetIdString with type "EOS"
							// This ensures PreLogin validation matches the type between client and server
							FUniqueNetIdPtr UniqueNetId = const_cast<FOnlineIdentityEOSKit*>(this)->CreateUniquePlayerId(UserIdString);
							
							if (UniqueNetId.IsValid())
							{
								UE_LOG_ONLINE(Verbose, TEXT("FOnlineIdentityEOSKit::GetUniquePlayerId: ✅ Returning ProductUserId=%s (Type: %s)"), 
									*UserIdString, *UniqueNetId->GetType().ToString());
								return UniqueNetId;
							}
							else
							{
								UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit::GetUniquePlayerId: ❌ Failed to create UniqueNetId from ProductUserId: %s"), *UserIdString);
							}
						}
						else
						{
							UE_LOG_ONLINE(Error, TEXT("FOnlineIdentityEOSKit::GetUniquePlayerId: ❌ Failed to convert ProductUserId to string"));
						}
					}
					else
					{
						UE_LOG_ONLINE(Verbose, TEXT("FOnlineIdentityEOSKit::GetUniquePlayerId: ProductUserId is invalid for LocalUserNum=%d (user may not be logged in yet)"), LocalUserNum);
					}
				}
			}
		}
	}
	
	UE_LOG_ONLINE(Verbose, TEXT("FOnlineIdentityEOSKit::GetUniquePlayerId: Returning nullptr for LocalUserNum=%d"), LocalUserNum);
	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityEOSKit::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	// Create from bytes - not implemented yet
	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityEOSKit::CreateUniquePlayerId(const FString& Str)
{
	// Create from string - in UE 5.5, FUniqueNetIdString constructor is protected
	// Use FUniqueNetIdString::Create with subsystem name parameter
	// The Create function signature in UE 5.5 is: Create(const FString& InUniqueNetIdStr, const FName& InType)
	if (!Str.IsEmpty())
	{
		return FUniqueNetIdString::Create(Str, FName(TEXT("EOS")));
	}
	return nullptr;
}

ELoginStatus::Type FOnlineIdentityEOSKit::GetLoginStatus(int32 LocalUserNum) const
{
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetLoginStatus(*UserId);
	}
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityEOSKit::GetLoginStatus(const FUniqueNetId& UserId) const
{
	FScopeLock ScopeLock(&AccountLock);
	const TSharedRef<FUserOnlineAccount>* Account = UserAccounts.Find(UserId.AsShared());
	if (Account)
	{
		return ELoginStatus::LoggedIn;
	}
	return ELoginStatus::NotLoggedIn;
}

FString FOnlineIdentityEOSKit::GetPlayerNickname(int32 LocalUserNum) const
{
	FUniqueNetIdPtr UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetPlayerNickname(*UserId);
	}
	return FString();
}

FString FOnlineIdentityEOSKit::GetPlayerNickname(const FUniqueNetId& UserId) const
{
#if WITH_EOS_SDK
	// Try to get display name from EOS UserInfo interface
	if (EOSKitSubsystem && EOSKitSubsystem->UserInfoHandle)
	{
		// Convert FUniqueNetId to EOS_EpicAccountId
		FString UserIdString = UserId.ToString();
		FString EpicAccountIdStr = UserIdString;
		
		// Parse EpicAccountId from string (handle "EpicAccountId|ProductUserId" format)
		if (UserIdString.Contains(TEXT("|")))
		{
			TArray<FString> Parts;
			UserIdString.ParseIntoArray(Parts, TEXT("|"), true);
			if (Parts.Num() >= 1)
			{
				EpicAccountIdStr = Parts[0]; // Get the EpicAccountId part
			}
		}
		
		// Convert to EOS_EpicAccountId
		EOS_EpicAccountId EpicAccountId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*EpicAccountIdStr));
		if (EOS_EpicAccountId_IsValid(EpicAccountId))
		{
			// Copy UserInfo - this is synchronous if data is cached
			EOS_UserInfo_CopyUserInfoOptions CopyOptions = {};
			CopyOptions.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
			CopyOptions.LocalUserId = EpicAccountId;
			CopyOptions.TargetUserId = EpicAccountId;
			
			EOS_UserInfo* UserInfo = nullptr;
			EOS_EResult Result = EOS_UserInfo_CopyUserInfo(EOSKitSubsystem->UserInfoHandle, &CopyOptions, &UserInfo);
			
			if (Result == EOS_EResult::EOS_Success && UserInfo)
			{
				FString DisplayName = UTF8_TO_TCHAR(UserInfo->DisplayName);
				EOS_UserInfo_Release(UserInfo);
				
				if (!DisplayName.IsEmpty())
				{
					return DisplayName;
				}
			}
		}
	}
#endif
	
	// Fallback to account display name (works for Device ID logins)
	TSharedPtr<FUserOnlineAccount> Account = GetUserAccount(UserId);
	if (Account.IsValid())
	{
		FString DisplayName = Account->GetDisplayName();
		if (!DisplayName.IsEmpty() && DisplayName != UserId.ToString())
		{
			return DisplayName;
		}
	}
	
	// Final fallback: check LocalUserNumToDisplayName map
	for (const TPair<int32, FString>& Pair : LocalUserNumToDisplayName)
	{
		FUniqueNetIdPtr LocalUserId = GetUniquePlayerId(Pair.Key);
		if (LocalUserId.IsValid() && *LocalUserId == UserId)
		{
			return Pair.Value;
		}
	}
	
	// If all else fails, return part of the UserId as a readable identifier
	// For Epic Auth, the UserInfo query might not be complete yet
	FString UserIdStr = UserId.ToString();
	if (UserIdStr.Len() > 8)
	{
		return FString::Printf(TEXT("User_%s"), *UserIdStr.Right(8));
	}
	
	return FString(TEXT("Unknown"));
}

FString FOnlineIdentityEOSKit::GetAuthToken(int32 LocalUserNum) const
{
	// Get auth token - not fully implemented yet
	return FString();
}

void FOnlineIdentityEOSKit::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate, EShowPrivilegeResolveUI ShowUI)
{
	// Privilege checking - not implemented yet
	Delegate.ExecuteIfBound(UserId, Privilege, (uint32)EUserPrivileges::CanPlay);
}

FPlatformUserId FOnlineIdentityEOSKit::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	// Platform user ID mapping - not implemented yet
	return FPlatformUserId();
}

void FOnlineIdentityEOSKit::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	// Revoke auth token - not implemented yet
	// In UE 5.5, FOnRevokeAuthTokenCompleteDelegate takes (const FUniqueNetId&, const FOnlineError&)
	Delegate.ExecuteIfBound(UserId, FOnlineError(TEXT("Not implemented")));
}

#endif // WITH_EOS_SDK

