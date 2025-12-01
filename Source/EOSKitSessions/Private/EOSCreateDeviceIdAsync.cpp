// Copyright (C) 2024, All Rights Reserved.

#include "EOSCreateDeviceIdAsync.h"
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

UEOSCreateDeviceIdAsync* UEOSCreateDeviceIdAsync::CreateDeviceId(FString DeviceModel)
{
	UEOSCreateDeviceIdAsync* Node = NewObject<UEOSCreateDeviceIdAsync>();
	Node->VarDeviceModel = DeviceModel;
	return Node;
}

void UEOSCreateDeviceIdAsync::Activate()
{
	CreateDevice();
	Super::Activate();
}

void UEOSCreateDeviceIdAsync::CreateDevice()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Creating Device ID"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Device Model: %s"), *VarDeviceModel);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnComplete.Broadcast(false);
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
		OnComplete.Broadcast(false);
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
		OnComplete.Broadcast(false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate device model
	if (VarDeviceModel.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Device Model is empty - this is required"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Example: 'iPhone14,3', 'SM-G998B', 'Windows-PC', etc."));
		OnComplete.Broadcast(false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Setup Create Device ID Options
	EOS_Connect_CreateDeviceIdOptions CreateOptions = {};
	CreateOptions.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
	
	FTCHARToUTF8 DeviceModelConverter(*VarDeviceModel);
	CreateOptions.DeviceModel = DeviceModelConverter.Get();
	
	// Create callback context
	struct FCreateDeviceContext
	{
		UEOSCreateDeviceIdAsync* AsyncNode;
		FString DeviceModel;
	};
	
	FCreateDeviceContext* CallbackContext = new FCreateDeviceContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->DeviceModel = VarDeviceModel;
	
	// Call EOS SDK
	EOS_Connect_CreateDeviceId(ConnectHandle, &CreateOptions, CallbackContext,
		[](const EOS_Connect_CreateDeviceIdCallbackInfo* Data)
		{
			FCreateDeviceContext* Context = static_cast<FCreateDeviceContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Device ID created successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Device Model: %s"), *Context->DeviceModel);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Device ID is now registered for this device"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Use EOS_ECT_DEVICEID_ACCESS_TOKEN to login automatically"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: IMPORTANT: Link a real account to prevent data loss!"));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(true);
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to create Device ID: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_DuplicateNotAllowed)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Device ID already exists for this device"));
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Proceed with EOS_Connect_Login using EOS_ECT_DEVICEID_ACCESS_TOKEN"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - check Device Model format"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - platform configuration issue"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidAuth)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid auth - check EOS credentials configuration"));
					}
					
					if (Context->AsyncNode)
					{
						// Even if duplicate, this might be "successful" from user perspective
						bool bSuccess = (Data->ResultCode == EOS_EResult::EOS_DuplicateNotAllowed);
						Context->AsyncNode->OnComplete.Broadcast(bSuccess);
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
