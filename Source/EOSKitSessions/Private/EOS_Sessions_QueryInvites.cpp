// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_QueryInvites.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_sessions.h"
#include "Async/Async.h"

UEOS_Sessions_QueryInvites* UEOS_Sessions_QueryInvites::EOS_Sessions_QueryInvites(const FEOSKitProductUserId& LocalUserId)
{
	UEOS_Sessions_QueryInvites* Node = NewObject<UEOS_Sessions_QueryInvites>();
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOS_Sessions_QueryInvites::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to query invites - World is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, FEOSKitProductUserId());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to query invites - GameInstance is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, FEOSKitProductUserId());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to query invites - EOSKit Subsystem or Platform Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, FEOSKitProductUserId());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to query invites - Sessions Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, FEOSKitProductUserId());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to query invites - Invalid Local User ID"));
		OnCallback.Broadcast(EEOSKitResult::InvalidUser, FEOSKitProductUserId());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_QueryInvitesOptions QueryInvitesOptions = {};
	QueryInvitesOptions.ApiVersion = EOS_SESSIONS_QUERYINVITES_API_LATEST;
	QueryInvitesOptions.LocalUserId = LocalUserIdEOS;

	EOS_Sessions_QueryInvites(SessionsHandle, &QueryInvitesOptions, this, &UEOS_Sessions_QueryInvites::OnQueryInvitesCallback);
}

void UEOS_Sessions_QueryInvites::OnQueryInvitesCallback(const EOS_Sessions_QueryInvitesCallbackInfo* Data)
{
	if (UEOS_Sessions_QueryInvites* Node = static_cast<UEOS_Sessions_QueryInvites*>(Data->ClientData))
	{
		FEOSKitProductUserId LocalUserId(Data->LocalUserId);
		AsyncTask(ENamedThreads::GameThread, [Node, Data, LocalUserId]()
		{
			Node->OnCallback.Broadcast(ConvertEOSResult(Data->ResultCode), LocalUserId);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

