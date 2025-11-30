// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_RejectInvite.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_sessions.h"
#include "Async/Async.h"

UEOS_Sessions_RejectInvite* UEOS_Sessions_RejectInvite::EOS_Sessions_RejectInvite(const FString& InviteId, const FEOSKitProductUserId& LocalUserId)
{
	UEOS_Sessions_RejectInvite* Node = NewObject<UEOS_Sessions_RejectInvite>();
	Node->Var_InviteId = InviteId;
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOS_Sessions_RejectInvite::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to reject invite - World is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to reject invite - GameInstance is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to reject invite - EOSKit Subsystem or Platform Handle is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to reject invite - Sessions Handle is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to reject invite - Invalid Local User ID"));
		OnCallback.Broadcast(EEOSKitResult::InvalidUser);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_RejectInviteOptions RejectInviteOptions = {};
	RejectInviteOptions.ApiVersion = EOS_SESSIONS_REJECTINVITE_API_LATEST;
	RejectInviteOptions.InviteId = TCHAR_TO_ANSI(*Var_InviteId);
	RejectInviteOptions.LocalUserId = LocalUserIdEOS;

	EOS_Sessions_RejectInvite(SessionsHandle, &RejectInviteOptions, this, &UEOS_Sessions_RejectInvite::OnRejectInviteCallback);
}

void UEOS_Sessions_RejectInvite::OnRejectInviteCallback(const EOS_Sessions_RejectInviteCallbackInfo* Data)
{
	if (UEOS_Sessions_RejectInvite* Node = static_cast<UEOS_Sessions_RejectInvite*>(Data->ClientData))
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

