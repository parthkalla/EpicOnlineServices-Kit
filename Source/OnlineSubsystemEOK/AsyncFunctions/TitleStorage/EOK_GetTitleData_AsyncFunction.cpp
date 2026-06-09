// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_GetTitleData_AsyncFunction.h"

#include "EOKSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineTitleFileInterface.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"

UEOK_GetTitleData_AsyncFunction* UEOK_GetTitleData_AsyncFunction::GetTitleData(FString FileName)
{
	UEOK_GetTitleData_AsyncFunction* BlueprintNode = NewObject<UEOK_GetTitleData_AsyncFunction>();
	BlueprintNode->FileName = FileName;
	return BlueprintNode;
}

void UEOK_GetTitleData_AsyncFunction::Activate()
{
	GetTitleData();
	Super::Activate();
}

void UEOK_GetTitleData_AsyncFunction::OnGetFileProgress(const FString& FileName1, uint64 BytesRead)
{
	OnProgress.Broadcast(true, BytesRead, TArray<uint8>());
}

void UEOK_GetTitleData_AsyncFunction::GetTitleData()
{
	if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
	{
		if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
		{
			if (const IOnlineTitleFilePtr TitleFilePointerRef = SubsystemRef->GetTitleFileInterface())
			{
				if (!IdentityPointerRef->GetUniquePlayerId(0))
				{
					if (const UEOKSettings* EOKSettings = GetMutableDefault<UEOKSettings>())
					{
						{
							UE_LOG(LogEOK, Error,TEXT("EOK Log: GetTitleData: IdentityPointerRef->GetUniquePlayerId(0) is nullptr"));
						}
					}
					if (!bDelegateCalled)
					{
						bDelegateCalled = true;
						OnFail.Broadcast(false,0, TArray<uint8>());
						SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						MarkAsGarbage();
#else
						MarkPendingKill();
#endif
						return;
					}
				}
				TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
				TitleFilePointerRef->OnReadFileCompleteDelegates.AddUObject(this, &UEOK_GetTitleData_AsyncFunction::OnGetFileComplete);
				TitleFilePointerRef->OnReadFileProgressDelegates.AddUObject(this, &UEOK_GetTitleData_AsyncFunction::OnGetFileProgress);
				TitleFilePointerRef->ReadFile(FileName);
			}
			else
			{
				if (!bDelegateCalled)
				{
					bDelegateCalled = true;
					OnFail.Broadcast(false, 0,TArray<uint8>());
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
			if (!bDelegateCalled)
			{
				bDelegateCalled = true;
				OnFail.Broadcast(false, 0, TArray<uint8>());
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
		if (!bDelegateCalled)
		{
			bDelegateCalled = true;
			OnFail.Broadcast(false, 0,TArray<uint8>());
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}

void UEOK_GetTitleData_AsyncFunction::OnGetFileComplete(bool bSuccess, const FString& V_FileName)
{
	if (bSuccess)
	{
		if (const IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				if (const IOnlineTitleFilePtr TitleFilePointerRef = SubsystemRef->GetTitleFileInterface())
				{
					TSharedPtr<const FUniqueNetId> UserIDRef = IdentityPointerRef->GetUniquePlayerId(0).ToSharedRef();
					TArray<uint8> FileContents;
					TitleFilePointerRef->GetFileContents(V_FileName, FileContents);
					if (FileContents.Num() > 0)
					{
						if (!bDelegateCalled)
						{
							bDelegateCalled = true;
							OnSuccess.Broadcast(true,0, FileContents);
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
						if (!bDelegateCalled)
						{
							bDelegateCalled = true;
							OnFail.Broadcast(false, 0,TArray<uint8>());
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
					if (!bDelegateCalled)
					{
						bDelegateCalled = true;
						OnFail.Broadcast(false, 0,TArray<uint8>());
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
				if (!bDelegateCalled)
				{
					bDelegateCalled = true;
					OnFail.Broadcast(false,0, TArray<uint8>());
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
			if (!bDelegateCalled)
			{
				bDelegateCalled = true;
				OnFail.Broadcast(false, 0,TArray<uint8>());
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
		if (!bDelegateCalled)
		{
			bDelegateCalled = true;
			OnFail.Broadcast(false,0, TArray<uint8>());
			SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			MarkAsGarbage();
#else
			MarkPendingKill();
#endif
		}
	}
}
