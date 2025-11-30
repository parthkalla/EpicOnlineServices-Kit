// Copyright (C) 2024, All Rights Reserved.

#include "EOSUnlinkAccountAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_connect.h"
#include "eos_connect_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSUnlinkAccountAsync* UEOSUnlinkAccountAsync::UnlinkAccount(FString LocalUserId)
{
	UEOSUnlinkAccountAsync* Node = NewObject<UEOSUnlinkAccountAsync>();
	Node->VarLocalUserId = LocalUserId;
	return Node;
}

void UEOSUnlinkAccountAsync::Activate()
{
	PerformUnlink();
	Super::Activate();
}

void UEOSUnlinkAccountAsync::PerformUnlink()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Unlinking Account"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: WARNING: This will unlink the external auth credentials!"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnComplete.Broadcast("", false);
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
		UE_LOG(LogTemp, Error, TEXT("EOSKit: EOSKit Subsystem or Platform Handle is null"));
		OnComplete.Broadcast("", false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	
	if (!ConnectHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Connect Handle"));
		OnComplete.Broadcast("", false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Convert string Product User ID to EOS_ProductUserId
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarLocalUserId));
	if (!LocalUserId || !EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Product User ID format"));
		OnComplete.Broadcast("", false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Unlink Options
	EOS_Connect_UnlinkAccountOptions UnlinkOptions = {};
	UnlinkOptions.ApiVersion = EOS_CONNECT_UNLINKACCOUNT_API_LATEST;
	UnlinkOptions.LocalUserId = LocalUserId;
	
	// Create callback context
	struct FUnlinkContext
	{
		UEOSUnlinkAccountAsync* AsyncNode;
		FString UserId;
	};
	
	FUnlinkContext* CallbackContext = new FUnlinkContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->UserId = VarLocalUserId;
	
	// Call EOS SDK
	EOS_Connect_UnlinkAccount(ConnectHandle, &UnlinkOptions, CallbackContext,
		[](const EOS_Connect_UnlinkAccountCallbackInfo* Data)
		{
			FUnlinkContext* Context = static_cast<FUnlinkContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					// Convert Product User ID to string
					char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
					int32_t BufferSize = sizeof(UserIdStr);
					EOS_ProductUserId_ToString(Data->LocalUserId, UserIdStr, &BufferSize);
					FString UnlinkedUserId = UTF8_TO_TCHAR(UserIdStr);
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully unlinked account!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Product User ID: %s"), *UnlinkedUserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: External auth credentials have been removed from keychain"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: User can now re-link with different account"));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(UnlinkedUserId, true);
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to unlink account: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not found or not logged in"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - Account was not authenticated in current session"));
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Security: Can only unlink accounts logged in during current session"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_NotFound)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: No external account found to unlink"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidAuth)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid auth - User not properly authenticated"));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast("", false);
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				
				delete Context;
			});
		});
}
