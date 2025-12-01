// Copyright (C) 2024, All Rights Reserved.

#include "EOSRejectInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSRejectInviteAsync* UEOSRejectInviteAsync::RejectInvite(FString InviteId)
{
	UEOSRejectInviteAsync* Node = NewObject<UEOSRejectInviteAsync>();
	Node->InviteId = InviteId;
	return Node;
}

void UEOSRejectInviteAsync::Activate()
{
	RejectSessionInvite();
	Super::Activate();
}

void UEOSRejectInviteAsync::RejectSessionInvite()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Rejecting Session Invite via SDK"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invite ID is empty - cannot reject invite"));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Reject Invite Options
	EOS_Sessions_RejectInviteOptions RejectOptions = {};
	RejectOptions.ApiVersion = EOS_SESSIONS_REJECTINVITE_API_LATEST;
	RejectOptions.LocalUserId = LocalUserId;
	
	FTCHARToUTF8 InviteIdConverter(*InviteId);
	RejectOptions.InviteId = InviteIdConverter.Get();
	
	// Create callback context
	struct FRejectInviteContext
	{
		UEOSRejectInviteAsync* AsyncNode;
		FString InviteId;
	};
	
	FRejectInviteContext* CallbackContext = new FRejectInviteContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->InviteId = InviteId;
	
	// Call EOS SDK to reject the invite
	EOS_Sessions_RejectInvite(SessionsHandle, &RejectOptions, CallbackContext,
		[](const EOS_Sessions_RejectInviteCallbackInfo* Data)
		{
			FRejectInviteContext* Context = static_cast<FRejectInviteContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully rejected invite!"));
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to reject invite: %s"), 
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
				
				delete Context;
			});
		});
}
