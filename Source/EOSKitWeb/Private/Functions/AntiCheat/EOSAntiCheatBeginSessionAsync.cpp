// Copyright (C) 2024, All Rights Reserved.

#include "Functions/AntiCheat/EOSAntiCheatBeginSessionAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_anticheatclient.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSAntiCheatBeginSessionAsync* UEOSAntiCheatBeginSessionAsync::BeginAntiCheatSession(
	UObject* WorldContextObject,
	const FString& LocalUserId,
	int32 Mode)
{
	UEOSAntiCheatBeginSessionAsync* Node = NewObject<UEOSAntiCheatBeginSessionAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->UserId = LocalUserId;
	Node->AntiCheatMode = Mode;
	return Node;
}

void UEOSAntiCheatBeginSessionAsync::Activate()
{
	PerformBeginSession();
}

void UEOSAntiCheatBeginSessionAsync::PerformBeginSession()
{
	if (!WorldContextObject)
	{
		OnFail.Broadcast(TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFail.Broadcast(TEXT("Failed to get Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFail.Broadcast(TEXT("EOSKit Subsystem not initialized"));
		SetReadyToDestroy();
		return;
	}

	EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
	EOS_HAntiCheatClient AntiCheatHandle = EOS_Platform_GetAntiCheatClientInterface(PlatformHandle);

	if (!AntiCheatHandle)
	{
		OnFail.Broadcast(TEXT("Failed to get AntiCheat Client interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert user ID
	EOS_ProductUserId LocalPUID = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserId));
	if (!LocalPUID)
	{
		OnFail.Broadcast(TEXT("Invalid Product User ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup begin session options
	EOS_AntiCheatClient_BeginSessionOptions SessionOptions = {};
	SessionOptions.ApiVersion = EOS_ANTICHEATCLIENT_BEGINSESSION_API_LATEST;
	SessionOptions.LocalUserId = LocalPUID;
	
	// Set mode (Client = 0, Server = 1)
	SessionOptions.Mode = (AntiCheatMode == 0) ? EOS_EAntiCheatClientMode::EOS_ACCM_ClientServer : EOS_EAntiCheatClientMode::EOS_ACCM_PeerToPeer;

	UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Beginning Anti-Cheat client session"));

	// Begin session
	EOS_EResult Result = EOS_AntiCheatClient_BeginSession(AntiCheatHandle, &SessionOptions);

	AsyncTask(ENamedThreads::GameThread, [this, Result]()
	{
		if (Result == EOS_EResult::EOS_Success)
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Anti-Cheat session started successfully"));
			OnSuccess.Broadcast();
		}
		else
		{
			FString ErrorMessage = FString::Printf(TEXT("Begin Anti-Cheat session failed: %s"), 
				UTF8_TO_TCHAR(EOS_EResult_ToString(Result)));
			UE_LOG(LogTemp, Error, TEXT("EOSKitWeb: %s"), *ErrorMessage);
			OnFail.Broadcast(ErrorMessage);
		}

		SetReadyToDestroy();
	});
}
