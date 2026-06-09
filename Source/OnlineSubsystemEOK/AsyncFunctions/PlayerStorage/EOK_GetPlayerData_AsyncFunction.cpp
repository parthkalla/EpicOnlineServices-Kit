// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_GetPlayerData_AsyncFunction.h"

#include "EOKSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_GetPlayerData_AsyncFunction* UEOK_GetPlayerData_AsyncFunction::GetPlayerData(FString FileName)
{
	UEOK_GetPlayerData_AsyncFunction* BlueprintNode = NewObject<UEOK_GetPlayerData_AsyncFunction>();
	BlueprintNode->FileName = FileName;
	return BlueprintNode;
}

void UEOK_GetPlayerData_AsyncFunction::Activate()
{
	GetPlayerData();
	Super::Activate();
}

void UEOK_GetPlayerData_AsyncFunction::GetPlayerData()
{
	if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get())
	{
		if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if(const IOnlineUserCloudPtr CloudPointerRef = SubsystemRef->GetUserCloudInterface())
			{
				if(!IdentityPointerRef->GetUniquePlayerId(0))
				{
						{
							UE_LOG(LogEOK, Error, TEXT("EOK Log: GetPlayerData: IdentityPointerRef->GetUniquePlayerId(0) is nullptr"));
						}
					if(!bDelegateCalled)
					{
						bDelegateCalled = true;
						OnFail.Broadcast(false, TArray<uint8>());
						SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						MarkAsGarbage();
#else
						MarkPendingKill();
#endif
					}
				}
				TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
				CloudPointerRef->OnReadUserFileCompleteDelegates.AddUObject(this, &UEOK_GetPlayerData_AsyncFunction::OnGetFileComplete);
				CloudPointerRef->ReadUserFile(*UserIDRef, FileName);
			}
			else
			{
				if(!bDelegateCalled)
				{
					bDelegateCalled = true;
					OnFail.Broadcast(false, TArray<uint8>());
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
				OnFail.Broadcast(false, TArray<uint8>());
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
			OnFail.Broadcast(false, TArray<uint8>());
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}

void UEOK_GetPlayerData_AsyncFunction::OnGetFileComplete(bool bSuccess, const FUniqueNetId& UserID,
	const FString& V_FileName)
{
	if(bSuccess)
	{
		if(const IOnlineSubsystem *SubsystemRef = IOnlineSubsystem::Get())
		{
			if(const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				if(const IOnlineUserCloudPtr CloudPointerRef = SubsystemRef->GetUserCloudInterface())
				{
					TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
					TArray<uint8> FileContents;
					CloudPointerRef->GetFileContents(*UserIDRef,FileName,FileContents);
					if(FileContents.Num() > 0)
					{
						if(!bDelegateCalled)
						{
							bDelegateCalled = true;
							OnSuccess.Broadcast(true, FileContents);
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
							OnFail.Broadcast(false, TArray<uint8>());
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
						OnFail.Broadcast(false, TArray<uint8>());
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
					OnFail.Broadcast(false, TArray<uint8>());
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
				OnFail.Broadcast(false, TArray<uint8>());
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
			OnFail.Broadcast(false, TArray<uint8>());
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}
