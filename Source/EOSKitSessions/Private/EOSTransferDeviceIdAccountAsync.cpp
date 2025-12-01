// Copyright (C) 2024, All Rights Reserved.

#include "EOSTransferDeviceIdAccountAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_connect.h"
#include "eos_connect_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSTransferDeviceIdAccountAsync* UEOSTransferDeviceIdAccountAsync::TransferDeviceIdAccount(
	FString PrimaryLocalUserId,
	FString LocalUserId,
	FString ProductUserIdToPreserve)
{
	UEOSTransferDeviceIdAccountAsync* Node = NewObject<UEOSTransferDeviceIdAccountAsync>();
	Node->VarPrimaryLocalUserId = PrimaryLocalUserId;
	Node->VarLocalUserId = LocalUserId;
	Node->VarProductUserIdToPreserve = ProductUserIdToPreserve;
	return Node;
}

void UEOSTransferDeviceIdAccountAsync::Activate()
{
	TransferAccount();
	Super::Activate();
}

void UEOSTransferDeviceIdAccountAsync::TransferAccount()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Transferring Device ID Account"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Primary User ID: %s"), *VarPrimaryLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local Device User ID: %s"), *VarLocalUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Product User ID to Preserve: %s"), *VarProductUserIdToPreserve);
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
	
	// Convert string Product User IDs to EOS_ProductUserId
	EOS_ProductUserId PrimaryUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarPrimaryLocalUserId));
	if (!PrimaryUserId || !EOS_ProductUserId_IsValid(PrimaryUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Primary User ID format"));
		OnComplete.Broadcast("", false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	EOS_ProductUserId LocalDeviceUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarLocalUserId));
	if (!LocalDeviceUserId || !EOS_ProductUserId_IsValid(LocalDeviceUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Local Device User ID format"));
		OnComplete.Broadcast("", false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	EOS_ProductUserId UserIdToPreserve = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarProductUserIdToPreserve));
	if (!UserIdToPreserve || !EOS_ProductUserId_IsValid(UserIdToPreserve))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Product User ID to Preserve format"));
		OnComplete.Broadcast("", false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Transfer Options
	EOS_Connect_TransferDeviceIdAccountOptions TransferOptions = {};
	TransferOptions.ApiVersion = EOS_CONNECT_TRANSFERDEVICEIDACCOUNT_API_LATEST;
	TransferOptions.PrimaryLocalUserId = PrimaryUserId;
	TransferOptions.LocalDeviceUserId = LocalDeviceUserId;
	TransferOptions.ProductUserIdToPreserve = UserIdToPreserve;
	
	// Create callback context
	struct FTransferContext
	{
		UEOSTransferDeviceIdAccountAsync* AsyncNode;
		FString PreservedUserId;
	};
	
	FTransferContext* CallbackContext = new FTransferContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->PreservedUserId = VarProductUserIdToPreserve;
	
	// Call EOS SDK
	EOS_Connect_TransferDeviceIdAccount(ConnectHandle, &TransferOptions, CallbackContext,
		[](const EOS_Connect_TransferDeviceIdAccountCallbackInfo* Data)
		{
			FTransferContext* Context = static_cast<FTransferContext*>(Data->ClientData);
			
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
					// Convert preserved Product User ID to string
					char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
					int32_t BufferSize = sizeof(UserIdStr);
					EOS_ProductUserId_ToString(Data->LocalUserId, UserIdStr, &BufferSize);
					FString PreservedUserId = UTF8_TO_TCHAR(UserIdStr);
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully transferred Device ID account!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Preserved Product User ID: %s"), *PreservedUserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: The other product user has been permanently discarded"));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(PreservedUserId, true);
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to transfer Device ID account: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - one or more Product User IDs are invalid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidState)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid state - transfer not allowed in current state"));
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
