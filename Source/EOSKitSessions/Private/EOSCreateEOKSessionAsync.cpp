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
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	if (!CachedWorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: WorldContextObject is null"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
		return;
	}
	
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(CachedWorldContextObject);
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
		return;
	}
	
	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: EOSKit Subsystem or Platform Handle is null"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HSessions SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformHandle);
	
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Sessions Handle"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
		return;
	}
	
	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!LocalUserId)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Product User ID is null - user must be logged in"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
		return;
	}
	
	// Create and populate the context that will live until the callback
	FSessionCreateContext* CallbackContext = new FSessionCreateContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	
	// Convert strings to UTF8 and store them safely in the context
	FTCHARToUTF8 SessionNameConverter(*CallbackContext->SessionName);
	CallbackContext->SessionNameUTF8.Append((uint8*)SessionNameConverter.Get(), SessionNameConverter.Length() + 1);
	
	FString BucketIdStr = ExtraSettings.BucketId.IsEmpty() ? TEXT("DefaultBucket") : ExtraSettings.BucketId;
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
	
	EOS_HSessionModification SessionModHandle = nullptr;
	EOS_EResult CreateModResult = EOS_Sessions_CreateSessionModification(SessionsHandle, &CreateModOptions, &SessionModHandle);
	
	if (CreateModResult != EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create session modification: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(CreateModResult)));
		delete CallbackContext;
		OnFail.Broadcast("");
		SetReadyToDestroy();
		return;
	}
	
	CallbackContext->SessionModHandle = SessionModHandle;
	
	// Step 2: Set Session Properties
	EOS_SessionModification_SetPermissionLevelOptions PermOptions = {};
	PermOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
	PermOptions.PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised;
	EOS_SessionModification_SetPermissionLevel(SessionModHandle, &PermOptions);
	
	EOS_SessionModification_SetJoinInProgressAllowedOptions JIPOptions = {};
	JIPOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
	JIPOptions.bAllowJoinInProgress = ExtraSettings.bAllowJoinInProgress ? EOS_TRUE : EOS_FALSE;
	EOS_SessionModification_SetJoinInProgressAllowed(SessionModHandle, &JIPOptions);
	
	// Add custom attributes, ensuring strings are kept alive in the context
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
		}
		else if (Setting.Value.IntValue != 0)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
			AttrData.Value.AsInt64 = Setting.Value.IntValue;
		}
		else
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
			AttrData.Value.AsBool = Setting.Value.BoolValue ? EOS_TRUE : EOS_FALSE;
		}
		
		EOS_SessionModification_AddAttributeOptions AttrOptions = {};
		AttrOptions.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
		AttrOptions.SessionAttribute = &AttrData;
		AttrOptions.AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise;
		
		EOS_SessionModification_AddAttribute(SessionModHandle, &AttrOptions);
	}
	
	// Step 3: Update Session (Create it on EOS backend)
	EOS_Sessions_UpdateSessionOptions UpdateOptions = {};
	UpdateOptions.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
	UpdateOptions.SessionModificationHandle = SessionModHandle;
	
	EOS_Sessions_UpdateSession(SessionsHandle, &UpdateOptions, CallbackContext, 
		[](const EOS_Sessions_UpdateSessionCallbackInfo* Data)
		{
			FSessionCreateContext* Context = static_cast<FSessionCreateContext*>(Data->ClientData);
			
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Session created successfully on EOS backend!"));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Session ID: %s"), UTF8_TO_TCHAR(Data->SessionId));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast(UTF8_TO_TCHAR(Data->SessionId));
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create session: %s"), UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast("");
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
