// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_SanctionsSubsystem.h"

#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

TEnumAsByte<EEOK_Result> UEOK_SanctionsSubsystem::EOK_Sanctions_CopyPlayerSanctionByIndex(
	FEOK_ProductUserId LocalUserId, int32 Index, FEOK_Sanctions_PlayerSanction& OutSanction)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sanctions_CopyPlayerSanctionByIndexOptions Options = { };
			Options.ApiVersion = EOS_SANCTIONS_COPYPLAYERSANCTIONBYINDEX_API_LATEST;
			Options.SanctionIndex = Index;
			Options.TargetUserId = LocalUserId.GetValueAsEosType();
			EOS_Sanctions_PlayerSanction* OutSanctionPtr;
			auto Result = EOS_Sanctions_CopyPlayerSanctionByIndex(EOSRef->SanctionsHandle, &Options, &OutSanctionPtr);
			if (Result == EOS_EResult::EOS_Success)
			{
				OutSanction = *OutSanctionPtr;
				EOS_Sanctions_PlayerSanction_Release(OutSanctionPtr);
			}
			return static_cast<EEOK_Result>(Result);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Sanctions_CopyPlayerSanctionByIndex: Failed to get EOS Platform Handle"));
	return EEOK_Result::EOS_ServiceFailure;
}

int32 UEOK_SanctionsSubsystem::EOK_Sanctions_GetPlayerSanctionCount(FEOK_ProductUserId LocalUserId)
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sanctions_GetPlayerSanctionCountOptions Options = { };
			Options.ApiVersion = EOS_SANCTIONS_GETPLAYERSANCTIONCOUNT_API_LATEST;
			Options.TargetUserId = LocalUserId.GetValueAsEosType();
			return EOS_Sanctions_GetPlayerSanctionCount(EOSRef->SanctionsHandle, &Options);
		}
	}
	UE_LOG(LogEOK, Error, TEXT("EOK_Sanctions_GetPlayerSanctionCount: Failed to get EOS Platform Handle"));
	return -1;
}
