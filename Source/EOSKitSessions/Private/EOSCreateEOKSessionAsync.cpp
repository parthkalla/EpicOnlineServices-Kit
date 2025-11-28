// Copyright (C) 2024, All Rights Reserved.

#include "EOSCreateEOKSessionAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_sessions.h"
#include "Async/Async.h"

// Define a context struct to hold all data for the async operation
struct FSessionCreateContext
{
	UEOSCreateEOKSessionAsync* AsyncNode = nullptr;
	FString SessionName;
	TArray<uint8> SessionNameUTF8;
	TArray<uint8> BucketIdUTF8;
	TArray<TArray<uint8>> AttributeKeysUTF8;
	TArray<TArray<uint8>> AttributeValuesUTF8;
	EOS_HSessionModification SessionModHandle = nullptr;
};

void UEOSCreateEOKSessionAsync::Activate()
{
	CreateSession();
	Super::Activate();
}

void UEOSCreateEOKSessionAsync::CreateSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Creating EOS Session via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Public Connections: %d"), NumberOfPublicConnections);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: BucketId: %s"), *ExtraSettings.BucketId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bIsLanMatch: %s (must be false for EOS)"), ExtraSettings.bIsLanMatch ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bUsePresence: %s (required for presence-based sessions)"), ExtraSettings.bUsePresence ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bShouldAdvertise: %s"), ExtraSettings.bShouldAdvertise ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bAllowJoinViaPresence: %s"), ExtraSettings.bAllowJoinViaPresence ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bAllowJoinInProgress: %s"), ExtraSettings.bAllowJoinInProgress ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));

	// Validate and auto-correct critical settings per EOS requirements
	if (ExtraSettings.bIsLanMatch)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING - bIsLanMatch is TRUE but must be FALSE for EOS. Overriding to false."));
		ExtraSettings.bIsLanMatch = false;
	}

	if (NumberOfPublicConnections <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING - NumberOfPublicConnections is %d but must be > 0. Setting to 4."), NumberOfPublicConnections);
		NumberOfPublicConnections = 4;
	}

	if (!ExtraSettings.bShouldAdvertise)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING - bShouldAdvertise is FALSE. Session will not be searchable."));
	}
	
	if (!CachedWorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - WorldContextObject is null"));
		OnFail.Broadcast(TEXT("WorldContextObject is null"));
		SetReadyToDestroy();
		return;
	}
	
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(CachedWorldContextObject);
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - Cannot get Game Instance"));
		OnFail.Broadcast(TEXT("Cannot get Game Instance"));
		SetReadyToDestroy();
		return;
	}
	
	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - EOSKit Subsystem or Platform Handle is null"));
		OnFail.Broadcast(TEXT("EOSKit Subsystem or Platform Handle is null"));
		SetReadyToDestroy();
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HSessions SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformHandle);
	
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - Failed to get Sessions Handle"));
		OnFail.Broadcast(TEXT("Failed to get Sessions Handle"));
		SetReadyToDestroy();
		return;
	}
	
	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED - Product User ID is null. User must be logged in."));
		OnFail.Broadcast(TEXT("Product User ID is null - user must be logged in"));
		SetReadyToDestroy();
		return;
	}

	// Log user ID for debugging
	char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
	EOS_ProductUserId_ToString(LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession - LocalUserId: %s"), UTF8_TO_TCHAR(ProductUserIdStr));
	
	// Create and populate the context that will live until the callback
	FSessionCreateContext* CallbackContext = new FSessionCreateContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	
	// Convert strings to UTF8 and store them safely in the context
	FTCHARToUTF8 SessionNameConverter(*CallbackContext->SessionName);
	CallbackContext->SessionNameUTF8.Append((uint8*)SessionNameConverter.Get(), SessionNameConverter.Length() + 1);
	
	// CRITICAL: Ensure BucketId has a default value - EOS sometimes fails without it
	FString BucketIdStr = ExtraSettings.BucketId.IsEmpty() ? TEXT("MyGameBucket") : ExtraSettings.BucketId;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession - Using BucketId: %s"), *BucketIdStr);
	FTCHARToUTF8 BucketIdConverter(*BucketIdStr);
	CallbackContext->BucketIdUTF8.Append((uint8*)BucketIdConverter.Get(), BucketIdConverter.Length() + 1);
	
	// Step 1: Create Session Modification Handle
	EOS_Sessions_CreateSessionModificationOptions CreateModOptions = {};
	CreateModOptions.ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
	CreateModOptions.SessionName = (const char*)CallbackContext->SessionNameUTF8.GetData();
	CreateModOptions.BucketId = (const char*)CallbackContext->BucketIdUTF8.GetData();
	CreateModOptions.MaxPlayers = NumberOfPublicConnections;
	CreateModOptions.LocalUserId = LocalUserId;
	CreateModOptions.bPresenceEnabled = ExtraSettings.bUsePresence ? EOS_TRUE : EOS_FALSE;
	CreateModOptions.bSanctionsEnabled = ExtraSettings.bEnforceSanctions ? EOS_TRUE : EOS_FALSE;

	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession - Creating modification handle..."));
	UE_LOG(LogTemp, Log, TEXT("  SessionName: %s"), UTF8_TO_TCHAR(CreateModOptions.SessionName));
	UE_LOG(LogTemp, Log, TEXT("  BucketId: %s"), UTF8_TO_TCHAR(CreateModOptions.BucketId));
	UE_LOG(LogTemp, Log, TEXT("  MaxPlayers: %d"), CreateModOptions.MaxPlayers);
	UE_LOG(LogTemp, Log, TEXT("  bPresenceEnabled: %s"), CreateModOptions.bPresenceEnabled ? TEXT("true") : TEXT("false"));
	
	EOS_HSessionModification SessionModHandle = nullptr;
	EOS_EResult CreateModResult = EOS_Sessions_CreateSessionModification(SessionsHandle, &CreateModOptions, &SessionModHandle);
	
	if (CreateModResult != EOS_EResult::EOS_Success)
	{
		const char* ErrorStr = EOS_EResult_ToString(CreateModResult);
		UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED at CreateSessionModification!"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Error Code: %s (%d)"), UTF8_TO_TCHAR(ErrorStr), static_cast<int32>(CreateModResult));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
		delete CallbackContext;
		OnFail.Broadcast(FString::Printf(TEXT("CreateSessionModification failed: %s"), UTF8_TO_TCHAR(ErrorStr)));
		SetReadyToDestroy();
		return;
	}
	
	CallbackContext->SessionModHandle = SessionModHandle;
	
	// Step 2: Set Session Properties
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession - Setting session properties..."));

	// Set permission level based on bShouldAdvertise
	EOS_SessionModification_SetPermissionLevelOptions PermOptions = {};
	PermOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
	PermOptions.PermissionLevel = ExtraSettings.bShouldAdvertise ? 
		EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised : 
		EOS_EOnlineSessionPermissionLevel::EOS_OSPF_InviteOnly;
	EOS_EResult PermResult = EOS_SessionModification_SetPermissionLevel(SessionModHandle, &PermOptions);
	UE_LOG(LogTemp, Log, TEXT("  SetPermissionLevel: %s (result: %s)"), 
		ExtraSettings.bShouldAdvertise ? TEXT("PublicAdvertised") : TEXT("InviteOnly"),
		UTF8_TO_TCHAR(EOS_EResult_ToString(PermResult)));
	
	// Set join in progress allowed
	EOS_SessionModification_SetJoinInProgressAllowedOptions JIPOptions = {};
	JIPOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
	JIPOptions.bAllowJoinInProgress = ExtraSettings.bAllowJoinInProgress ? EOS_TRUE : EOS_FALSE;
	EOS_EResult JIPResult = EOS_SessionModification_SetJoinInProgressAllowed(SessionModHandle, &JIPOptions);
	UE_LOG(LogTemp, Log, TEXT("  SetJoinInProgressAllowed: %s (result: %s)"), 
		ExtraSettings.bAllowJoinInProgress ? TEXT("true") : TEXT("false"),
		UTF8_TO_TCHAR(EOS_EResult_ToString(JIPResult)));
	
	// Add custom attributes, ensuring strings are kept alive in the context
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession - Adding %d custom attributes..."), SessionSettings.Num());
	for (const auto& Setting : SessionSettings)
	{
		if (Setting.Key.IsEmpty()) continue;
		
		// Store key string in context
		FTCHARToUTF8 KeyConverter(*Setting.Key);
		TArray<uint8>& KeyUTF8 = CallbackContext->AttributeKeysUTF8.Emplace_GetRef();
		KeyUTF8.Append((uint8*)KeyConverter.Get(), KeyConverter.Length() + 1);

		EOS_Sessions_AttributeData AttrData = {};
		AttrData.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		AttrData.Key = (const char*)KeyUTF8.GetData();
		
		if (!Setting.Value.StringValue.IsEmpty())
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
			FTCHARToUTF8 ValueConverter(*Setting.Value.StringValue);
			TArray<uint8>& ValueUTF8 = CallbackContext->AttributeValuesUTF8.Emplace_GetRef();
			ValueUTF8.Append((uint8*)ValueConverter.Get(), ValueConverter.Length() + 1);
			AttrData.Value.AsUtf8 = (const char*)ValueUTF8.GetData();
			UE_LOG(LogTemp, Log, TEXT("  Attribute '%s' = '%s' (String)"), *Setting.Key, *Setting.Value.StringValue);
		}
		else if (Setting.Value.IntValue != 0)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
			AttrData.Value.AsInt64 = Setting.Value.IntValue;
			UE_LOG(LogTemp, Log, TEXT("  Attribute '%s' = %d (Int64)"), *Setting.Key, Setting.Value.IntValue);
		}
		else
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
			AttrData.Value.AsBool = Setting.Value.BoolValue ? EOS_TRUE : EOS_FALSE;
			UE_LOG(LogTemp, Log, TEXT("  Attribute '%s' = %s (Bool)"), *Setting.Key, Setting.Value.BoolValue ? TEXT("true") : TEXT("false"));
		}
		
		EOS_SessionModification_AddAttributeOptions AttrOptions = {};
		AttrOptions.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
		AttrOptions.SessionAttribute = &AttrData;
		AttrOptions.AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise;
		
		EOS_EResult AttrResult = EOS_SessionModification_AddAttribute(SessionModHandle, &AttrOptions);
		if (AttrResult != EOS_EResult::EOS_Success)
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: Failed to add attribute '%s': %s"), *Setting.Key, UTF8_TO_TCHAR(EOS_EResult_ToString(AttrResult)));
		}
	}
	
	// Step 3: Update Session (Create it on EOS backend)
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession - Calling EOS_Sessions_UpdateSession..."));
	EOS_Sessions_UpdateSessionOptions UpdateOptions = {};
	UpdateOptions.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
	UpdateOptions.SessionModificationHandle = SessionModHandle;
	
	EOS_Sessions_UpdateSession(SessionsHandle, &UpdateOptions, CallbackContext, 
		[](const EOS_Sessions_UpdateSessionCallbackInfo* Data)
		{
			FSessionCreateContext* Context = static_cast<FSessionCreateContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession callback - Invalid context"));
				if (Context)
				{
					if (Context->SessionModHandle)
					{
						EOS_SessionModification_Release(Context->SessionModHandle);
					}
					delete Context;
				}
				return;
			}

			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Log, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession SUCCESS!"));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Session ID: %s"), UTF8_TO_TCHAR(Data->SessionId));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: ========================================"));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast(UTF8_TO_TCHAR(Data->SessionId));
					}
				}
				else
				{
					const char* ErrorStr = EOS_EResult_ToString(Data->ResultCode);
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateSession FAILED!"));
					UE_LOG(LogTemp, Error, TEXT("EOSKit: Error Code: %s (%d)"), UTF8_TO_TCHAR(ErrorStr), static_cast<int32>(Data->ResultCode));
					UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast(FString::Printf(TEXT("UpdateSession failed: %s"), UTF8_TO_TCHAR(ErrorStr)));
					}
				}

				if (Context->AsyncNode)
				{
					Context->AsyncNode->SetReadyToDestroy();
				}
				
				if (Context->SessionModHandle)
				{
					EOS_SessionModification_Release(Context->SessionModHandle);
				}
				
				delete Context;
			});
		});
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateSession - Waiting for callback..."));
}

void UEOSCreateEOKSessionAsync::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	// Not used - we use the EOS SDK callback directly
}

UEOSCreateEOKSessionAsync* UEOSCreateEOKSessionAsync::CreateEOKSession(
	UObject* WorldContextObject,
	TMap<FString, FEOSKitAttribute> SessionSettings,
	FName SessionName,
	int32 NumberOfPublicConnections,
	FEOSKitDedicatedServerSettings DedicatedServerSettings,
	FEOSKitCreateSessionSettings ExtraSettings)
{
	UEOSCreateEOKSessionAsync* Ueik_CreateSessionObject = NewObject<UEOSCreateEOKSessionAsync>();
	Ueik_CreateSessionObject->CachedWorldContextObject = WorldContextObject;
	Ueik_CreateSessionObject->SessionSettings = SessionSettings;
	Ueik_CreateSessionObject->NumberOfPublicConnections = NumberOfPublicConnections;
	Ueik_CreateSessionObject->DedicatedServerSettings = DedicatedServerSettings;
	Ueik_CreateSessionObject->VSessionName = SessionName;
	Ueik_CreateSessionObject->ExtraSettings = ExtraSettings;
	return Ueik_CreateSessionObject;
}
