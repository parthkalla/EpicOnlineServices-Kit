// Copyright (C) 2024, All Rights Reserved.

#include "EOSLinkAccountAsync.h"
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

UEOSLinkAccountAsync* UEOSLinkAccountAsync::LinkAccount(
	FString LocalProductUserId,
	FEOSKitContinuanceToken ContinuanceToken)
{
	UEOSLinkAccountAsync* Node = NewObject<UEOSLinkAccountAsync>();
	Node->VarLocalProductUserId = LocalProductUserId;
	Node->VarContinuanceToken = ContinuanceToken;
	return Node;
}

void UEOSLinkAccountAsync::Activate()
{
	PerformLink();
	Super::Activate();
}

void UEOSLinkAccountAsync::PerformLink()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Linking Account"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Local Product User ID: %s"), *VarLocalProductUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Continuance Token Present: %s"), VarContinuanceToken.ContinuanceToken.IsEmpty() ? TEXT("No") : TEXT("Yes"));
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
	
	// Validate Product User ID
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarLocalProductUserId));
	if (!LocalUserId || !EOS_ProductUserId_IsValid(LocalUserId))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid Product User ID format"));
		OnComplete.Broadcast(false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Validate Continuance Token
	if (VarContinuanceToken.ContinuanceToken.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Continuance Token is empty"));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Token should be obtained from failed login attempt"));
		OnComplete.Broadcast(false);
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Note: EOS SDK uses an opaque EOS_ContinuanceToken type
	// We need to store and pass the actual EOS token, not just a string
	// For now, we'll note this limitation in the logs
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: NOTE - Continuance Token handling requires direct EOS_ContinuanceToken type"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: This implementation uses string representation"));
	
	// Setup Link Account Options
	EOS_Connect_LinkAccountOptions LinkOptions = {};
	LinkOptions.ApiVersion = EOS_CONNECT_LINKACCOUNT_API_LATEST;
	LinkOptions.LocalUserId = LocalUserId;
	
	// Note: The actual EOS_ContinuanceToken would come from the login callback
	// This is a limitation of the string-based approach
	// In a production system, you'd need to pass the actual EOS_ContinuanceToken pointer
	LinkOptions.ContinuanceToken = nullptr; // Would be set from actual callback
	
	UE_LOG(LogTemp, Error, TEXT("EOSKit: Link Account requires direct EOS_ContinuanceToken from login callback"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: String-based token cannot be converted back to EOS type"));
	UE_LOG(LogTemp, Error, TEXT("EOSKit: Recommended: Handle linking in the login callback directly"));
	
	// Cannot proceed without valid EOS_ContinuanceToken
	OnComplete.Broadcast(false);
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
	
	// Commented out the actual EOS SDK call since we can't properly convert the token
	/*
	// Create callback context
	struct FLinkContext
	{
		UEOSLinkAccountAsync* AsyncNode;
		FString UserId;
	};
	
	FLinkContext* CallbackContext = new FLinkContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->UserId = VarLocalProductUserId;
	
	// Call EOS SDK
	EOS_Connect_LinkAccount(ConnectHandle, &LinkOptions, CallbackContext,
		[](const EOS_Connect_LinkAccountCallbackInfo* Data)
		{
			FLinkContext* Context = static_cast<FLinkContext*>(Data->ClientData);
			
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
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully linked account!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Product User ID: %s"), *Context->UserId);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: External account now linked"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: User can login with both Device ID and external account"));
					
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to link account: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					// Common error explanations
					if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid user - Product User ID not found"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_InvalidAuth)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Invalid auth - Continuance Token expired or invalid"));
					}
					else if (Data->ResultCode == EOS_EResult::EOS_DuplicateNotAllowed)
					{
						UE_LOG(LogTemp, Error, TEXT("EOSKit: Duplicate - External account already linked to another user"));
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(false);
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
	*/
}
