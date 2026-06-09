// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_SetPlayerData_AsyncFunction.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Kismet/GameplayStatics.h"

UEOK_SetPlayerData_AsyncFunction* UEOK_SetPlayerData_AsyncFunction::SetPlayerData(FString FileName, const TArray<uint8>& DataToSave)
{
	UEOK_SetPlayerData_AsyncFunction* BlueprintNode = NewObject<UEOK_SetPlayerData_AsyncFunction>();
	BlueprintNode->FileName = FileName;
	BlueprintNode->DataToSave = DataToSave;
	return BlueprintNode;
}

void UEOK_SetPlayerData_AsyncFunction::Activate()
{
	SetPlayerData();
	Super::Activate();
}

void UEOK_SetPlayerData_AsyncFunction::SetPlayerData()
{
	if(DataToSave.Num() > 0)
	{
		if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get("EOK"))
		{
			if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				if(const IOnlineUserCloudPtr CloudPointerRef = SubsystemRef->GetUserCloudInterface())
				{
					if(!IdentityPointerRef->GetUniquePlayerId(0).IsValid())
					{
						if(!bDelegateCalled)
						{
							bDelegateCalled = true;
							OnFail.Broadcast();
							SetReadyToDestroy();
							return;
						}
					}
					const TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
					CloudPointerRef->OnWriteUserFileCompleteDelegates.AddUObject(this, &UEOK_SetPlayerData_AsyncFunction::OnWriteFileComplete);
					CloudPointerRef->WriteUserFile(*UserIDRef,FileName,DataToSave);
				}
				else
				{
					if(!bDelegateCalled)
					{
						bDelegateCalled = true;
						OnFail.Broadcast();
						SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						MarkAsGarbage();
#else
						MarkPendingKill();
#endif
					}
				}
			}
			else
			{
				if(!bDelegateCalled)
				{
					bDelegateCalled = true;
					OnFail.Broadcast();
					SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					MarkAsGarbage();
#else
					MarkPendingKill();
#endif
				}
			}
		}
		else
		{
			if(!bDelegateCalled)
			{
				bDelegateCalled = true;
				OnFail.Broadcast();
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
			}
		}
	}
	else
	{
		if(!bDelegateCalled)
		{
			bDelegateCalled = true;
			OnFail.Broadcast();
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}

void UEOK_SetPlayerData_AsyncFunction::OnWriteFileComplete(bool bSuccess, const FUniqueNetId& UserID,
	const FString& Var_FileName)
{
	if(bSuccess)
	{
		if(!bDelegateCalled)
		{
			bDelegateCalled = true;
			OnSuccess.Broadcast();
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
		if(!bDelegateCalled)
		{
			bDelegateCalled = true;
			OnFail.Broadcast();
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}
