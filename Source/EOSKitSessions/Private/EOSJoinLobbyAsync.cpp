// Restored from original EOSKitLobbies module
#include "EOSJoinLobbyAsync.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"

UEOSJoinLobbyAsync* UEOSJoinLobbyAsync::JoinEOSKitSession(
	UObject* WorldContextObject,
	FName SessionName,
	FBlueprintSessionResult SessionToJoin,
	bool bLanSession)
{
	UEOSJoinLobbyAsync* JoinSessionObject = NewObject<UEOSJoinLobbyAsync>();
	JoinSessionObject->Var_SessionToJoin = SessionToJoin;
	JoinSessionObject->Var_SessionName = SessionName;
	JoinSessionObject->Var_WorldContextObject = WorldContextObject;
	return JoinSessionObject;
}

void UEOSJoinLobbyAsync::Activate()
{
	JoinSession();
	Super::Activate();
}

void UEOSJoinLobbyAsync::JoinSession()
{
	// Use Online::GetSubsystem like EIK does (with WorldContextObject)
	if (const IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(Var_WorldContextObject->GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			// Use OnJoinSessionCompleteDelegates like EIK (not AddOnJoinSessionCompleteDelegate_Handle)
			SessionPtrRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSJoinLobbyAsync::OnJoinSessionCompleted);
			
			// Join using SessionResult.OnlineResult like EIK
			SessionPtrRef->JoinSession(0, Var_SessionName, Var_SessionToJoin.OnlineResult);
		}
		else
		{
			if (bDelegateCalled)
			{
				return;
			}
			Failure.Broadcast(EEOSKitJoinResult::UnknownError, FString());
			bDelegateCalled = true;
			SetReadyToDestroy();
			MarkAsGarbage();
		}
	}
	else
	{
		if (bDelegateCalled)
		{
			return;
		}
		Failure.Broadcast(EEOSKitJoinResult::UnknownError, FString());
		bDelegateCalled = true;
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

void UEOSJoinLobbyAsync::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	if (bDelegateCalled)
	{
		return;
	}
	
	// Check for party session like EIK does (access from OnlineResult.Session.SessionSettings)
	bool bIsPartySession = false;
	if (Var_SessionToJoin.OnlineResult.Session.SessionSettings.Get(FName(TEXT("IsPartySession")), bIsPartySession) && bIsPartySession)
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Successfully joined party session"));
		Success.Broadcast(EEOSKitJoinResult::Success, FString());
		Result.Broadcast(EEOSKitJoinResult::Success, FString());
		bDelegateCalled = true;
		SetReadyToDestroy();
		MarkAsGarbage();
		return;
	}
	
	if (JoinResult == EOnJoinSessionCompleteResult::Success)
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
					
					// Handle dedicated server port info like EIK
					bool bIsDedicatedServer = false;
					Var_SessionToJoin.OnlineResult.Session.SessionSettings.Get(FName(TEXT("IsDedicatedServer")), bIsDedicatedServer);
					if (bIsDedicatedServer)
					{
						FString PortInfo = TEXT("7777");
						Var_SessionToJoin.OnlineResult.Session.SessionSettings.Get(FName(TEXT("PortInfo")), PortInfo);
						TArray<FString> IpPortArray;
						JoinAddress.ParseIntoArray(IpPortArray, TEXT(":"), true);
						if (IpPortArray.Num() > 0)
						{
							const FString IpAddress = IpPortArray[0];
							JoinAddress = IpAddress + TEXT(":") + PortInfo;
						}
					}

					if (!JoinAddress.IsEmpty())
					{
						PlayerControllerRef->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
						Success.Broadcast(EEOSKitJoinResult::Success, JoinAddress);
						Result.Broadcast(EEOSKitJoinResult::Success, JoinAddress);
						bDelegateCalled = true;
						SetReadyToDestroy();
						MarkAsGarbage();
						return;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("EOSKit: Could not retrieve address"));
						Failure.Broadcast(EEOSKitJoinResult::CouldNotRetrieveAddress, FString());
						Result.Broadcast(EEOSKitJoinResult::CouldNotRetrieveAddress, FString());
						bDelegateCalled = true;
						SetReadyToDestroy();
						MarkAsGarbage();
						return;
					}
				}
			}
		}
		else
		{
			Failure.Broadcast(EEOSKitJoinResult::UnknownError, FString());
			Result.Broadcast(EEOSKitJoinResult::UnknownError, FString());
			bDelegateCalled = true;
			SetReadyToDestroy();
			MarkAsGarbage();
			return;
		}
	}
	else
	{
		EEOSKitJoinResult ResultType;
		switch (JoinResult)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ResultType = EEOSKitJoinResult::SessionIsFull;
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ResultType = EEOSKitJoinResult::SessionDoesNotExist;
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			ResultType = EEOSKitJoinResult::CouldNotRetrieveAddress;
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			ResultType = EEOSKitJoinResult::AlreadyInSession;
			break;
		default:
			ResultType = EEOSKitJoinResult::UnknownError;
		}

		Failure.Broadcast(ResultType, FString());
		Result.Broadcast(ResultType, FString());
		SetReadyToDestroy();
		MarkAsGarbage();
		bDelegateCalled = true;
	}
}


