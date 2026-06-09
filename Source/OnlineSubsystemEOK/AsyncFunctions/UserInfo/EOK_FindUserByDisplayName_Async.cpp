// Copyright Epic Games, Inc. All Rights Reserved.

#include "EOK_FindUserByDisplayName_Async.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"

UEOK_FindUserByDisplayName_Async* UEOK_FindUserByDisplayName_Async::FindEOKUserByDisplayName(FString TargetDisplayName, FString LocalEpicID)
{
    UEOK_FindUserByDisplayName_Async* UEOK_FindUserByDisplayName_Object = NewObject < UEOK_FindUserByDisplayName_Async>();

    UEOK_FindUserByDisplayName_Object->TargetDisplayName = TargetDisplayName;
    UEOK_FindUserByDisplayName_Object->LocalEpicID = LocalEpicID;

	return UEOK_FindUserByDisplayName_Object;
}

void UEOK_FindUserByDisplayName_Async::FindUserByDisplayName()
{
    if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
    {
        if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
        {
            if (EOSRef->ReportsHandle != nullptr)
            {
                if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
                {
                    if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
                    {
                        EOS_UserInfo_QueryUserInfoByDisplayNameOptions QueryOptions;
                        QueryOptions.ApiVersion = EOS_USERINFO_QUERYUSERINFOBYDISPLAYNAME_API_LATEST;
                        QueryOptions.DisplayName = TCHAR_TO_UTF8(*TargetDisplayName);
                        QueryOptions.LocalUserId = EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*LocalEpicID));

                        EOS_UserInfo_QueryUserInfoByDisplayName(EOSRef->UserInfoHandle, &QueryOptions, this, FindUserByDisplayNameCallback);
                    }
                    else
                    {
                        ResultFaliure();
                    }
                }
                else
                {
                    ResultFaliure();
                }
            }
            else
            {
                ResultFaliure();
            }
        }
        else
        {
            ResultFaliure();
        }
    }
    else
    {
        ResultFaliure();
    }

}

void UEOK_FindUserByDisplayName_Async::FindUserByDisplayNameCallback(const EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo* Data)
{
    if (UEOK_FindUserByDisplayName_Async* FindEOKUserByDisplayName = static_cast<UEOK_FindUserByDisplayName_Async*>(Data->ClientData))
    {
        if (Data->ResultCode == EOS_EResult::EOS_Success)
        {
            if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
            {
                if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
                {
                    if (EOSRef->UserInfoHandle != nullptr)
                    {
                        if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
                        {
                            EOS_UserInfo_CopyUserInfoOptions CopyUserInfoObj;
                            CopyUserInfoObj.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
                            CopyUserInfoObj.LocalUserId = Data->LocalUserId;
                            CopyUserInfoObj.TargetUserId = Data->TargetUserId;

                            EOS_UserInfo* UserInfo;

                            if (EOS_UserInfo_CopyUserInfo(EOSRef->UserInfoHandle, &CopyUserInfoObj, &UserInfo) == EOS_EResult::EOS_Success)
                            {
                                FEOKUserInfo UserInfoStruct;

                                UserInfoStruct.Country = UserInfo->Country;
                                UserInfoStruct.DisplayName = UserInfo->DisplayName;
                                UserInfoStruct.DisplayNameSanitized = UserInfo->DisplayNameSanitized;
                                UserInfoStruct.Nickname = UserInfo->Nickname;
                                char Buffer[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
                                int32_t BufferLength = EOS_EPICACCOUNTID_MAX_LENGTH + 1;



                                if (EOS_EpicAccountId_ToString(UserInfo->UserId, Buffer, &BufferLength) == EOS_EResult::EOS_Success)
                                {
                                    UserInfoStruct.EpicAccountID = FString(UTF8_TO_TCHAR(Buffer));
                                    FindEOKUserByDisplayName->ResultSuccess(UserInfoStruct);
                                }
                                else
                                {
                                    FindEOKUserByDisplayName->ResultFaliure();
                                }
                            }
                            else
                            {
                                FindEOKUserByDisplayName->ResultFaliure();
                            }
                        }
                        else
                        {
                            FindEOKUserByDisplayName->ResultFaliure();
                        }
                    }
                    else
                    {
                        FindEOKUserByDisplayName->ResultFaliure();
                    }
                }
                else
                {
                    FindEOKUserByDisplayName->ResultFaliure();
                }
            }
            else
            {
                FindEOKUserByDisplayName->ResultFaliure();
            }

        }
        else
        {
            FindEOKUserByDisplayName->ResultFaliure();
        }
    }
}

// Function to handle failure cases
void UEOK_FindUserByDisplayName_Async::ResultFaliure()
{
    Failure.Broadcast(FEOKUserInfo());
    SetReadyToDestroy();
}

// Function to handle success cases
void UEOK_FindUserByDisplayName_Async::ResultSuccess(const FEOKUserInfo UserInfoStruct)
{
    if (EOS_EpicAccountId_IsValid(EOS_EpicAccountId_FromString(TCHAR_TO_UTF8(*UserInfoStruct.EpicAccountID))))
    {
        Success.Broadcast(UserInfoStruct);
        SetReadyToDestroy();
    }
    else
    {
        ResultFaliure();
    }
}

// Function to activate the asynchronous function
void UEOK_FindUserByDisplayName_Async::Activate()
{
    FindUserByDisplayName();
    Super::Activate();
}

