// Copyright (C) 2024, All Rights Reserved.

#include "EOSQueryExternalAccountMappingsAsync.h"
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

UEOSQueryExternalAccountMappingsAsync* UEOSQueryExternalAccountMappingsAsync::QueryExternalAccountMappings(
	FString ProductUserId,
	TEnumAsByte<EEOSKitExternalAccountType> AccountType,
	const TArray<FString>& ExternalAccountIds)
{
	UEOSQueryExternalAccountMappingsAsync* Node = NewObject<UEOSQueryExternalAccountMappingsAsync>();
	Node->VarProductUserId = ProductUserId;
	Node->VarAccountType = AccountType;
	Node->VarExternalAccountIds = ExternalAccountIds;
	return Node;
}

void UEOSQueryExternalAccountMappingsAsync::Activate()
{
	QueryMappings();
	Super::Activate();
}

void UEOSQueryExternalAccountMappingsAsync::QueryMappings()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Querying External Account Mappings"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Product User ID: %s"), *VarProductUserId);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Account Type: %d"), VarAccountType.GetValue());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: External Account Count: %d"), VarExternalAccountIds.Num());
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
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*VarProductUserId));
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
	
	// Prepare external account IDs array
	const char** ExternalAccountIdsArray = new const char*[VarExternalAccountIds.Num()];
	for (int32 i = 0; i < VarExternalAccountIds.Num(); i++)
	{
		ExternalAccountIdsArray[i] = TCHAR_TO_UTF8(*VarExternalAccountIds[i]);
	}
	
	// Setup Query Options
	EOS_Connect_QueryExternalAccountMappingsOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_API_LATEST;
	QueryOptions.LocalUserId = LocalUserId;
	QueryOptions.AccountIdType = static_cast<EOS_EExternalAccountType>(VarAccountType.GetValue());
	QueryOptions.ExternalAccountIds = ExternalAccountIdsArray;
	QueryOptions.ExternalAccountIdCount = VarExternalAccountIds.Num();
	
	// Create callback context
	struct FQueryMappingsContext
	{
		UEOSQueryExternalAccountMappingsAsync* AsyncNode;
		FString ProductUserId;
		const char** ExternalAccountIdsArray;
	};
	
	FQueryMappingsContext* CallbackContext = new FQueryMappingsContext();
	CallbackContext->AsyncNode = this;
	CallbackContext->ProductUserId = VarProductUserId;
	CallbackContext->ExternalAccountIdsArray = ExternalAccountIdsArray;
	
	// Call EOS SDK
	EOS_Connect_QueryExternalAccountMappings(ConnectHandle, &QueryOptions, CallbackContext,
		[](const EOS_Connect_QueryExternalAccountMappingsCallbackInfo* Data)
		{
			FQueryMappingsContext* Context = static_cast<FQueryMappingsContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				if (Context)
				{
					delete[] Context->ExternalAccountIdsArray;
				}
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Successfully queried external account mappings!"));
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Product User ID: %s"), *Context->ProductUserId);
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnComplete.Broadcast(Context->ProductUserId, true);
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
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to query external account mappings: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
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
				
				// Cleanup
				delete[] Context->ExternalAccountIdsArray;
				delete Context;
			});
		});
}
