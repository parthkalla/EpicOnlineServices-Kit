// Copyright (C) 2024, All Rights Reserved.

#include "EOS_SessionSearch_Find.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_sessions.h"
#include "eos_sessions_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOS_SessionSearch_Find* UEOS_SessionSearch_Find::EOS_SessionSearch_Find(const FEOSKitHSessionSearch& SessionSearchHandle, const FEOSKitProductUserId& LocalUserId)
{
	UEOS_SessionSearch_Find* Node = NewObject<UEOS_SessionSearch_Find>();
	Node->Var_SessionSearchHandle = SessionSearchHandle;
	Node->Var_LocalUserId = LocalUserId;
	return Node;
}

void UEOS_SessionSearch_Find::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find session - World is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find session - GameInstance is null"));
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find session - EOSKit Subsystem or Platform Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_HSessionSearch SessionSearchHandleEOS = Var_SessionSearchHandle.GetEOSHandle();
	if (!SessionSearchHandleEOS)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find session - SessionSearchHandle is not valid"));
		OnCallback.Broadcast(EEOSKitResult::InvalidParameters);
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find session - Invalid Local User ID"));
		OnCallback.Broadcast(EEOSKitResult::InvalidUser);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_SessionSearch_FindOptions SessionSearchFindOptions = {};
	SessionSearchFindOptions.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
	SessionSearchFindOptions.LocalUserId = LocalUserIdEOS;

#if WITH_EOS_SDK
	::EOS_SessionSearch_Find(SessionSearchHandleEOS, &SessionSearchFindOptions, this, &UEOS_SessionSearch_Find::OnSessionSearch_FindCallback);
#endif
}

#if WITH_EOS_SDK
void UEOS_SessionSearch_Find::OnSessionSearch_FindCallback(const EOS_SessionSearch_FindCallbackInfo* Data)
{
	if (UEOS_SessionSearch_Find* Node = static_cast<UEOS_SessionSearch_Find*>(Data->ClientData))
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
#endif // WITH_EOS_SDK

