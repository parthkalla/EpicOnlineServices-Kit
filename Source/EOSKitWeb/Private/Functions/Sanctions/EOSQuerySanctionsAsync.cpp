// Copyright (C) 2024, All Rights Reserved.

#include "Functions/Sanctions/EOSQuerySanctionsAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_sanctions.h"
#include "eos_sanctions_types.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSQuerySanctionsAsync* UEOSQuerySanctionsAsync::QuerySanctions(
	UObject* WorldContextObject,
	const FString& LocalUserId)
{
	UEOSQuerySanctionsAsync* Node = NewObject<UEOSQuerySanctionsAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->UserId = LocalUserId;
	return Node;
}

void UEOSQuerySanctionsAsync::Activate()
{
	PerformQuery();
}

void UEOSQuerySanctionsAsync::PerformQuery()
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
	EOS_HSanctions SanctionsHandle = EOS_Platform_GetSanctionsInterface(PlatformHandle);

	if (!SanctionsHandle)
	{
		OnFail.Broadcast(TEXT("Failed to get Sanctions interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert user ID
	EOS_ProductUserId TargetUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*UserId));
	if (!TargetUserId)
	{
		OnFail.Broadcast(TEXT("Invalid Product User ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup query options
	EOS_Sanctions_QueryActivePlayerSanctionsOptions QueryOptions = {};
	QueryOptions.ApiVersion = EOS_SANCTIONS_QUERYACTIVEPLAYERSANCTIONS_API_LATEST;
	QueryOptions.TargetUserId = TargetUserId;
	QueryOptions.LocalUserId = TargetUserId; // Usually the same user checking their own sanctions

	UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Querying active player sanctions for: %s"), *UserId);

	// Perform query - use lambda wrapper
	EOS_Sanctions_QueryActivePlayerSanctions(SanctionsHandle, &QueryOptions, this, [](const EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo* Data)
	{
		if (!Data || !Data->ClientData)
		{
			return;
		}

		UEOSQuerySanctionsAsync* Self = static_cast<UEOSQuerySanctionsAsync*>(Data->ClientData);

		AsyncTask(ENamedThreads::GameThread, [Self, Data]()
		{
			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject);
				if (GameInstance)
				{
					UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
					if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
					{
						EOS_HSanctions SanctionsHandle = EOS_Platform_GetSanctionsInterface(EOSSubsystem->GetPlatformHandle());
						if (SanctionsHandle)
						{
							// Get sanction count
							EOS_Sanctions_GetPlayerSanctionCountOptions CountOptions = {};
							CountOptions.ApiVersion = EOS_SANCTIONS_GETPLAYERSANCTIONCOUNT_API_LATEST;
							CountOptions.TargetUserId = Data->TargetUserId;

							uint32_t SanctionCount = EOS_Sanctions_GetPlayerSanctionCount(SanctionsHandle, &CountOptions);
							
							TArray<FEOSSanctionInfo> Sanctions;
							Sanctions.Reserve(SanctionCount);

							// Get each sanction
							for (uint32_t i = 0; i < SanctionCount; i++)
							{
								EOS_Sanctions_CopyPlayerSanctionByIndexOptions CopyOptions = {};
								CopyOptions.ApiVersion = EOS_SANCTIONS_COPYPLAYERSANCTIONBYINDEX_API_LATEST;
								CopyOptions.TargetUserId = Data->TargetUserId;
								CopyOptions.SanctionIndex = i;

								EOS_Sanctions_PlayerSanction* Sanction = nullptr;
								EOS_EResult CopyResult = EOS_Sanctions_CopyPlayerSanctionByIndex(SanctionsHandle, &CopyOptions, &Sanction);

								if (CopyResult == EOS_EResult::EOS_Success && Sanction)
								{
									FEOSSanctionInfo SanctionInfo;
									
									// Only access fields available in SDK 1.18
									if (Sanction->ReferenceId)
									{
										SanctionInfo.ReferenceId = UTF8_TO_TCHAR(Sanction->ReferenceId);
									}
									
									if (Sanction->Action)
									{
										SanctionInfo.Action = UTF8_TO_TCHAR(Sanction->Action);
									}
									
									SanctionInfo.TimePlaced = FDateTime::FromUnixTimestamp(Sanction->TimePlaced);
									SanctionInfo.TimeExpires = FDateTime::FromUnixTimestamp(Sanction->TimeExpires);

									// Check if it's a permanent ban (time expires is very far in future or -1)
									SanctionInfo.bIsPermanent = (Sanction->TimeExpires < 0);

									Sanctions.Add(SanctionInfo);

									// Release the sanction
									EOS_Sanctions_PlayerSanction_Release(Sanction);
								}
							}

							UE_LOG(LogTemp, Log, TEXT("EOSKitWeb: Query sanctions successful. Found %d sanctions"), Sanctions.Num());
							Self->OnSuccess.Broadcast(Sanctions);
							Self->SetReadyToDestroy();
							return;
						}
					}
				}

				Self->OnFail.Broadcast(TEXT("Failed to retrieve sanctions"));
				Self->SetReadyToDestroy();
			}
			else
			{
				FString ErrorMessage = FString::Printf(TEXT("Query sanctions failed: %s"), 
					UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
				UE_LOG(LogTemp, Error, TEXT("EOSKitWeb: %s"), *ErrorMessage);
				Self->OnFail.Broadcast(ErrorMessage);
				Self->SetReadyToDestroy();
			}
		});
	});
}
