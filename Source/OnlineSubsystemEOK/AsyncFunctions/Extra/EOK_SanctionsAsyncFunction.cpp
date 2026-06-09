// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_SanctionsAsyncFunction.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"

UEOK_SanctionsAsyncFunction* UEOK_SanctionsAsyncFunction::GetEOKPlayerSanctionsAsyncFunction(FString LocalProductUserID, FString TargetProductUserID)
{
	UEOK_SanctionsAsyncFunction* UEOK_GetSanctionObject = NewObject<UEOK_SanctionsAsyncFunction>();
	UEOK_GetSanctionObject->Var_LocalProductUserID = LocalProductUserID;
	UEOK_GetSanctionObject->Var_TargetProductUserID = TargetProductUserID;
	return UEOK_GetSanctionObject;
}

void UEOK_SanctionsAsyncFunction::Func_GetSanctions()
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Sanctions_QueryActivePlayerSanctionsOptions SanctionsOptions;
			SanctionsOptions.ApiVersion = EOS_SANCTIONS_QUERYACTIVEPLAYERSANCTIONS_API_LATEST;
			SanctionsOptions.LocalUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Var_LocalProductUserID));
			SanctionsOptions.TargetUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Var_TargetProductUserID));
			EOS_Sanctions_QueryActivePlayerSanctions(EOSRef->SanctionsHandle,&SanctionsOptions,this,ReturnFunc);
			return;
		}
	}
	FireFailure();
}

void UEOK_SanctionsAsyncFunction::ReturnFunc(const EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo* Data)
{
	if(Data->ClientData)
	{
		if(UEOK_SanctionsAsyncFunction* SanctionsFunction = static_cast<UEOK_SanctionsAsyncFunction*>(Data->ClientData))
		{
			if(Data->ResultCode == EOS_EResult::EOS_Success)
			{
				SanctionsFunction->GetFinalValues();
			}
			else
			{
				SanctionsFunction->FireFailure();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MySanctionsFunction is null. No callback will be fired"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClientData is null. No callback will be fired"));
	}
}

void UEOK_SanctionsAsyncFunction::GetFinalValues()
{
	if(	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			TArray<FEOK_Sanctions_PlayerSanction> SanctionsArray;
			EOS_Sanctions_GetPlayerSanctionCountOptions SanctionsCountOptions;
			SanctionsCountOptions.ApiVersion = EOS_SANCTIONS_GETPLAYERSANCTIONCOUNT_API_LATEST;
			SanctionsCountOptions.TargetUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Var_TargetProductUserID));
			uint32_t SanctionsCount = EOS_Sanctions_GetPlayerSanctionCount(EOSRef->SanctionsHandle,&SanctionsCountOptions);
			if(SanctionsCount <= 0)
			{
				Success.Broadcast(SanctionsArray);
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				return;
			}
			EOS_Sanctions_CopyPlayerSanctionByIndexOptions SanctionsCopyOptions;
			EOS_Sanctions_PlayerSanction ** OutSanction = new EOS_Sanctions_PlayerSanction*[SanctionsCount];
			SanctionsCopyOptions.ApiVersion = EOS_SANCTIONS_COPYPLAYERSANCTIONBYINDEX_API_LATEST;
			SanctionsCopyOptions.TargetUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*Var_TargetProductUserID));
			EOS_Sanctions_CopyPlayerSanctionByIndex(EOSRef->SanctionsHandle,&SanctionsCopyOptions,OutSanction);
			if(OutSanction)
			{
				for(int32 i =0; i < (int32)SanctionsCount;i++)
				{
					if (OutSanction[i])
					{
						FEOK_Sanctions_PlayerSanction LocalTempRef = *OutSanction[i];
						SanctionsArray.Add(LocalTempRef);
					}
				}
				for(int32 i=0; i < (int32)SanctionsCount;i++)
				{
					if(OutSanction[i])
						EOS_Sanctions_PlayerSanction_Release(OutSanction[i]);
				}
			}
			else
			{
				FireFailure();
				return;
			}
			Success.Broadcast(SanctionsArray);
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
		else
		{
			FireFailure();
		}
	}
	else
	{
		FireFailure();
	}
}

void UEOK_SanctionsAsyncFunction::FireFailure()
{
	Failure.Broadcast(TArray<FEOK_Sanctions_PlayerSanction>());
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_SanctionsAsyncFunction::Activate()
{
	Func_GetSanctions();
	Super::Activate();
}
