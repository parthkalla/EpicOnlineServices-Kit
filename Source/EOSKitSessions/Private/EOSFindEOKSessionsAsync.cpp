// Copyright (C) 2024, All Rights Reserved.

#include "EOSFindEOKSessionsAsync.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EOS_SDK
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include "eos_platform.h"
#include "eos_sessions.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"

UEOSFindEOKSessionsAsync* UEOSFindEOKSessionsAsync::FindEOKSessions(
	UObject* WorldContextObject,
	TMap<FString, FEOSKitAttribute> SessionSettings,
	EEOSKitMatchType MatchType,
	int32 MaxResults,
	EEOSKitRegion RegionToSearch,
	bool bLanSearch,
	bool bIncludePartySessions)
{
	UEOSFindEOKSessionsAsync* Node = NewObject<UEOSFindEOKSessionsAsync>();
	Node->CachedWorldContextObject = WorldContextObject;
	Node->SessionSettings = SessionSettings;
	Node->E_MatchType = MatchType;
	Node->I_MaxResults = MaxResults;
	Node->E_RegionToSearch = RegionToSearch;
	Node->B_bLanSearch = bLanSearch;
	Node->bIncludePartySessions = bIncludePartySessions;
	return Node;
}

void UEOSFindEOKSessionsAsync::Activate()
{
	FindSession();
	Super::Activate();
}

void UEOSFindEOKSessionsAsync::FindSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Finding EOS Sessions via SDK"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Max Results: %d"), I_MaxResults);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ========================================"));
	
	// Get the GameInstance from CachedWorldContextObject
	if (!CachedWorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: WorldContextObject is null"));
		OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(CachedWorldContextObject);
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Game Instance"));
		OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem || !EOSKitSubsystem->GetPlatformHandle())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: EOSKit Subsystem or Platform Handle is null"));
		OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	EOS_HPlatform PlatformHandle = EOSKitSubsystem->GetPlatformHandle();
	EOS_HSessions SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformHandle);
	
	if (!SessionsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get Sessions Handle"));
		OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Get Product User ID
	EOS_ProductUserId LocalUserId = EOSKitSubsystem->GetProductUserId();
	if (!LocalUserId)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Product User ID is null - user must be logged in"));
		OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	// Create callback context - STORE UTF8 STRINGS!
	struct FSessionFindContext
	{
		UEOSFindEOKSessionsAsync* AsyncNode;
		EOS_HSessionSearch SearchHandle;
		TArray<TArray<uint8>> AttributeKeysUTF8;
		TArray<TArray<uint8>> AttributeValuesUTF8;
	};
	
	FSessionFindContext* CallbackContext = new FSessionFindContext();
	CallbackContext->AsyncNode = this;
	
	// Step 1: Create Session Search Handle
	EOS_Sessions_CreateSessionSearchOptions SearchOptions = {};
	SearchOptions.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
	SearchOptions.MaxSearchResults = I_MaxResults;
	
	EOS_HSessionSearch SearchHandle = nullptr;
	EOS_EResult CreateSearchResult = EOS_Sessions_CreateSessionSearch(SessionsHandle, &SearchOptions, &SearchHandle);
	
	if (CreateSearchResult != EOS_EResult::EOS_Success || !SearchHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to create session search: %s"), 
			*FString(EOS_EResult_ToString(CreateSearchResult)));
		delete CallbackContext;
		OnFail.Broadcast(TArray<FEOSKitSessionFindResult>());
		SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
		return;
	}
	
	CallbackContext->SearchHandle = SearchHandle;
	
	// Step 2: Set search parameters (filters) - STORE STRINGS!
	for (const auto& Setting : SessionSettings)
	{
		if (Setting.Key.IsEmpty())
		{
			continue;
		}
		
		// Convert and store key
		FTCHARToUTF8 KeyConverter(*Setting.Key);
		TArray<uint8> KeyUTF8;
		KeyUTF8.SetNum(KeyConverter.Length() + 1);
		FMemory::Memcpy(KeyUTF8.GetData(), KeyConverter.Get(), KeyConverter.Length() + 1);
		
		EOS_Sessions_AttributeData AttrData = {};
		AttrData.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
		AttrData.Key = (const char*)KeyUTF8.GetData();
		
		// Set value based on type and store if string
		TArray<uint8> ValueUTF8;
		if (!Setting.Value.StringValue.IsEmpty())
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
			FTCHARToUTF8 ValueConverter(*Setting.Value.StringValue);
			ValueUTF8.SetNum(ValueConverter.Length() + 1);
			FMemory::Memcpy(ValueUTF8.GetData(), ValueConverter.Get(), ValueConverter.Length() + 1);
			AttrData.Value.AsUtf8 = (const char*)ValueUTF8.GetData();
		}
		else if (Setting.Value.IntValue != 0)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
			AttrData.Value.AsInt64 = Setting.Value.IntValue;
		}
		else if (Setting.Value.BoolValue)
		{
			AttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Boolean;
			AttrData.Value.AsBool = EOS_TRUE;
		}
		else
		{
			continue; // Skip empty values
		}
		
		// Set parameter for the search
		EOS_SessionSearch_SetParameterOptions ParamOptions = {};
		ParamOptions.ApiVersion = EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST;
		ParamOptions.Parameter = &AttrData;
		ParamOptions.ComparisonOp = EOS_EComparisonOp::EOS_CO_EQUAL;
		
		EOS_SessionSearch_SetParameter(SearchHandle, &ParamOptions);
		
		// Store UTF8 data in context
		CallbackContext->AttributeKeysUTF8.Add(KeyUTF8);
		if (!ValueUTF8.IsEmpty())
		{
			CallbackContext->AttributeValuesUTF8.Add(ValueUTF8);
		}
	}
	
	// Step 3: Execute the search
	EOS_SessionSearch_FindOptions FindOptions = {};
	FindOptions.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
	FindOptions.LocalUserId = LocalUserId;
	
	EOS_SessionSearch_Find(SearchHandle, &FindOptions, CallbackContext,
		[](const EOS_SessionSearch_FindCallbackInfo* Data)
		{
			FSessionFindContext* Context = static_cast<FSessionFindContext*>(Data->ClientData);
			
			if (!Context || !Context->AsyncNode)
			{
				if (Context && Context->SearchHandle)
				{
					EOS_SessionSearch_Release(Context->SearchHandle);
				}
				delete Context;
				return;
			}
			
			// Execute on game thread
			AsyncTask(ENamedThreads::GameThread, [Context, Data]()
			{
				TArray<FEOSKitSessionFindResult> Results;
				
				if (Data->ResultCode == EOS_EResult::EOS_Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Session search completed successfully!"));
					
					// Get search result count
					EOS_SessionSearch_GetSearchResultCountOptions CountOptions = {};
					CountOptions.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
					
					uint32_t ResultCount = EOS_SessionSearch_GetSearchResultCount(Context->SearchHandle, &CountOptions);
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Found %d sessions"), ResultCount);
					
					// Iterate through results
					for (uint32_t i = 0; i < ResultCount; i++)
					{
						EOS_SessionSearch_CopySearchResultByIndexOptions CopyOptions = {};
						CopyOptions.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
						CopyOptions.SessionIndex = i;
						
						EOS_HSessionDetails SessionHandle = nullptr;
						EOS_EResult CopyResult = EOS_SessionSearch_CopySearchResultByIndex(Context->SearchHandle, &CopyOptions, &SessionHandle);
						
						if (CopyResult == EOS_EResult::EOS_Success && SessionHandle)
						{
							// Copy session info
							EOS_SessionDetails_CopyInfoOptions InfoOptions = {};
							InfoOptions.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
							
							EOS_SessionDetails_Info* SessionInfo = nullptr;
							EOS_EResult InfoResult = EOS_SessionDetails_CopyInfo(SessionHandle, &InfoOptions, &SessionInfo);
							
							if (InfoResult == EOS_EResult::EOS_Success && SessionInfo)
							{
								FEOSKitSessionFindResult Result;
								Result.SessionName = UTF8_TO_TCHAR(SessionInfo->SessionId);
								Result.CurrentNumberOfPlayers = SessionInfo->NumOpenPublicConnections;
								Result.MaxNumberOfPlayers = SessionInfo->Settings ? SessionInfo->Settings->NumPublicConnections : 0;
								
								// Note: To copy attributes, you would use EOS_SessionDetails_CopySessionAttributeByIndex
								// For simplicity, we skip that here
								
								Results.Add(Result);
								
								// Release session info
								EOS_SessionDetails_Info_Release(SessionInfo);
							}
							
							// Release session handle
							EOS_SessionDetails_Release(SessionHandle);
						}
					}
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnSuccess.Broadcast(Results);
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("EOSKit: ? Failed to find sessions: %s"), 
						*FString(EOS_EResult_ToString(Data->ResultCode)));
					
					if (Context->AsyncNode)
					{
						Context->AsyncNode->OnFail.Broadcast(Results);
						Context->AsyncNode->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
						Context->AsyncNode->MarkAsGarbage();
#else
						Context->AsyncNode->MarkPendingKill();
#endif
					}
				}
				
				// Release search handle
				if (Context->SearchHandle)
				{
					EOS_SessionSearch_Release(Context->SearchHandle);
				}
				
				delete Context;
			});
		});
}

void UEOSFindEOKSessionsAsync::OnFindSessionCompleted(bool bWasSuccess)
{
	// Legacy callback - not used anymore
}
