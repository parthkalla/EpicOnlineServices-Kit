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
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface())
		{
			SessionPtrRef->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSJoinLobbyAsync::OnJoinSessionCompleted)
			);

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


