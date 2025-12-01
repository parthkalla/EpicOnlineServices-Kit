// Copyright (C) 2024, All Rights Reserved.

#include "EOSGetPUIDFromEpicIdAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitLoginHelpers.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_connect.h"
#include "eos_connect_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSGetPUIDFromEpicIdAsync* UEOSGetPUIDFromEpicIdAsync::GetPUIDFromEpicId(UObject* WorldContextObject, const TArray<FString>& TargetEpicAccountIds)
{
	UEOSGetPUIDFromEpicIdAsync* Node = NewObject<UEOSGetPUIDFromEpicIdAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->TargetEpicAccountIdStrings = TargetEpicAccountIds;
	return Node;
}

void UEOSGetPUIDFromEpicIdAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		SetReadyToDestroy();
		return;
	}

	// Get Product User ID from subsystem
	FString ProductUserIdString = UEOSKitLoginHelpers::GetProductUserIdString(WorldContextObject);
	if (ProductUserIdString.IsEmpty())
	{
		OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		SetReadyToDestroy();
		return;
	}

	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());
	if (!ConnectHandle)
	{
		OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		SetReadyToDestroy();
		return;
	}

	// Convert Product User ID
	EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserIdString));
	if (!EOS_ProductUserId_IsValid(LocalUserId))
	{
		OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		SetReadyToDestroy();
		return;
	}

	// Prepare external account IDs array
	if (TargetEpicAccountIdStrings.Num() == 0)
	{
		OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		SetReadyToDestroy();
		return;
	}

	TArray<const char*> ExternalAccountIdsArr;
	ExternalAccountIdsArr.Reserve(TargetEpicAccountIdStrings.Num());
	TArray<FTCHARToUTF8, TInlineAllocator<32>> Converters;
	Converters.Reserve(TargetEpicAccountIdStrings.Num());
	
	for (const FString& EpicAccountId : TargetEpicAccountIdStrings)
	{
		Converters.Emplace(*EpicAccountId);
		ExternalAccountIdsArr.Add(Converters.Last().Get());
	}

	// Setup options
	EOS_Connect_QueryExternalAccountMappingsOptions Options = {};
	Options.ApiVersion = EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_API_LATEST;
	Options.LocalUserId = LocalUserId;
	Options.AccountIdType = EOS_EExternalAccountType::EOS_EAT_EPIC;
	Options.ExternalAccountIds = ExternalAccountIdsArr.GetData();
	Options.ExternalAccountIdCount = ExternalAccountIdsArr.Num();

	// Query external account mappings
	EOS_Connect_QueryExternalAccountMappings(ConnectHandle, &Options, this, &UEOSGetPUIDFromEpicIdAsync::OnQueryExternalAccountMappingsComplete);
}

void EOS_CALL UEOSGetPUIDFromEpicIdAsync::OnQueryExternalAccountMappingsComplete(const EOS_Connect_QueryExternalAccountMappingsCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSGetPUIDFromEpicIdAsync* Self = static_cast<UEOSGetPUIDFromEpicIdAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Get Product User IDs for each Epic Account ID
		TArray<FEOSKitProductUserIdAndEpicId> ProductUserIdStrings;

		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject))
		{
			UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
			if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
			{
				EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(EOSSubsystem->GetPlatformHandle());
				if (ConnectHandle)
				{
					FString ProductUserIdString = UEOSKitLoginHelpers::GetProductUserIdString(Self->WorldContextObject);
					EOS_ProductUserId LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserIdString));

					for (const FString& EpicAccountId : Self->TargetEpicAccountIdStrings)
					{
						EOS_Connect_GetExternalAccountMappingsOptions GetOptions = {};
						GetOptions.ApiVersion = EOS_CONNECT_GETEXTERNALACCOUNTMAPPING_API_LATEST;
						GetOptions.AccountIdType = EOS_EExternalAccountType::EOS_EAT_EPIC;
						GetOptions.LocalUserId = LocalUserId;
						GetOptions.TargetExternalUserId = TCHAR_TO_UTF8(*EpicAccountId);

						EOS_ProductUserId PUID = EOS_Connect_GetExternalAccountMapping(ConnectHandle, &GetOptions);

						if (EOS_ProductUserId_IsValid(PUID))
						{
							char PUIDString[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
							int32_t BufferSize = sizeof(PUIDString);
							EOS_EResult Result = EOS_ProductUserId_ToString(PUID, PUIDString, &BufferSize);

							if (Result == EOS_EResult::EOS_Success)
							{
								FEOSKitProductUserIdAndEpicId UserInfo;
								UserInfo.EpicAccountId = EpicAccountId;
								UserInfo.ProductUserId = UTF8_TO_TCHAR(PUIDString);
								ProductUserIdStrings.Add(UserInfo);
							}
						}
					}

					Self->OnSuccess.Broadcast(ProductUserIdStrings);
				}
				else
				{
					Self->OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
				}
			}
			else
			{
				Self->OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
			}
		}
		else
		{
			Self->OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
		}
	}
	else
	{
		Self->OnFailure.Broadcast(TArray<FEOSKitProductUserIdAndEpicId>());
	}

	Self->SetReadyToDestroy();
}

