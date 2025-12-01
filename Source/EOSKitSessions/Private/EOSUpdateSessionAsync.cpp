// Copyright (C) 2024, All Rights Reserved.

#include "EOSUpdateSessionAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSUpdateSessionAsync* UEOSUpdateSessionAsync::UpdateSession(
	FName SessionName,
	const TMap<FString, FEOSKitAttribute>& NewSessionSettings,
	int32 NewMaxPlayers,
	bool bNewAllowJoinInProgress)
{
	UEOSUpdateSessionAsync* Node = NewObject<UEOSUpdateSessionAsync>();
	Node->VSessionName = SessionName;
	Node->SessionSettings = NewSessionSettings;
	Node->MaxPlayers = NewMaxPlayers;
	Node->bAllowJoinInProgress = bNewAllowJoinInProgress;
	return Node;
}

void UEOSUpdateSessionAsync::Activate()
{
	UpdateGameSession();
	Super::Activate();
}

void UEOSUpdateSessionAsync::UpdateGameSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Updating Session via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: New Settings Count: %d"), SessionSettings.Num());
	if (MaxPlayers > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: New Max Players: %d"), MaxPlayers);
	}
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: EOSKit Subsystem or Platform Handle is null"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HSessions SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformHandle);
	
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Sessions Handle"));
		OnFail.Broadcast("");
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Step 1: Create Update Session Modification Handle
	EOS_Sessions_UpdateSessionModificationOptions UpdateModOptions = {};
	UpdateModOptions.ApiVersion = EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*VSessionName.ToString());
	UpdateModOptions.SessionName = SessionNameConverter.Get();
	
	EOS_HSessionModification SessionModHandle = nullptr;
	EOS_EResult CreateModResult = EOS_Sessions_UpdateSessionModification(SessionsHandle, &UpdateModOptions, &SessionModHandle);
	
	if (CreateModResult != EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create update session modification: %s"), 
			*FString(EOS_EResult_ToString(CreateModResult)));
		OnFail.Broadcast("");
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Step 2: Set new session properties
	
	// Update max players if specified
	if (MaxPlayers > 0)
	{
		EOS_SessionModification_SetMaxPlayersOptions MaxPlayersOptions = {};
		MaxPlayersOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST;
		MaxPlayersOptions.MaxPlayers = MaxPlayers;
		EOS_SessionModification_SetMaxPlayers(SessionModHandle, &MaxPlayersOptions);
	}
	
	// Update join in progress setting
	EOS_SessionModification_SetJoinInProgressAllowedOptions JIPOptions = {};
	JIPOptions.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
	JIPOptions.bAllowJoinInProgress = bAllowJoinInProgress ? EOS_TRUE : EOS_FALSE;
	EOS_SessionModification_SetJoinInProgressAllowed(SessionModHandle, &JIPOptions);
	
	// Update custom attributes
	for (const auto& Setting : SessionSettings)
	{
		if (Setting.Key.IsEmpty())
		{
			continue;
		}
		
		EOS_Sessions_AttributeData AttrData = {};
		AttrData.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		
		FTCHARToUTF8 KeyConverter(*Setting.Key);
		AttrData.Key = KeyConverter.Get();
		AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
		
		FString ValueStr;
		if (!Setting.Value.StringValue.IsEmpty())
		{
			ValueStr = Setting.Value.StringValue;
		}
		else if (Setting.Value.IntValue != 0)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
			AttrData.Value.AsInt64 = Setting.Value.IntValue;
		}
		else if (Setting.Value.BoolValue)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
			AttrData.Value.AsBool = EOS_TRUE;
		}
		else if (Setting.Value.FloatValue != 0.0f)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Double;
			AttrData.Value.AsDouble = Setting.Value.FloatValue;
		}
		
		if (AttrData.ValueType == EOS_ESessionAttributeType::EOS_SAT_String)
		{
			FTCHARToUTF8 ValueConverter(*ValueStr);
			AttrData.Value.AsUtf8 = ValueConverter.Get();
		}
		
		EOS_SessionModification_AddAttributeOptions AttrOptions = {};
		AttrOptions.ApiVersion = EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST;
		AttrOptions.SessionAttribute = &AttrData;
		AttrOptions.AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise;
		
		EOS_SessionModification_AddAttribute(SessionModHandle, &AttrOptions);
	}
	
	// Step 3: Apply the update to the session
	EOS_Sessions_UpdateSessionOptions UpdateOptions = {};
	UpdateOptions.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
	UpdateOptions.SessionModificationHandle = SessionModHandle;
	
	// Create callback context
	struct FSessionUpdateContext
	{
		UEOSUpdateSessionAsync* AsyncNode;
		FString SessionName;
		EOS_HSessionModification ModHandle;
	};
	
	FSessionUpdateContext* CallbackContext = new FSessionUpdateContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	CallbackContext->ModHandle = SessionModHandle; // Store for cleanup
	
	// Call EOS SDK to update the session
	EOS_Sessions_UpdateSession(SessionsHandle, &UpdateOptions, CallbackContext,
		[](const EOS_Sessions_UpdateSessionCallbackInfo* Data)
		{
			FSessionUpdateContext* Context = static_cast<FSessionUpdateContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				if (Context && Context->ModHandle)
				{
					EOS_SessionModification_Release(Context->ModHandle);
				}
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Session updated successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session ID: %s"), UTF8_TO_TCHAR(Data->SessionId));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast(UTF8_TO_TCHAR(Data->SessionId));
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to update session: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_Sessions_OutOfSync)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Session is out of sync - changes may have been made elsewhere"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Session not found - it may have been destroyed"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Session is in invalid state for updating"));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast("");
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				
				// Cleanup
				if (Context->ModHandle)
				{
					EOS_SessionModification_Release(Context->ModHandle);
				}
				delete Context;
			});
		});
}
