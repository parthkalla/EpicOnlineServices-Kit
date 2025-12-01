// Copyright (C) 2024, All Rights Reserved.

#include "EOSJoinEOKSessionAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSJoinEOKSessionAsync* UEOSJoinEOKSessionAsync::JoinEOKSession(
	FName SessionName,
	FEOSKitSessionFindResult SessionResult,
	bool bUsePresence)
{
	UEOSJoinEOKSessionAsync* Node = NewObject<UEOSJoinEOKSessionAsync>();
	Node->VSessionName = SessionName;
	Node->SessionResult = SessionResult;
	Node->bUsePresence = bUsePresence;
	return Node;
}

void UEOSJoinEOKSessionAsync::Activate()
{
	JoinSession();
	Super::Activate();
}

void UEOSJoinEOKSessionAsync::JoinSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Joining EOS Session via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Target Session: %s"), *SessionResult.SessionName);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	//  IMPORTANT LIMITATION:
	// To properly join a session using EOS SDK, you need the EOS_HSessionDetails handle
	// from the search results. However, this handle cannot be stored in a USTRUCT
	// because it's a C++ pointer type not compatible with Blueprint.
	//
	// WORKAROUND OPTIONS:
	// 1. Store the session handle in a separate manager class
	// 2. Search for the session again using the SessionId before joining
	// 3. Use a different architecture that passes the handle through C++ code
	//
	// For now, this implementation will fail with a documented error.
	// You need to implement one of the workarounds above in your game code.
	
	UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: JOIN SESSION LIMITATION"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: EOS_HSessions Details handle is required but cannot be passed through Blueprint"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: SOLUTION: Create a C++ Session Manager class that stores handles"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: Or use the legacy UEOSJoinLobbyAsync which uses IOnlineSubsystem"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
	
	OnFail.Broadcast();
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
	
	/* REFERENCE IMPLEMENTATION (requires EOS_HSessionDetails handle):
	
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast();
		SetReadyToDestroy();
		return;
	}
	
	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: EOSKit Subsystem or Platform Handle is null"));
		OnFail.Broadcast();
		SetReadyToDestroy();
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HSessions SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformHandle);
	
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Sessions Handle"));
		OnFail.Broadcast();
		SetReadyToDestroy();
		return;
	}
	
	// Get Product User ID
	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!LocalUserId)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Product User ID is null - user must be logged in"));
		OnFail.Broadcast();
		SetReadyToDestroy();
		return;
	}
	
	// Setup Join Session Options
	EOS_Sessions_JoinSessionOptions JoinOptions = {};
	JoinOptions.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*VSessionName.ToString());
	JoinOptions.SessionName = SessionNameConverter.Get();
	JoinOptions.LocalUserId = LocalUserId;
	JoinOptions.SessionHandle = nullptr; // THIS IS THE PROBLEM - needs EOS_HSessionDetails from search
	JoinOptions.bPresenceEnabled = bUsePresence ? EOS_TRUE : EOS_FALSE;
	
	// Create callback context
	struct FSessionJoinContext
	{
		UEOSJoinEOKSessionAsync* AsyncNode;
		FString SessionName;
	};
	
	FSessionJoinContext* CallbackContext = new FSessionJoinContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	
	// Call EOS SDK to join the session
	EOS_Sessions_JoinSession(SessionsHandle, &JoinOptions, CallbackContext,
		[](const EOS_Sessions_JoinSessionCallbackInfo* Data)
		{
			FSessionJoinContext* Context = static_cast<FSessionJoinContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully joined session!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to join session: %s"), 
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
	*/
}
