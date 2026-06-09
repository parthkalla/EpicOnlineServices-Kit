// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Ecom_RedeemEntitlements.h"

#include "OnlineSubsystemEOS.h"
#include "Async/Async.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_Ecom_RedeemEntitlements* UEOK_Ecom_RedeemEntitlements::EOK_Ecom_RedeemEntitlements(
	const FEOK_EpicAccountId& LocalUserId, const TArray<FEOK_Ecom_EntitlementId>& EntitlementIds)
{
	UEOK_Ecom_RedeemEntitlements* Node = NewObject<UEOK_Ecom_RedeemEntitlements>();
	Node->Var_LocalUserId = LocalUserId;
	Node->Var_EntitlementIds = EntitlementIds;
	return Node;
}

void UEOK_Ecom_RedeemEntitlements::OnRedeemEntitlementsCallback(const EOS_Ecom_RedeemEntitlementsCallbackInfo* Data)
{
	if (UEOK_Ecom_RedeemEntitlements* Node = static_cast<UEOK_Ecom_RedeemEntitlements*>(Data->ClientData))
	{
		AsyncTask(ENamedThreads::GameThread, [Node, Data]()
		{
			Node->OnCallback.Broadcast(Data->LocalUserId, static_cast<EEOK_Result>(Data->ResultCode), Data->RedeemedEntitlementIdsCount);
		});
		Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		Node->MarkAsGarbage();
#else
		Node->MarkPendingKill();
#endif
	}
}

void UEOK_Ecom_RedeemEntitlements::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_Ecom_RedeemEntitlementsOptions RedeemEntitlementsOptions;
			RedeemEntitlementsOptions.ApiVersion = EOS_ECOM_REDEEMENTITLEMENTS_API_LATEST;
			RedeemEntitlementsOptions.LocalUserId = Var_LocalUserId.GetValueAsEosType();
			RedeemEntitlementsOptions.EntitlementIdCount = Var_EntitlementIds.Num();
			RedeemEntitlementsOptions.EntitlementIds = new EOS_Ecom_EntitlementId[Var_EntitlementIds.Num()];
			for (int32 i = 0; i < Var_EntitlementIds.Num(); i++)
			{
				RedeemEntitlementsOptions.EntitlementIds[i] = Var_EntitlementIds[i].Ref;
			}
			EOS_Ecom_RedeemEntitlements(EOSRef->EcomHandle, &RedeemEntitlementsOptions, this, &UEOK_Ecom_RedeemEntitlements::OnRedeemEntitlementsCallback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to redeem entitlements either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(FEOK_EpicAccountId(), EEOK_Result::EOS_Disabled, 0);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
