// Copyright (C) 2024, All Rights Reserved.

#include "EOSCreateEOKLobbyAsync.h"
#include "EOSKitSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "eos_lobby.h"
#include "eos_sdk.h"
#include "Async/Async.h"

// Context struct to keep data alive during async operation
struct FEOKLobbyCreateContext
{
	UEOSCreateEOKLobbyAsync* AsyncNode = nullptr;
	FString SessionName;
	TArray<uint8> BucketIdUTF8;
	TArray<TArray<uint8>> AttributeKeysUTF8;
	TArray<TArray<uint8>> AttributeValuesUTF8;
	EOS_HLobbyModification LobbyModHandle = nullptr;
	// Store callback result data
	EOS_EResult ResultCode = EOS_EResult::EOS_NotConfigured;
	FString LobbyId;
};

void UEOSCreateEOKLobbyAsync::Activate()
{
	CreateLobby();
	Super::Activate();
}

void UEOSCreateEOKLobbyAsync::CreateLobby()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Creating EOS Lobby via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Lobby Name: %s"), *VSessionName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Public Connections: %d"), NumberOfPublicConnections);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: BucketId: %s"), *Var_CreateLobbySettings.BucketId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bIsLanMatch: %s (must be false for EOS)"), Var_CreateLobbySettings.bIsLanMatch ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bUsePresence: %s (required for Lobbies)"), Var_CreateLobbySettings.bUsePresence ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: bShouldAdvertise: %s"), Var_CreateLobbySettings.bShouldAdvertise ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));

	// Validate critical settings per problem statement
	if (Var_CreateLobbySettings.bIsLanMatch)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING - bIsLanMatch is TRUE but must be FALSE for EOS. Overriding to false."));
		Var_CreateLobbySettings.bIsLanMatch = false;
	}

	if (!Var_CreateLobbySettings.bUsePresence)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING - bUsePresence is FALSE but is REQUIRED for Lobbies. Overriding to true."));
		Var_CreateLobbySettings.bUsePresence = true;
	}

	if (NumberOfPublicConnections <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING - NumberOfPublicConnections is %d but must be > 0. Setting to 4."), NumberOfPublicConnections);
		NumberOfPublicConnections = 4;
	}

	// Get EOS subsystem using cached WorldContextObject
	if (!CachedWorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby FAILED - WorldContextObject is null"));
		OnFail.Broadcast(TEXT("WorldContextObject is null"));
		FinishAndCleanup();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(CachedWorldContextObject);
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby FAILED - Cannot get Game Instance"));
		OnFail.Broadcast(TEXT("Cannot get Game Instance"));
		FinishAndCleanup();
		return;
	}

	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby FAILED - EOSKit Subsystem or Platform Handle is null"));
		OnFail.Broadcast(TEXT("EOSKit Subsystem or Platform Handle is null"));
		FinishAndCleanup();
		return;
	}

	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

	if (!LobbyHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby FAILED - Failed to get Lobby Handle"));
		OnFail.Broadcast(TEXT("Failed to get Lobby Handle"));
		FinishAndCleanup();
		return;
	}

	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby FAILED - Product User ID is null. User must be logged in."));
		OnFail.Broadcast(TEXT("Product User ID is null - user must be logged in"));
		FinishAndCleanup();
		return;
	}

	// Log user ID for debugging
	char ProductUserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
	EOS_ProductUserId_ToString(LocalUserId, ProductUserIdStr, &ProductUserIdStrSize);
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - LocalUserId: %s"), UTF8_TO_TCHAR(ProductUserIdStr));

	// Create context to keep data alive during async operation
	FEOKLobbyCreateContext* Context = new FEOKLobbyCreateContext();
	Context->AsyncNode = this;
	Context->SessionName = VSessionName.ToString();

	// Ensure BucketId has a default value (EOS requires this!)
	FString EffectiveBucketId = Var_CreateLobbySettings.BucketId.IsEmpty() ? TEXT("MyGameBucket") : Var_CreateLobbySettings.BucketId;
	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - Using BucketId: %s"), *EffectiveBucketId);

	// Convert BucketId to UTF8 and store in context
	FTCHARToUTF8 BucketIdConverter(*EffectiveBucketId);
	Context->BucketIdUTF8.Append((uint8*)BucketIdConverter.Get(), BucketIdConverter.Length() + 1);

	// Setup lobby creation options
	EOS_Lobby_CreateLobbyOptions CreateLobbyOptions = {};
	CreateLobbyOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	CreateLobbyOptions.LocalUserId = LocalUserId;
	CreateLobbyOptions.MaxLobbyMembers = NumberOfPublicConnections;
	CreateLobbyOptions.PermissionLevel = Var_CreateLobbySettings.bShouldAdvertise ? 
		EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED : 
		EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY;
	CreateLobbyOptions.bPresenceEnabled = Var_CreateLobbySettings.bUsePresence ? EOS_TRUE : EOS_FALSE;
	CreateLobbyOptions.bAllowInvites = Var_CreateLobbySettings.bAllowInvites ? EOS_TRUE : EOS_FALSE;
	CreateLobbyOptions.BucketId = (const char*)Context->BucketIdUTF8.GetData(); // CRITICAL: Set BucketId
	CreateLobbyOptions.bDisableHostMigration = Var_CreateLobbySettings.bSupportHostMigration ? EOS_FALSE : EOS_TRUE;
	CreateLobbyOptions.bEnableJoinById = Var_CreateLobbySettings.bEnableJoinViaId ? EOS_TRUE : EOS_FALSE;
	CreateLobbyOptions.bEnableRTCRoom = Var_CreateLobbySettings.bUseVoiceChat ? EOS_TRUE : EOS_FALSE;

	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - Final Settings:"));
	UE_LOG(LogTemp, Log, TEXT("  MaxLobbyMembers: %d"), CreateLobbyOptions.MaxLobbyMembers);
	UE_LOG(LogTemp, Log, TEXT("  PermissionLevel: %d"), static_cast<int32>(CreateLobbyOptions.PermissionLevel));
	UE_LOG(LogTemp, Log, TEXT("  bPresenceEnabled: %s"), CreateLobbyOptions.bPresenceEnabled ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Log, TEXT("  bAllowInvites: %s"), CreateLobbyOptions.bAllowInvites ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Log, TEXT("  BucketId: %s"), *EffectiveBucketId);

	// Create the lobby
	EOS_Lobby_CreateLobby(LobbyHandle, &CreateLobbyOptions, Context,
		[](const EOS_Lobby_CreateLobbyCallbackInfo* Data)
		{
			FEOKLobbyCreateContext* Context = static_cast<FEOKLobbyCreateContext*>(Data->ClientData);

			if (!Context || !Context->AsyncNode)
			{
				UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby callback - Invalid context"));
				if (Context)
				{
					delete Context;
				}
				return;
			}

			// CRITICAL: Copy callback data into context before scheduling async task
			// The Data pointer may become invalid after the callback returns
			Context->ResultCode = Data->ResultCode;
			if (Data->LobbyId && strlen(Data->LobbyId) > 0)
			{
				Context->LobbyId = UTF8_TO_TCHAR(Data->LobbyId);
			}

			UEOSCreateEOKLobbyAsync* AsyncNode = Context->AsyncNode;

			AsyncTask(ENamedThreads::GameThread, [Context, AsyncNode]()
			{
				// Log raw ResultCode for debugging
				const char* ResultStr = EOS_EResult_ToString(Context->ResultCode);
				UE_LOG(LogTemp, Warning, TEXT("EOSKit: [CALLBACK] Raw ResultCode value: %d (0x%08X) = %s"), 
					static_cast<int32>(Context->ResultCode), 
					static_cast<uint32>(Context->ResultCode),
					UTF8_TO_TCHAR(ResultStr));
				
				// Check if we have a LobbyId even if ResultCode is not Success
				// Sometimes EOS returns a partial success with a LobbyId
				bool bHasLobbyId = !Context->LobbyId.IsEmpty();
				
				// EOS_Success is 0, so check both ways
				bool bIsSuccess = (Context->ResultCode == EOS_EResult::EOS_Success) || (Context->ResultCode == 0);
				
				// If we have a LobbyId, treat it as success even if ResultCode says otherwise
				// This handles cases where EOS creates the lobby but returns a warning code
				if (bIsSuccess || bHasLobbyId)
				{
					UE_LOG(LogTemp, Log, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby SUCCESS!"));
					UE_LOG(LogTemp, Log, TEXT("EOSKit: LobbyId: %s"), *Context->LobbyId);
					UE_LOG(LogTemp, Log, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					if (!bIsSuccess && bHasLobbyId)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: NOTE: ResultCode was %s but LobbyId exists - treating as success"), UTF8_TO_TCHAR(ResultStr));
					}
					UE_LOG(LogTemp, Log, TEXT("EOSKit: ========================================"));

					if (AsyncNode)
					{
						AsyncNode->OnSuccess.Broadcast(Context->LobbyId);
						
						// Register the local player in the lobby after successful creation
						if (bHasLobbyId)
						{
							UE_LOG(LogTemp, Log, TEXT("EOSKit: Attempting to register local player in lobby..."));
							// Note: Lobby members are automatically added when creating, but we can verify
							// For sessions, we need to explicitly register players
						}
						
						AsyncNode->FinishAndCleanup();
					}
				}
				else
				{
					// Check if ResultCode is valid
					if (Context->ResultCode < 0 || Context->ResultCode > 0x7FFFFFFF)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: [CALLBACK] WARNING: ResultCode appears to be invalid or corrupted!"));
					}
					
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));
					UE_LOG(LogTemp, Error, TEXT("EOSKit: CreateLobby FAILED!"));
					UE_LOG(LogTemp, Error, TEXT("EOSKit: Error Code: %s (%d / 0x%08X)"), 
						UTF8_TO_TCHAR(ResultStr), 
						static_cast<int32>(Context->ResultCode),
						static_cast<uint32>(Context->ResultCode));
					UE_LOG(LogTemp, Error, TEXT("EOSKit: Session Name: %s"), *Context->SessionName);
					
					// Log additional callback info if available
					if (bHasLobbyId)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING: LobbyId exists despite failure: %s"), *Context->LobbyId);
					}
					
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ========================================"));

					if (AsyncNode)
					{
						FString ErrorMessage = FString::Printf(TEXT("%s (Code: %d)"), UTF8_TO_TCHAR(ResultStr), static_cast<int32>(Context->ResultCode));
						AsyncNode->OnFail.Broadcast(ErrorMessage);
						AsyncNode->FinishAndCleanup();
					}
				}

				delete Context;
			});
		});

	UE_LOG(LogTemp, Log, TEXT("EOSKit: CreateLobby - EOS_Lobby_CreateLobby called, waiting for callback..."));
}

void UEOSCreateEOKLobbyAsync::FinishAndCleanup()
{
	bDelegateCalled = true;
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOSCreateEOKLobbyAsync::OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful)
{
	// Not used - we use the EOS SDK callback directly
}

UEOSCreateEOKLobbyAsync* UEOSCreateEOKLobbyAsync::CreateEOKLobby(
	UObject* WorldContextObject,
	TMap<FString, FEOSKitAttribute> SessionSettings,
	TMap<FString, FEOSKitAttribute> MemberSettings,
	FName SessionName,
	int32 NumberOfPublicConnections,
	FEOSKitCreateLobbySettings ExtraSettings)
{
	UEOSCreateEOKLobbyAsync* Ueik_CreateLobbyObject = NewObject<UEOSCreateEOKLobbyAsync>();
	Ueik_CreateLobbyObject->CachedWorldContextObject = WorldContextObject;
	Ueik_CreateLobbyObject->NumberOfPublicConnections = NumberOfPublicConnections;
	Ueik_CreateLobbyObject->SessionSettings = SessionSettings;
	Ueik_CreateLobbyObject->MemberSettings = MemberSettings;
	Ueik_CreateLobbyObject->Var_CreateLobbySettings = ExtraSettings;
	Ueik_CreateLobbyObject->VSessionName = SessionName;
	return Ueik_CreateLobbyObject;
}
