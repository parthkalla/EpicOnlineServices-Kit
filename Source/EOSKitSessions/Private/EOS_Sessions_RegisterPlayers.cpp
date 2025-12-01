// Copyright (C) 2024, All Rights Reserved.

#include "EOS_Sessions_RegisterPlayers.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_sessions.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOS_Sessions_RegisterPlayers* UEOS_Sessions_RegisterPlayers::EOK_Sessions_RegisterPlayers(const FString& SessionName, const TArray<FEOSKitProductUserId>& PlayersToRegister)
{
	UEOS_Sessions_RegisterPlayers* Node = NewObject<UEOS_Sessions_RegisterPlayers>();
	Node->Var_SessionName = SessionName;
	Node->Var_PlayersToRegister = PlayersToRegister;
	return Node;
}

void UEOS_Sessions_RegisterPlayers::Activate()
{
	Super::Activate();

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to register players - World is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, TArray<FEOSKitProductUserId>(), TArray<FEOSKitProductUserId>());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to register players - GameInstance is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState, TArray<FEOSKitProductUserId>(), TArray<FEOSKitProductUserId>());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to register players - EOSKit Subsystem or Platform Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, TArray<FEOSKitProductUserId>(), TArray<FEOSKitProductUserId>());
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to register players - Sessions Handle is null"));
		OnCallback.Broadcast(EEOSKitResult::NotFound, TArray<FEOSKitProductUserId>(), TArray<FEOSKitProductUserId>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	if (Var_PlayersToRegister.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to register players - No players to register"));
		OnCallback.Broadcast(EEOSKitResult::InvalidParameters, TArray<FEOSKitProductUserId>(), TArray<FEOSKitProductUserId>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	EOS_Sessions_RegisterPlayersOptions RegisterPlayersOptions = {};
	RegisterPlayersOptions.ApiVersion = EOS_SESSIONS_REGISTERPLAYERS_API_LATEST;
	RegisterPlayersOptions.SessionName = TCHAR_TO_ANSI(*Var_SessionName);
	RegisterPlayersOptions.PlayersToRegisterCount = Var_PlayersToRegister.Num();
	RegisterPlayersOptions.PlayersToRegister = new EOS_ProductUserId[Var_PlayersToRegister.Num()];
	
	for (int32 i = 0; i < Var_PlayersToRegister.Num(); i++)
	{
		RegisterPlayersOptions.PlayersToRegister[i] = Var_PlayersToRegister[i].GetValueAsEosType();
	}

	::EOS_Sessions_RegisterPlayers(SessionsHandle, &RegisterPlayersOptions, this, &UEOS_Sessions_RegisterPlayers::OnRegisterPlayersCallback);
	// Note: PlayersToRegister array is copied by EOS SDK, safe to delete here
	delete[] RegisterPlayersOptions.PlayersToRegister;
}

void UEOS_Sessions_RegisterPlayers::OnRegisterPlayersCallback(const EOS_Sessions_RegisterPlayersCallbackInfo* Data)
{
	if (UEOS_Sessions_RegisterPlayers* Node = static_cast<UEOS_Sessions_RegisterPlayers*>(Data->ClientData))
	{
		TArray<FEOSKitProductUserId> RegisteredPlayers;
		for (int32 i = 0; i < (int32)Data->RegisteredPlayersCount; i++)
		{
			RegisteredPlayers.Add(FEOSKitProductUserId(Data->RegisteredPlayers[i]));
		}

		TArray<FEOSKitProductUserId> SanctionedPlayers;
		for (int32 i = 0; i < (int32)Data->SanctionedPlayersCount; i++)
		{
			SanctionedPlayers.Add(FEOSKitProductUserId(Data->SanctionedPlayers[i]));
		}

		AsyncTask(ENamedThreads::GameThread, [Node, Data, RegisteredPlayers, SanctionedPlayers]()
		{
			Node->OnCallback.Broadcast(ConvertEOSResult(Data->ResultCode), RegisteredPlayers, SanctionedPlayers);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

