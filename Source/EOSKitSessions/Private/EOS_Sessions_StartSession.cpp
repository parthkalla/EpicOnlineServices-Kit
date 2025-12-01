// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_StartSession.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOS_Sessions_StartSession* UEOS_Sessions_StartSession::EOK_Sessions_StartSession(const FString& SessionName)
{
	UEOS_Sessions_StartSession* Node = NewObject<UEOS_Sessions_StartSession>();
	Node->Var_SessionName = SessionName;
	return Node;
}

void UEOS_Sessions_StartSession::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to start session - World is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to start session - GameInstance is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to start session - EOSKit Subsystem or Platform Handle is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to start session - Sessions Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_StartSessionOptions StartSessionOptions = {};
	StartSessionOptions.ApiVersion = EOS_SESSIONS_STARTSESSION_API_LATEST;
	StartSessionOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);

	::EOS_Sessions_StartSession(SessionsHandle, &StartSessionOptions, this, &UEOS_Sessions_StartSession::OnStartSessionCallback);
}

void UEOS_Sessions_StartSession::OnStartSessionCallback(const EOS_Sessions_StartSessionCallbackInfo* Data)
{
	if (UEOS_Sessions_StartSession* Node = static_cast<UEOS_Sessions_StartSession*>(Data->ClientData))
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

