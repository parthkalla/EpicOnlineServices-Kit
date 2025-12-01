// Copyright (C) 2024, All Rights Reserved.

#include "EOSGetFriendListAsync.h"
#include "EOSKitSubsystem.h"
#include "EOSKitGameInstanceSubsystem.h"
#include "EOSKitFriendsSubsystem.h"
#include "EOSKitSharedTypes.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "eos_sdk.h"
#include "eos_friends.h"
#include "eos_friends_types.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UEOSGetFriendListAsync* UEOSGetFriendListAsync::GetFriendList(UObject* WorldContextObject)
{
	UEOSGetFriendListAsync* Node = NewObject<UEOSGetFriendListAsync>();
	Node->WorldContextObject = WorldContextObject;
	return Node;
}

void UEOSGetFriendListAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(TArray<FEOSKitFriendData>(), TEXT("Invalid World Context"));
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(TArray<FEOSKitFriendData>(), TEXT("Invalid Game Instance"));
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(TArray<FEOSKitFriendData>(), TEXT("EOS Platform not initialized"));
		SetReadyToDestroy();
		return;
	}

	// Get Epic Account ID from subsystem
	FString EpicAccountIdString = UEOSKitGameInstanceSubsystem::GetEpicAccountId(0);
	if (EpicAccountIdString.IsEmpty())
	{
		OnFailure.Broadcast(TArray<FEOSKitFriendData>(), TEXT("User not logged in"));
		SetReadyToDestroy();
		return;
	}

	EOS_HFriends FriendsHandle = EOS_Platform_GetFriendsInterface(EOSSubsystem->GetPlatformHandle());
	if (!FriendsHandle)
	{
		OnFailure.Broadcast(TArray<FEOSKitFriendData>(), TEXT("Failed to get Friends Interface"));
		SetReadyToDestroy();
		return;
	}

	// Convert Epic Account ID
	EOS_EpicAccountId LocalUserId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*EpicAccountIdString));
	if (!EOS_EpicAccountId_IsValid(LocalUserId))
	{
		OnFailure.Broadcast(TArray<FEOSKitFriendData>(), TEXT("Invalid Epic Account ID"));
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_Friends_QueryFriendsOptions Options = {};
	Options.ApiVersion = EOS_FRIENDS_QUERYFRIENDS_API_LATEST;
	Options.LocalUserId = LocalUserId;

	// Query friends
	EOS_Friends_QueryFriends(FriendsHandle, &Options, this, &UEOSGetFriendListAsync::OnQueryFriendsComplete);
}

void EOS_CALL UEOSGetFriendListAsync::OnQueryFriendsComplete(const EOS_Friends_QueryFriendsCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSGetFriendListAsync* Self = static_cast<UEOSGetFriendListAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Get friends list
		TArray<FEOSKitFriendData> FriendList;

		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject))
		{
			UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
			if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
			{
				EOS_HFriends FriendsHandle = EOS_Platform_GetFriendsInterface(EOSSubsystem->GetPlatformHandle());
				if (FriendsHandle)
				{
					// Get friends count
					EOS_Friends_GetFriendsCountOptions CountOptions = {};
					CountOptions.ApiVersion = EOS_FRIENDS_GETFRIENDSCOUNT_API_LATEST;
					CountOptions.LocalUserId = Data->LocalUserId;
					int32 FriendsCount = EOS_Friends_GetFriendsCount(FriendsHandle, &CountOptions);

					// Get each friend
					for (int32 i = 0; i < FriendsCount; i++)
					{
						EOS_Friends_GetFriendAtIndexOptions IndexOptions = {};
						IndexOptions.ApiVersion = EOS_FRIENDS_GETFRIENDATINDEX_API_LATEST;
						IndexOptions.LocalUserId = Data->LocalUserId;
						IndexOptions.Index = i;

						EOS_EpicAccountId FriendId = EOS_Friends_GetFriendAtIndex(FriendsHandle, &IndexOptions);
						if (EOS_EpicAccountId_IsValid(FriendId))
						{
							FEOSKitFriendData FriendData;
							FriendData.EpicAccountId = FEOSKitEpicAccountId(FriendId);

							// Get friendship status
							EOS_Friends_GetStatusOptions StatusOptions = {};
							StatusOptions.ApiVersion = EOS_FRIENDS_GETSTATUS_API_LATEST;
							StatusOptions.LocalUserId = Data->LocalUserId;
							StatusOptions.TargetUserId = FriendId;
							EOS_EFriendsStatus Status = EOS_Friends_GetStatus(FriendsHandle, &StatusOptions);
							FriendData.InviteStatus = static_cast<EEOSKitFriendsStatus>(Status);

							// TODO: Get display name and presence from UserInfo/Presence interfaces
							// For now, just use the Epic Account ID as display name
							char AccountIdStr[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
							int32_t BufferSize = sizeof(AccountIdStr);
							if (EOS_EpicAccountId_ToString(FriendId, AccountIdStr, &BufferSize) == EOS_EResult::EOS_Success)
							{
								FriendData.DisplayName = UTF8_TO_TCHAR(AccountIdStr);
							}

							FriendList.Add(FriendData);
						}
					}
				}
			}
		}

		Self->OnSuccess.Broadcast(FriendList, TEXT(""));
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("Query Friends Failed: %s"), 
			UTF8_TO_TCHAR(EOS_EResult_ToString(Data->ResultCode)));
		Self->OnFailure.Broadcast(TArray<FEOSKitFriendData>(), ErrorMessage);
	}

	Self->SetReadyToDestroy();
}

