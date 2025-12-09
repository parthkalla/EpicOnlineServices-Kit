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

UEOS_Sessions_StartSession* UEOS_Sessions_StartSession::EOK_Sessions_StartSession(UObject* WorldContextObject, const FString& SessionName)
{
	UEOS_Sessions_StartSession* Node = NewObject<UEOS_Sessions_StartSession>();
	Node->Var_SessionName = SessionName;
	Node->CachedWorldContextObject = WorldContextObject;
	return Node;
}

void UEOS_Sessions_StartSession::Activate()
{
	Super::Activate();

	// Try to get WorldContextObject - use cached one or try GetWorld()
	UObject* WorldContext = CachedWorldContextObject.Get();
	if (!WorldContext)
	{
		UWorld* World = GetWorld();
		WorldContext = World;
	}

	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to start session - WorldContextObject is null"));
		OnCallback.Broadcast(EEOSKitResult::InvalidState);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	// Get GameInstance using WorldContextObject (works even if GetWorld() returns null)
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext);
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

	// Validate session name
	if (Var_SessionName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to start session - Session Name is empty"));
		OnCallback.Broadcast(EEOSKitResult::InvalidParameters);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}

	// Convert to UTF8 - FTCHARToUTF8 stays alive for the duration of this function call
	// EOS SDK copies the string internally, so we don't need to keep it alive after the call
	FTCHARToUTF8 Utf8SessionName(*Var_SessionName);
	
	EOS_Sessions_StartSessionOptions StartSessionOptions = {};
	StartSessionOptions.ApiVersion = EOS_SESSIONS_STARTSESSION_API_LATEST;
	StartSessionOptions.SessionName = Utf8SessionName.Get();

	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Calling EOS_Sessions_StartSession for session: %s"), *Var_SessionName);
	::EOS_Sessions_StartSession(SessionsHandle, &StartSessionOptions, this, &UEOS_Sessions_StartSession::OnStartSessionCallback);
}

void UEOS_Sessions_StartSession::OnStartSessionCallback(const EOS_Sessions_StartSessionCallbackInfo* Data)
{
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: StartSession callback received null Data"));
		return;
	}

	UEOS_Sessions_StartSession* Node = static_cast<UEOS_Sessions_StartSession*>(Data->ClientData);
	if (!Node)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: StartSession callback received null Node"));
		return;
	}

	// Copy ResultCode before async task (Data pointer may become invalid)
	EOS_EResult ResultCode = Data->ResultCode;
	
	// Execute on game thread
	AsyncTask(ENamedThreads::GameThread, [Node, ResultCode]()
	{
		if (Node && IsValid(Node))
		{
			if (ResultCode == EOS_EResult::EOS_Success)
			{
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: ✅ Session started successfully via SDK!"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKit: ❌ Failed to start session: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(ResultCode)));
			}
			
			Node->OnCallback.Broadcast(ConvertEOSResult(ResultCode));
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			Node->MarkAsGarbage();
#else
			Node->MarkPendingKill();
#endif
		}
	});
}

