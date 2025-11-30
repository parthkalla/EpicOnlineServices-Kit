// Copyright (C) 2024, All Rights Reserved.

#include "EOSAcceptInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitSessionsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_sessions.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSAcceptInviteAsync* UEOSAcceptInviteAsync::AcceptInvite(FString InviteId)
{
	UEOSAcceptInviteAsync* Node = NewObject<UEOSAcceptInviteAsync>();
	Node->InviteId = InviteId;
	return Node;
}

void UEOSAcceptInviteAsync::Activate()
{
	AcceptSessionInvite();
	Super::Activate();
}

void UEOSAcceptInviteAsync::AcceptSessionInvite()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Accepting Session Invite via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Invite ID: %s"), *InviteId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast();
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
		OnFail.Broadcast();
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
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Get Product User ID
	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!LocalUserId)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Product User ID is null - user must be logged in"));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate Invite ID
	if (InviteId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invite ID is empty - cannot accept invite"));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Step 1: Get session handle from invite ID
	EOS_Sessions_CopySessionHandleByInviteIdOptions CopyOptions = {};
	CopyOptions.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST;
	
	FTCHARToUTF8 InviteIdConverter(*InviteId);
	CopyOptions.InviteId = InviteIdConverter.Get();
	
	EOS_HSessionDetails SessionHandle = nullptr;
	EOS_EResult CopyResult = EOS_Sessions_CopySessionHandleByInviteId(SessionsHandle, &CopyOptions, &SessionHandle);
	
	if (CopyResult != EOS_EResult::EOS_Success || !SessionHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get session handle from invite: %s"), 
			*FString(EOS_EResult_ToString(CopyResult)));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Step 2: Join the session using the handle
	EOS_Sessions_JoinSessionOptions JoinOptions = {};
	JoinOptions.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
	
	FString TempSessionName = FString::Printf(TEXT("InviteSession_%lld"), FDateTime::Now().GetTicks());
	FTCHARToUTF8 SessionNameConverter(*TempSessionName);
	JoinOptions.SessionName = SessionNameConverter.Get();
	JoinOptions.SessionHandle = SessionHandle;
	JoinOptions.LocalUserId = LocalUserId;
	JoinOptions.bPresenceEnabled = EOS_FALSE;
	
	// Create callback context
	struct FAcceptInviteContext
	{
		UEOSAcceptInviteAsync* AsyncNode;
		FString InviteId;
		EOS_HSessionDetails SessionHandle;
	};
	
	FAcceptInviteContext* CallbackContext = new FAcceptInviteContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->InviteId = InviteId;
	CallbackContext->SessionHandle = SessionHandle;
	
	// Call EOS SDK to join the session
	EOS_Sessions_JoinSession(SessionsHandle, &JoinOptions, CallbackContext,
		[](const EOS_Sessions_JoinSessionCallbackInfo* Data)
		{
			FAcceptInviteContext* Context = static_cast<FAcceptInviteContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				if (Context && Context->SessionHandle)
				{
					EOS_SessionDetails_Release(Context->SessionHandle);
				}
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully accepted invite and joined session!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Invite ID: %s"), *Context->InviteId);
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast();
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to accept invite: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast();
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				
				// Cleanup
				if (Context->SessionHandle)
				{
					EOS_SessionDetails_Release(Context->SessionHandle);
				}
				delete Context;
			});
		});
}
