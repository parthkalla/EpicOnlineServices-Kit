// Copyright (C) 2024, All Rights Reserved.

#include "EOSVerifyIdTokenAsync.h"
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

UEOSVerifyIdTokenAsync* UEOSVerifyIdTokenAsync::VerifyIdToken(FEOSKitConnectIdToken IdToken)
{
	UEOSVerifyIdTokenAsync* Node = NewObject<UEOSVerifyIdTokenAsync>();
	Node->VarIdToken = IdToken;
	return Node;
}

void UEOSVerifyIdTokenAsync::Activate()
{
	PerformVerification();
	Super::Activate();
}

void UEOSVerifyIdTokenAsync::PerformVerification()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Verifying ID Token"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Token Length: %d"), VarIdToken.JsonWebToken.Len());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the EOSKit subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnComplete.Broadcast(FEOSKitVerifyIdTokenCallbackInfo());
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
		OnComplete.Broadcast(FEOSKitVerifyIdTokenCallbackInfo());
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
		OnComplete.Broadcast(FEOSKitVerifyIdTokenCallbackInfo());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate token
	if (VarIdToken.JsonWebToken.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: ID Token is empty"));
		OnComplete.Broadcast(FEOSKitVerifyIdTokenCallbackInfo());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Create EOS_Connect_IdToken structure
	EOS_Connect_IdToken EOSIdToken = {};
	EOSIdToken.ApiVersion = EOS_CONNECT_IDTOKEN_API_LATEST;
	
	FTCHARToUTF8 TokenConverter(*VarIdToken.JsonWebToken);
	EOSIdToken.JsonWebToken = TokenConverter.Get();
	
	// Setup Verify Options
	EOS_Connect_VerifyIdTokenOptions VerifyOptions = {};
	VerifyOptions.ApiVersion = EOS_CONNECT_VERIFYIDTOKEN_API_LATEST;
	VerifyOptions.IdToken = &EOSIdToken;
	
	// Create callback context
	struct FVerifyContext
	{
		UEOSVerifyIdTokenAsync* AsyncNode;
	};
	
	FVerifyContext* CallbackContext = new FVerifyContext();
	CallbackContext->AsyncNode = this;
	
	// Call EOS SDK
	EOS_Connect_VerifyIdToken(ConnectHandle, &VerifyOptions, CallbackContext,
		[](const EOS_Connect_VerifyIdTokenCallbackInfo* Data)
		{
			FVerifyContext* Context = static_cast<FVerifyContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				FEOSKitVerifyIdTokenCallbackInfo CallbackInfo;
				
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					CallbackInfo.bSuccess = true;
					
					// Convert Product User ID to string
					if (Data->ProductUserId)
					{
						char UserIdStr[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
						int32_t BufferSize = sizeof(UserIdStr);
						EOS_ProductUserId_ToString(Data->ProductUserId, UserIdStr, &BufferSize);
						CallbackInfo.ProductUserId = UTF8_TO_TCHAR(UserIdStr);
					}
					
					// Check if account info is present
					CallbackInfo.bIsAccountInfoPresent = (Data->bIsAccountInfoPresent == EOS_TRUE);
					
					if (CallbackInfo.bIsAccountInfoPresent)
					{
						// Fill in account information
						CallbackInfo.AccountType = static_cast<EEOSKitExternalAccountType>(Data->AccountIdType);
						
						if (Data->AccountId)
						{
							CallbackInfo.AccountId = UTF8_TO_TCHAR(Data->AccountId);
						}
						
						if (Data->Platform)
						{
							CallbackInfo.Platform = UTF8_TO_TCHAR(Data->Platform);
						}
						
						if (Data->DeviceType)
						{
							CallbackInfo.DeviceType = UTF8_TO_TCHAR(Data->DeviceType);
						}
						
						if (Data->ClientId)
						{
							CallbackInfo.ClientId = UTF8_TO_TCHAR(Data->ClientId);
						}
						
						if (Data->ProductId)
						{
							CallbackInfo.ProductId = UTF8_TO_TCHAR(Data->ProductId);
						}
						
						if (Data->SandboxId)
						{
							CallbackInfo.SandboxId = UTF8_TO_TCHAR(Data->SandboxId);
						}
						
						if (Data->DeploymentId)
						{
							CallbackInfo.DeploymentId = UTF8_TO_TCHAR(Data->DeploymentId);
						}
					}
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? ID Token verified successfully!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Product User ID: %s"), *CallbackInfo.ProductUserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Account Info Present: %s"), CallbackInfo.bIsAccountInfoPresent ? TEXT("Yes") : TEXT("No"));
					
					if (CallbackInfo.bIsAccountInfoPresent)
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Account ID: %s"), *CallbackInfo.AccountId);
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Platform: %s"), *CallbackInfo.Platform);
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Device Type: %s"), *CallbackInfo.DeviceType);
					}
				}
				else
				{
					CallbackInfo.bSuccess = false;
					
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to verify ID token: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidAuth)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid auth - Token is invalid or expired"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidParameters)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid parameters - Token format is incorrect"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_Auth_InvalidToken)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid token - Token verification failed"));
					}
				}
				
				if (Context->AsyncNode)
				{
					Context->AsyncNode->OnComplete.Broadcast(CallbackInfo);
					Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					Context->AsyncNode->MarkAsGarbage();
#else
					Context->AsyncNode->MarkPendingKill();
#endif
				}
				
				delete Context;
			});
		});
}
