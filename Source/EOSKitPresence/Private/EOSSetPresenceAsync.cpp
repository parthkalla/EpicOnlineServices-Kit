// Copyright (C) 2024, All Rights Reserved.

#include "EOSSetPresenceAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_presence.h"
#include "eos_presence_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSSetPresenceAsync* UEOSSetPresenceAsync::SetPresence(UObject* WorldContextObject, 
	const FString& RichPresence, 
	EEOSKitPresenceStatus PresenceStatus)
{
	UEOSSetPresenceAsync* Node = NewObject<UEOSSetPresenceAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->RichPresenceString = RichPresence;
	Node->PresenceStatusEnum = PresenceStatus;
	return Node;
}

void UEOSSetPresenceAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	// Get Epic Account ID from subsystem
	FString EpicAccountIdString = EOSSubsystem->GetEpicAccountIdString();
	if (EpicAccountIdString.IsEmpty())
	{
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	EOS_HPresence PresenceHandle = EOS_Platform_GetPresenceInterface(EOSSubsystem->GetPlatformHandle());
	if (!PresenceHandle)
	{
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	// Convert Epic Account ID
	EOS_EpicAccountId LocalUserId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*EpicAccountIdString));
	if (!EOS_EpicAccountId_IsValid(LocalUserId))
	{
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	// Create presence modification handle
	EOS_HPresenceModification PresenceModificationHandle = nullptr;
	EOS_Presence_CreatePresenceModificationOptions CreateOptions = {};
	CreateOptions.ApiVersion = EOS_PRESENCE_CREATEPRESENCEMODIFICATION_API_LATEST;
	CreateOptions.LocalUserId = LocalUserId;

	EOS_EResult CreateResult = EOS_Presence_CreatePresenceModification(PresenceHandle, &CreateOptions, &PresenceModificationHandle);
	if (CreateResult != EOS_EResult::EOS_Success || !PresenceModificationHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPresence: Failed to create presence modification handle"));
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	// Convert presence status enum to EOS status
	EOS_Presence_EStatus EOSStatus = EOS_PS_Offline;
	switch (PresenceStatusEnum)
	{
	case EEOSKitPresenceStatus::Online:
		EOSStatus = EOS_PS_Online;
		break;
	case EEOSKitPresenceStatus::Away:
		EOSStatus = EOS_PS_Away;
		break;
	case EEOSKitPresenceStatus::ExtendedAway:
		EOSStatus = EOS_PS_ExtendedAway;
		break;
	case EEOSKitPresenceStatus::Offline:
		EOSStatus = EOS_PS_Offline;
		break;
	case EEOSKitPresenceStatus::DoNotDisturb:
		EOSStatus = EOS_PS_DoNotDisturb;
		break;
	}

	// Set status
	EOS_PresenceModification_SetStatusOptions StatusOptions = {};
	StatusOptions.ApiVersion = EOS_PRESENCE_SETSTATUS_API_LATEST;
	StatusOptions.Status = EOSStatus;
	EOS_EResult SetStatusResult = EOS_PresenceModification_SetStatus(PresenceModificationHandle, &StatusOptions);
	if (SetStatusResult != EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKitPresence: Failed to set presence status"));
		EOS_PresenceModification_Release(PresenceModificationHandle);
		OnFailure.Broadcast(TEXT(""), EEOSKitPresenceStatus::Offline);
		SetReadyToDestroy();
		return;
	}

	// Set rich text
	if (!RichPresenceString.IsEmpty())
	{
		EOS_PresenceModification_SetRawRichTextOptions RichTextOptions = {};
		RichTextOptions.ApiVersion = EOS_PRESENCE_SETRAWRICHTEXT_API_LATEST;
		FTCHARToUTF8 RichTextConverter(*RichPresenceString);
		RichTextOptions.RichText = RichTextConverter.Get();
		
		EOS_EResult SetRichTextResult = EOS_PresenceModification_SetRawRichText(PresenceModificationHandle, &RichTextOptions);
		if (SetRichTextResult != EOS_EResult::EOS_Success)
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKitPresence: Failed to set rich text, continuing anyway"));
		}
	}

	// Set presence
	EOS_Presence_SetPresenceOptions SetOptions = {};
	SetOptions.ApiVersion = EOS_PRESENCE_SETPRESENCE_API_LATEST;
	SetOptions.LocalUserId = LocalUserId;
	SetOptions.PresenceModificationHandle = PresenceModificationHandle;

	EOS_Presence_SetPresence(PresenceHandle, &SetOptions, this, &UEOSSetPresenceAsync::OnSetPresenceComplete);
}

void EOS_CALL UEOSSetPresenceAsync::OnSetPresenceComplete(const EOS_Presence_SetPresenceCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSSetPresenceAsync* Self = static_cast<UEOSSetPresenceAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		Self->OnSuccess.Broadcast(Self->RichPresenceString, Self->PresenceStatusEnum);
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Set Presence Failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFailure.Broadcast(ErrorMessage, EEOSKitPresenceStatus::Offline);
	}

	Self->SetReadyToDestroy();
}

