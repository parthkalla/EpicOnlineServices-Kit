// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_JoinSession_AsyncFunction.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"

UEOK_JoinSession_AsyncFunction* UEOK_JoinSession_AsyncFunction::JoinEOKSessions(UObject* WorldContextObject, FName SessionName, FSessionFindStruct SessionToJoin,bool bLanSession)
{
	UEOK_JoinSession_AsyncFunction* UEOK_JoinSessionObject = NewObject<UEOK_JoinSession_AsyncFunction>();
	UEOK_JoinSessionObject->Var_SessionToJoin = SessionToJoin;
	UEOK_JoinSessionObject->Var_SessionName = SessionName;
	UEOK_JoinSessionObject->Var_WorldContextObject = WorldContextObject;
	return UEOK_JoinSessionObject;
}

void UEOK_JoinSession_AsyncFunction::Activate()
{
	JoinSession();
	Super::Activate();
}

void UEOK_JoinSession_AsyncFunction::JoinSession()
{
	if(const IOnlineSubsystem *SubsystemRef = Online::GetSubsystem(this->GetWorld(), "EOK"))
	{
		if(const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
 			SessionPtrRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOK_JoinSession_AsyncFunction::OnJoinSessionCompleted);
			SessionPtrRef->JoinSession(0, Var_SessionName, Var_SessionToJoin.SessionResult.OnlineResult);
		}
		else
		{
			if(bDelegateCalled)
			{
				return;
			}
			OnFail.Broadcast(EEOKJoinResult::UnknownError, FString());
			bDelegateCalled = true;
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
	else
	{
		if(bDelegateCalled)
		{
			return;
		}
		OnFail.Broadcast(EEOKJoinResult::UnknownError, FString());
		bDelegateCalled = true;
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}
}
void UEOK_JoinSession_AsyncFunction::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (bDelegateCalled)
	{
		return;
	}
	if(Var_SessionToJoin.SessionSettings.Contains("IsPartySession") && Var_SessionToJoin.SessionSettings["IsPartySession"].BoolValue)
	{
		UE_LOG(LogEOK, Log, TEXT("EOK: Successfully joined party session"));
		OnSuccess.Broadcast(EEOKJoinResult::Success, "");
		bDelegateCalled = true;
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(Var_WorldContextObject, 0))
		{
			if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
			{
				const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
				if (SessionPtrRef.IsValid())
				{
					FString JoinAddress;
					SessionPtrRef->GetResolvedConnectString(SessionName, JoinAddress);
					if (Var_SessionToJoin.bIsDedicatedServer)
					{
						FString PortInfo = "7777";
						if (Var_SessionToJoin.SessionSettings.Contains("PortInfo"))
						{
							Var_SessionToJoin.SessionResult.OnlineResult.Session.SessionSettings.Get("PortInfo", PortInfo);
						}
						TArray<FString> IpPortArray;
						JoinAddress.ParseIntoArray(IpPortArray, TEXT(":"), true);
						const FString IpAddress = IpPortArray[0];
						JoinAddress = IpAddress + ":" + PortInfo;
					}

					if (!JoinAddress.IsEmpty())
					{
						PlayerControllerRef->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
						OnSuccess.Broadcast(EEOKJoinResult::Success, JoinAddress);
						bDelegateCalled = true;
						SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						MarkAsGarbage();
#else
						MarkPendingKill();
#endif
						return;
					}
					else
					{
						UE_LOG(LogEOK, Warning, TEXT("EOK: Could not retrieve address"));
						OnFail.Broadcast(EEOKJoinResult::CouldNotRetrieveAddress, FString());
						bDelegateCalled = true;
						SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						MarkAsGarbage();
#else
						MarkPendingKill();
#endif
						return;
					}
				}
			}
		}
		else
		{
			OnFail.Broadcast(EEOKJoinResult::UnknownError, FString());
			bDelegateCalled = true;
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
			return;
		}
	}
	else
	{
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			OnFail.Broadcast(EEOKJoinResult::SessionIsFull, FString());
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			OnFail.Broadcast(EEOKJoinResult::SessionDoesNotExist, FString());
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			OnFail.Broadcast(EEOKJoinResult::CouldNotRetrieveAddress, FString());
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			OnFail.Broadcast(EEOKJoinResult::AlreadyInSession, FString());
			break;
		default:
			OnFail.Broadcast(EEOKJoinResult::UnknownError, FString());
		}
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		bDelegateCalled = true;
	}
}

