// Copyright (C) 2024, All Rights Reserved.

#include "EOSFindUserByDisplayNameAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "eos_userinfo.h"
#include "eos_userinfo_types.h"

UEOSFindUserByDisplayNameAsync* UEOSFindUserByDisplayNameAsync::FindUserByDisplayName(UObject* WorldContextObject, const FString& TargetDisplayName)
{
	UEOSFindUserByDisplayNameAsync* Node = NewObject<UEOSFindUserByDisplayNameAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->TargetDisplayNameString = TargetDisplayName;
	return Node;
}

void UEOSFindUserByDisplayNameAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnFailure.Broadcast(FEOSKitUserInfo());
		SetReadyToDestroy();
		return;
	}

	if (TargetDisplayNameString.IsEmpty())
	{
		OnFailure.Broadcast(FEOSKitUserInfo());
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		OnFailure.Broadcast(FEOSKitUserInfo());
		SetReadyToDestroy();
		return;
	}

	UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSSubsystem || !EOSSubsystem->GetPlatformHandle())
	{
		OnFailure.Broadcast(FEOSKitUserInfo());
		SetReadyToDestroy();
		return;
	}

	// Get Epic Account ID from subsystem
	FString EpicAccountIdString = EOSSubsystem->GetEpicAccountIdString();
	if (EpicAccountIdString.IsEmpty())
	{
		OnFailure.Broadcast(FEOSKitUserInfo());
		SetReadyToDestroy();
		return;
	}

	EOS_HUserInfo UserInfoHandle = EOS_Platform_GetUserInfoInterface(EOSSubsystem->GetPlatformHandle());
	if (!UserInfoHandle)
	{
		OnFailure.Broadcast(FEOSKitUserInfo());
		SetReadyToDestroy();
		return;
	}

	// Convert Epic Account ID
	EOS_EpicAccountId LocalUserId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*EpicAccountIdString));
	if (!EOS_EpicAccountId_IsValid(LocalUserId))
	{
		OnFailure.Broadcast(FEOSKitUserInfo());
		SetReadyToDestroy();
		return;
	}

	// Setup options
	EOS_UserInfo_QueryUserInfoByDisplayNameOptions Options = {};
	Options.ApiVersion = EOS_USERINFO_QUERYUSERINFOBYDISPLAYNAME_API_LATEST;
	Options.DisplayName = TCHAR_TO_UTF8(*TargetDisplayNameString);
	Options.LocalUserId = LocalUserId;

	// Query user info by display name
	EOS_UserInfo_QueryUserInfoByDisplayName(UserInfoHandle, &Options, this, &UEOSFindUserByDisplayNameAsync::OnQueryUserInfoByDisplayNameComplete);
}

void EOS_CALL UEOSFindUserByDisplayNameAsync::OnQueryUserInfoByDisplayNameComplete(const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	UEOSFindUserByDisplayNameAsync* Self = static_cast<UEOSFindUserByDisplayNameAsync*>(Data->ClientData);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Copy user info
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(Self->WorldContextObject))
		{
			UEOSKitSubsystem* EOSSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
			if (EOSSubsystem && EOSSubsystem->GetPlatformHandle())
			{
				EOS_HUserInfo UserInfoHandle = EOS_Platform_GetUserInfoInterface(EOSSubsystem->GetPlatformHandle());
				if (UserInfoHandle)
				{
					EOS_UserInfo_CopyUserInfoOptions CopyOptions = {};
					CopyOptions.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
					CopyOptions.LocalUserId = Data->LocalUserId;
					CopyOptions.TargetUserId = Data->TargetUserId;

					EOS_UserInfo* UserInfo = nullptr;
					EOS_EResult CopyResult = EOS_UserInfo_CopyUserInfo(UserInfoHandle, &CopyOptions, &UserInfo);

					if (CopyResult == EOS_EResult::EOS_Success && UserInfo)
					{
						FEOSKitUserInfo UserInfoStruct;

						// Convert Epic Account ID to string
						char AccountIdBuffer[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
						int32_t BufferSize = sizeof(AccountIdBuffer);
						if (EOS_EpicAccountId_ToString(UserInfo->UserId, AccountIdBuffer, &BufferSize) == EOS_EResult::EOS_Success)
						{
							UserInfoStruct.EpicAccountId = UTF8_TO_TCHAR(AccountIdBuffer);
						}

						UserInfoStruct.Country = UserInfo->Country ? UTF8_TO_TCHAR(UserInfo->Country) : TEXT("");
						UserInfoStruct.DisplayName = UserInfo->DisplayName ? UTF8_TO_TCHAR(UserInfo->DisplayName) : TEXT("");
						UserInfoStruct.PreferredLanguage = UserInfo->PreferredLanguage ? UTF8_TO_TCHAR(UserInfo->PreferredLanguage) : TEXT("");
						UserInfoStruct.Nickname = UserInfo->Nickname ? UTF8_TO_TCHAR(UserInfo->Nickname) : TEXT("");
						UserInfoStruct.DisplayNameSanitized = UserInfo->DisplayNameSanitized ? UTF8_TO_TCHAR(UserInfo->DisplayNameSanitized) : TEXT("");

						EOS_UserInfo_Release(UserInfo);

						// Validate Epic Account ID
						if (!UserInfoStruct.EpicAccountId.IsEmpty() && 
							EOS_EpicAccountId_IsValid(EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*UserInfoStruct.EpicAccountId))))
						{
							Self->OnSuccess.Broadcast(UserInfoStruct);
						}
						else
						{
							Self->OnFailure.Broadcast(FEOSKitUserInfo());
						}
					}
					else
					{
						Self->OnFailure.Broadcast(FEOSKitUserInfo());
					}
				}
				else
				{
					Self->OnFailure.Broadcast(FEOSKitUserInfo());
				}
			}
			else
			{
				Self->OnFailure.Broadcast(FEOSKitUserInfo());
			}
		}
		else
		{
			Self->OnFailure.Broadcast(FEOSKitUserInfo());
		}
	}
	else
	{
		Self->OnFailure.Broadcast(FEOSKitUserInfo());
	}

	Self->SetReadyToDestroy();
}

