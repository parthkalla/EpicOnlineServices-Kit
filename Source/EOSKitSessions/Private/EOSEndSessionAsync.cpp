// Copyright (C) 2024, All Rights Reserved.

#include "EOSEndSessionAsync.h"
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

UEOSEndSessionAsync* UEOSEndSessionAsync::EndSession(FName SessionName)
{
	UEOSEndSessionAsync* Node = NewObject<UEOSEndSessionAsync>();
	Node->VSessionName = SessionName;
	return Node;
}

void UEOSEndSessionAsync::Activate()
{
	EndGameSession();
	Super::Activate();
}

void UEOSEndSessionAsync::EndGameSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Ending Session via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *VSessionName.ToString());
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
	
	// Setup End Session Options
	EOS_Sessions_EndSessionOptions EndOptions = {};
	EndOptions.ApiVersion = EOS_SESSIONS_ENDSESSION_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*VSessionName.ToString());
	EndOptions.SessionName = SessionNameConverter.Get();
	
	// Create callback context
	struct FEndSessionContext
	{
		UEOSEndSessionAsync* AsyncNode;
		FString SessionName;
	};
	
	FEndSessionContext* CallbackContext = new FEndSessionContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = VSessionName.ToString();
	
	// Call EOS SDK to end the session
	EOS_Sessions_EndSession(SessionsHandle, &EndOptions, CallbackContext,
		[](const EOS_Sessions_EndSessionCallbackInfo* Data)
		{
			FEndSessionContext* Context = static_cast<FEndSessionContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Session ended successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session is now marked as ended"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to end session: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_Sessions_OutOfSync)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Session is out of sync - it may have already ended or been destroyed"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Session is in invalid state - it may not have been started"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Session not found - it may have been destroyed"));
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
