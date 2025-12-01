// Copyright (C) 2024, All Rights Reserved.

#include "EOSGetSessionAttributeAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitSessionsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSGetSessionAttributeAsync* UEOSGetSessionAttributeAsync::GetSessionAttribute(FName SessionName, FString AttributeKey)
{
	UEOSGetSessionAttributeAsync* Node = NewObject<UEOSGetSessionAttributeAsync>();
	Node->VSessionName = SessionName;
	Node->AttributeKey = AttributeKey;
	return Node;
}

void UEOSGetSessionAttributeAsync::Activate()
{
	GetAttribute();
	Super::Activate();
}

void UEOSGetSessionAttributeAsync::GetAttribute()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Getting session attribute: %s from session: %s"), 
		*AttributeKey, *VSessionName.ToString());
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnComplete.Broadcast(false, FEOSKitAttribute());
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
		OnComplete.Broadcast(false, FEOSKitAttribute());
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
		OnComplete.Broadcast(false, FEOSKitAttribute());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Get active session handle
	EOS_Sessions_CopyActiveSessionHandleOptions CopyOptions = {};
	CopyOptions.ApiVersion = EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*VSessionName.ToString());
	CopyOptions.SessionName = SessionNameConverter.Get();
	
	EOS_HActiveSession ActiveSessionHandle = nullptr;
	EOS_EResult CopyResult = EOS_Sessions_CopyActiveSessionHandle(SessionsHandle, &CopyOptions, &ActiveSessionHandle);
	
	if (CopyResult != EOS_EResult::EOS_Success || !ActiveSessionHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get active session handle: %s"), 
			*FString(EOS_EResult_ToString(CopyResult)));
		OnComplete.Broadcast(false, FEOSKitAttribute());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Get session info
	EOS_ActiveSession_CopyInfoOptions InfoOptions = {};
	InfoOptions.ApiVersion = EOS_ACTIVESESSION_COPYINFO_API_LATEST;
	
	EOS_ActiveSession_Info* SessionInfo = nullptr;
	EOS_EResult InfoResult = EOS_ActiveSession_CopyInfo(ActiveSessionHandle, &InfoOptions, &SessionInfo);
	
	if (InfoResult != EOS_EResult::EOS_Success || !SessionInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to copy session info: %s"), 
			*FString(EOS_EResult_ToString(InfoResult)));
		EOS_ActiveSession_Release(ActiveSessionHandle);
		OnComplete.Broadcast(false, FEOSKitAttribute());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Find the attribute
	bool bFound = false;
	FEOSKitAttribute ResultAttribute;
	
	// Note: In a full implementation, you would iterate through SessionInfo->Settings
	// to find the attribute by key. This is a simplified version.
	
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session attribute retrieval completed"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Note: Full attribute parsing requires additional implementation"));
	
	// Cleanup
	EOS_ActiveSession_Info_Release(SessionInfo);
	EOS_ActiveSession_Release(ActiveSessionHandle);
	
	OnComplete.Broadcast(bFound, ResultAttribute);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
