// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_SendInvite.h"
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

UEOS_Sessions_SendInvite* UEOS_Sessions_SendInvite::EOS_Sessions_SendInvite(const FString& SessionName, const FEOSKitProductUserId& LocalUserId, const FEOSKitProductUserId& TargetUserId)
{
	UEOS_Sessions_SendInvite* Node = NewObject<UEOS_Sessions_SendInvite>();
	Node->Var_SessionName = SessionName;
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOS_Sessions_SendInvite::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to send invite - World is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to send invite - GameInstance is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to send invite - EOSKit Subsystem or Platform Handle is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to send invite - Sessions Handle is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to send invite - Invalid Local User ID"));
		OnCallback.Broadcast(EEOSKitResult::InvalidUser);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_ProductUserId TargetUserIdEOS = Var_TargetUserId.GetValueAsEosType();
	if (!EOS_ProductUserId_IsValid(TargetUserIdEOS))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to send invite - Invalid Target User ID"));
		OnCallback.Broadcast(EEOSKitResult::InvalidUser);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_SendInviteOptions SendInviteOptions = {};
	SendInviteOptions.ApiVersion = EOS_SESSIONS_SENDINVITE_API_LATEST;
	SendInviteOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
	SendInviteOptions.LocalUserId = LocalUserIdEOS;
	SendInviteOptions.TargetUserId = TargetUserIdEOS;

	EOS_Sessions_SendInvite(SessionsHandle, &SendInviteOptions, this, &UEOS_Sessions_SendInvite::OnSendInviteCallback);
}

void UEOS_Sessions_SendInvite::OnSendInviteCallback(const EOS_Sessions_SendInviteCallbackInfo* Data)
{
	if (UEOS_Sessions_SendInvite* Node = static_cast<UEOS_Sessions_SendInvite*>(Data->ClientData))
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

