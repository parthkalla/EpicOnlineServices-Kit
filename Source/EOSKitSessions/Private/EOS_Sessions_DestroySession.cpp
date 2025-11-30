// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_DestroySession.h"
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

UEOS_Sessions_DestroySession* UEOS_Sessions_DestroySession::EOS_Sessions_DestroySession(const FString& SessionName)
{
	UEOS_Sessions_DestroySession* Node = NewObject<UEOS_Sessions_DestroySession>();
	Node->Var_SessionName = SessionName;
	return Node;
}

void UEOS_Sessions_DestroySession::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to destroy session - World is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to destroy session - GameInstance is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState);
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to destroy session - EOSKit Subsystem or Platform Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound);
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to destroy session - Sessions Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_DestroySessionOptions DestroySessionOptions = {};
	DestroySessionOptions.ApiVersion = EOS_SESSIONS_DESTROYSESSION_API_LATEST;
	DestroySessionOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);

	EOS_Sessions_DestroySession(SessionsHandle, &DestroySessionOptions, this, &UEOS_Sessions_DestroySession::OnDestroySessionCallback);
}

void UEOS_Sessions_DestroySession::OnDestroySessionCallback(const EOS_Sessions_DestroySessionCallbackInfo* Data)
{
	if (UEOS_Sessions_DestroySession* Node = static_cast<UEOS_Sessions_DestroySession*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(ConvertEOSResult(Data->ResultCode));
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

