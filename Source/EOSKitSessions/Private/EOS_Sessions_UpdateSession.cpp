// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_UpdateSession.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOS_Sessions_UpdateSession* UEOS_Sessions_UpdateSession::EOS_Sessions_UpdateSession(const FEOSKitHSessionModification& SessionModificationHandle)
{
	UEOS_Sessions_UpdateSession* Node = NewObject<UEOS_Sessions_UpdateSession>();
	Node->Var_SessionModificationHandle = SessionModificationHandle;
	return Node;
}

void UEOS_Sessions_UpdateSession::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to update session - World is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, TEXT(""), TEXT(""));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to update session - GameInstance is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, TEXT(""), TEXT(""));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to update session - EOSKit Subsystem or Platform Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, TEXT(""), TEXT(""));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to update session - Sessions Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, TEXT(""), TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_HSessionModification SessionModHandle = Var_SessionModificationHandle.GetEOSHandle();
	if (!SessionModHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to update session - Invalid Session Modification Handle"));
		OnCallback.Broadcast(EEOSKitResult::InvalidParameters, TEXT(""), TEXT(""));
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_UpdateSessionOptions UpdateSessionOptions = {};
	UpdateSessionOptions.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
	UpdateSessionOptions.SessionModificationHandle = SessionModHandle;

	::EOS_Sessions_UpdateSession(SessionsHandle, &UpdateSessionOptions, this, &UEOS_Sessions_UpdateSession::OnUpdateSessionCallback);
}

void UEOS_Sessions_UpdateSession::OnUpdateSessionCallback(const EOS_Sessions_UpdateSessionCallbackInfo* Data)
{
	if (UEOS_Sessions_UpdateSession* Node = static_cast<UEOS_Sessions_UpdateSession*>(Data->ClientData))
	{
		FString SessionName = Data->SessionName ? UTF8_TO_TCHAR(Data->SessionName) : TEXT("");
		FString SessionId = Data->SessionId ? UTF8_TO_TCHAR(Data->SessionId) : TEXT("");

		AsyncTask(ENamedThreads::GameThread, [Node, Data, SessionName, SessionId]()
		{
			Node->OnCallback.Broadcast(ConvertEOSResult(Data->ResultCode), SessionName, SessionId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

