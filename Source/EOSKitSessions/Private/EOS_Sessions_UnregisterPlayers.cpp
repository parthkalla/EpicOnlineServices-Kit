// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_UnregisterPlayers.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_sessions.h"
#include "Async/Async.h"

UEOS_Sessions_UnregisterPlayers* UEOS_Sessions_UnregisterPlayers::EOS_Sessions_UnregisterPlayers(const FString& SessionName, const TArray<FEOSKitProductUserId>& PlayersToUnregister)
{
	UEOS_Sessions_UnregisterPlayers* Node = NewObject<UEOS_Sessions_UnregisterPlayers>();
	Node->Var_SessionName = SessionName;
	Node->Var_PlayersToUnregister = PlayersToUnregister;
	return Node;
}

void UEOS_Sessions_UnregisterPlayers::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to unregister players - World is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, TArray<FEOSKitProductUserId>());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to unregister players - GameInstance is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, TArray<FEOSKitProductUserId>());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to unregister players - EOSKit Subsystem or Platform Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, TArray<FEOSKitProductUserId>());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to unregister players - Sessions Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, TArray<FEOSKitProductUserId>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	if (Var_PlayersToUnregister.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to unregister players - No players to unregister"));
		OnCallback.Broadcast(EEOSKitResult::InvalidParameters, TArray<FEOSKitProductUserId>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_UnregisterPlayersOptions UnregisterPlayersOptions = {};
	UnregisterPlayersOptions.ApiVersion = EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST;
	UnregisterPlayersOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
	UnregisterPlayersOptions.PlayersToUnregisterCount = Var_PlayersToUnregister.Num();
	UnregisterPlayersOptions.PlayersToUnregister = new EOS_ProductUserId[Var_PlayersToUnregister.Num()];

	for (int32 i = 0; i < Var_PlayersToUnregister.Num(); i++)
	{
		UnregisterPlayersOptions.PlayersToUnregister[i] = Var_PlayersToUnregister[i].GetValueAsEosType();
	}

	EOS_Sessions_UnregisterPlayers(SessionsHandle, &UnregisterPlayersOptions, this, &UEOS_Sessions_UnregisterPlayers::OnUnregisterPlayersCallback);
	delete[] UnregisterPlayersOptions.PlayersToUnregister;
}

void UEOS_Sessions_UnregisterPlayers::OnUnregisterPlayersCallback(const EOS_Sessions_UnregisterPlayersCallbackInfo* Data)
{
	if (UEOS_Sessions_UnregisterPlayers* Node = static_cast<UEOS_Sessions_UnregisterPlayers*>(Data->ClientData))
	{
		TArray<FEOSKitProductUserId> UnregisteredPlayers;
		for (int32 i = 0; i < (int32)Data->UnregisteredPlayersCount; i++)
		{
			UnregisteredPlayers.Add(FEOSKitProductUserId(Data->UnregisteredPlayers[i]));
		}

		AsyncTask(ENamedThreads::GameThread, [Node, Data, UnregisteredPlayers]()
		{
			Node->OnCallback.Broadcast(ConvertEOSResult(Data->ResultCode), UnregisteredPlayers);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

