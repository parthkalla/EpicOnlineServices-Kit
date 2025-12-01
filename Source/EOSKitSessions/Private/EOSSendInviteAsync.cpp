// Copyright (C) 2024, All Rights Reserved.

#include "EOSSendInviteAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSSendInviteAsync* UEOSSendInviteAsync::SendInvite(
	FName SessionName,
	FString TargetUserId)
{
	UEOSSendInviteAsync* Node = NewObject<UEOSSendInviteAsync>();
	Node->VSessionName = SessionName;
	Node->TargetProductUserId = TargetUserId;
	return Node;
}

void UEOSSendInviteAsync::Activate()
{
	SendSessionInvite();
	Super::Activate();
}

void UEOSSendInviteAsync::SendSessionInvite()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Sending Session Invite via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Target User ID: %s"), *TargetProductUserId);
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
	
	// Get Product User ID (local user sending the invite)
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
	
	// Validate Target User ID
	if (TargetProductUserId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Target User ID is empty - cannot send invite"));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Convert string Target User ID to EOS_ProductUserId
	// NOTE: This is a simplified implementation. In a real system, you'd need to:
	// 1. Maintain a mapping of string IDs to EOS_ProductUserId handles
	// 2. Or use EOS_ProductUserId_FromString if the string is properly formatted
	
	// For now, we'll document the limitation
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: SEND INVITE LIMITATION"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Converting string to EOS_ProductUserId requires proper handle management"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Current implementation is simplified - use Friends List integration for production"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Try to convert the string to EOS_ProductUserId
	EOS_ProductUserId TargetUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*TargetProductUserId));
	
	if (!TargetUserId || !EOS_ProductUserId_IsValid(TargetUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to convert Target User ID string to EOS_ProductUserId"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Make sure the Target User ID is in the correct format"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Example format: '00021a5c8e4e4d5ab2c44f32eccb7c5a'"));
		OnFail.Broadcast();
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Send Invite Options
	EOS_Sessions_SendInviteOptions SendOptions = {};
	SendOptions.ApiVersion = EOS_SESSIONS_SENDINVITE_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*VSessionName.ToString());
	SendOptions.SessionName = SessionNameConverter.Get();
	SendOptions.LocalUserId = LocalUserId;
	SendOptions.TargetUserId = TargetUserId;
	
	// Create callback context
	struct FSendInviteContext
	{
		UEOSSendInviteAsync* AsyncNode;
		FString SessionName;
		FString TargetUserId;
	};
	
	FSendInviteContext* CallbackContext = new FSendInviteContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	CallbackContext->TargetUserId = TargetProductUserId;
	
	// Call EOS SDK to send the invite
	EOS_Sessions_SendInvite(SessionsHandle, &SendOptions, CallbackContext,
		[](const EOS_Sessions_SendInviteCallbackInfo* Data)
		{
			FSendInviteContext* Context = static_cast<FSendInviteContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully sent invite!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Invited User ID: %s"), *Context->TargetUserId);
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to send invite: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error codes:
					if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Target user ID is invalid or user not found"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Sessions_NotAllowed)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: You are not allowed to send invites for this session"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Make sure you created the session or are registered as a player"));
					}
					
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
