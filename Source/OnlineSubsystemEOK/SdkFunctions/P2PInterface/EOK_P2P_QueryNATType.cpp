// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_P2P_QueryNATType.h"

#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_P2P_QueryNATType* UEOK_P2P_QueryNATType::EOK_P2P_QueryNATType()
{
	UEOK_P2P_QueryNATType* BlueprintNode = NewObject<UEOK_P2P_QueryNATType>();
	return BlueprintNode;
}

void UEOK_P2P_QueryNATType::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			EOS_P2P_QueryNATTypeOptions Options = {};
			Options.ApiVersion = EOS_P2P_QUERYNATTYPE_API_LATEST;
			EOS_P2P_QueryNATType(EOSRef->SocketSubsystem->GetP2PHandle(), &Options, this, &UEOK_P2P_QueryNATType::EOS_P2P_QueryNATType_Callback);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("UEOK_P2P_QueryNATType::Activate: Unable to get EOS subsystem."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, EEOK_ENATType::EOK_NAT_Unknown);
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOK_P2P_QueryNATType::EOS_P2P_QueryNATType_Callback(const EOS_P2P_OnQueryNATTypeCompleteInfo* Data)
{
	if(UEOK_P2P_QueryNATType* ThisNode = static_cast<UEOK_P2P_QueryNATType*>(Data->ClientData))
	{
		ThisNode->OnCallback.Broadcast(static_cast<EEOK_Result>(Data->ResultCode), static_cast<EEOK_ENATType>(Data->NATType));
		ThisNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		ThisNode->MarkAsGarbage();
#else
		ThisNode->MarkPendingKill();
#endif
	}
}
