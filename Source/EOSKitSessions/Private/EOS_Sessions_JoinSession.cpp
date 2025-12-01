// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_JoinSession.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOS_Sessions_JoinSession* UEOS_Sessions_JoinSession::EOK_Sessions_JoinSession(const FString& SessionName, const FEOSKitProductUserId& LocalUserId, const FEOSKitHSessionDetails& SessionHandle, bool bPresenceEnabled)
{
	UEOS_Sessions_JoinSession* Node = NewObject<UEOS_Sessions_JoinSession>();
	Node->Var_SessionName = SessionName;
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_SessionHandle = SessionHandle;
	Node->Var_bPresenceEnabled = bPresenceEnabled;
	return Node;
}

void UEOS_Sessions_JoinSession::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to join session - World is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to join session - GameInstance is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to join session - EOSKit Subsystem or Platform Handle is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to join session - Sessions Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_ProductUserId LocalUserIdEOS = Var_LocalUserId.GetValueAsEosType();
	if (!EOS_ProductUserId_IsValid(LocalUserIdEOS))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to join session - Invalid Local User ID"));
		OnCallback.Broadcast(EEOSKitResult::InvalidUser);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_HSessionDetails SessionDetailsHandle = Var_SessionHandle.GetEOSHandle();
	if (!SessionDetailsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to join session - Invalid Session Handle"));
		OnCallback.Broadcast(EEOSKitResult::InvalidParameters);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_JoinSessionOptions JoinSessionOptions = {};
	JoinSessionOptions.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
	JoinSessionOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
	JoinSessionOptions.LocalUserId = LocalUserIdEOS;
	JoinSessionOptions.SessionHandle = SessionDetailsHandle;
	JoinSessionOptions.bPresenceEnabled = Var_bPresenceEnabled ? EOS_TRUE : EOS_FALSE;

	::EOS_Sessions_JoinSession(SessionsHandle, &JoinSessionOptions, this, &UEOS_Sessions_JoinSession::OnJoinSessionCallback);
}

void UEOS_Sessions_JoinSession::OnJoinSessionCallback(const EOS_Sessions_JoinSessionCallbackInfo* Data)
{
	if (UEOS_Sessions_JoinSession* Node = static_cast<UEOS_Sessions_JoinSession*>(Data->ClientData))
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

