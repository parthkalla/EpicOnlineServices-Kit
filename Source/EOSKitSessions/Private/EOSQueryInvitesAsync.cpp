// Copyright (C) 2024, All Rights Reserved.

#include "EOSQueryInvitesAsync.h"
#include "EOSKitSubsystem.h"
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

UEOSQueryInvitesAsync* UEOSQueryInvitesAsync::QueryInvites()
{
	UEOSQueryInvitesAsync* Node = NewObject<UEOSQueryInvitesAsync>();
	return Node;
}

void UEOSQueryInvitesAsync::Activate()
{
	QuerySessionInvites();
	Super::Activate();
}

void UEOSQueryInvitesAsync::QuerySessionInvites()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Querying Session Invites via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast(0);
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
		OnFail.Broadcast(0);
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
		OnFail.Broadcast(0);
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
		OnFail.Broadcast(0);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Query Invites Options
	EOS_Sessions_QueryInvitesOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_SESSIONS_QUERYINVITES_API_LATEST;
	QueryOptions.LocalUserId = LocalUserId;
	
	// Create callback context
	struct FQueryInvitesContext
	{
		UEOSQueryInvitesAsync* AsyncNode;
		EOS_HSessions SessionsHandle;
		EOS_ProductUserId LocalUserId;
	};
	
	FQueryInvitesContext* CallbackContext = new FQueryInvitesContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionsHandle = SessionsHandle;
	CallbackContext->LocalUserId = LocalUserId;
	
	// Call EOS SDK to query invites
	EOS_Sessions_QueryInvites(SessionsHandle, &QueryOptions, CallbackContext,
		[](const EOS_Sessions_QueryInvitesCallbackInfo* Data)
		{
			FQueryInvitesContext* Context = static_cast<FQueryInvitesContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully queried invites!"));
					
					// Get the count of invites
					EOS_Sessions_GetInviteCountOptions CountOptions = {};
					CountOptions.ApiVersion = EOS_SESSIONS_GETINVITECOUNT_API_LATEST;
					CountOptions.LocalUserId = Context->LocalUserId;
					
					uint32_t InviteCount = EOS_Sessions_GetInviteCount(Context->SessionsHandle, &CountOptions);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Found %d invites"), InviteCount);
					
					// Log invite details
					for (uint32_t i = 0; i < InviteCount; i++)
					{
						EOS_Sessions_CopySessionHandleByInviteIdOptions CopyOptions = {};
						CopyOptions.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST;
						
						// Get invite ID first
						EOS_Sessions_GetInviteIdByIndexOptions IdOptions = {};
						IdOptions.ApiVersion = EOS_SESSIONS_GETINVITEIDBYINDEX_API_LATEST;
						IdOptions.LocalUserId = Context->LocalUserId;
						IdOptions.Index = i;
						
						char InviteIdBuffer[256];
						int32_t InviteIdLength = sizeof(InviteIdBuffer);
						EOS_EResult GetIdResult = EOS_Sessions_GetInviteIdByIndex(Context->SessionsHandle, &IdOptions, 
							InviteIdBuffer, &InviteIdLength);
						
						if (GetIdResult == EOS_EResult::EOS_Success)
						{
							UE_LOG(LogTemp, Log, TEXT("EOSKit: Invite #%d ID: %s"), i + 1, UTF8_TO_TCHAR(InviteIdBuffer));
						}
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast(InviteCount);
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to query invites: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast(0);
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
