// Copyright (C) 2024, All Rights Reserved.

#include "EOSDestroyEOKSessionAsync.h"
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

void UEOSDestroyEOKSessionAsync::Activate()
{
	DestroySession();
	Super::Activate();
}

void UEOSDestroyEOKSessionAsync::DestroySession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Destroying EOS Session via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Session Name: %s"), *Var_SessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem to access the EOS Platform Handle
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
	
	// Setup Destroy Session Options
	EOS_Sessions_DestroySessionOptions DestroyOptions = {};
	DestroyOptions.ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
	
	FTCHARToUTF8 SessionNameConverter(*Var_SessionName.ToString());
	DestroyOptions.SessionName = SessionNameConverter.Get();
	
	// Create callback context
	struct FSessionDestroyContext
	{
		UEOSDestroyEOKSessionAsync* AsyncNode;
		FString SessionName;
	};
	
	FSessionDestroyContext* CallbackContext = new FSessionDestroyContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->SessionName = Var_SessionName.ToString();
	
	// Call EOS SDK to destroy the session
	EOS_Sessions_DestroySession(SessionsHandle, &DestroyOptions, CallbackContext,
		[](const EOS_Sessions_DestroySessionCallbackInfo* Data)
		{
			FSessionDestroyContext* Context = static_cast<FSessionDestroyContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Session destroyed successfully!"));
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to destroy session: %s"), 
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

UEOSDestroyEOKSessionAsync* UEOSDestroyEOKSessionAsync::DestroyEOKSession(FName SessionName)
{
	UEOSDestroyEOKSessionAsync* Node = NewObject<UEOSDestroyEOKSessionAsync>();
	Node->Var_SessionName = SessionName;
	return Node;
}
