// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_Lobby_KickMember.h"

UEOK_Lobby_KickMember* UEOK_Lobby_KickMember::EOK_Lobby_KickMember(FEOK_LobbyId LobbyId,
	FEOK_ProductUserId ProductUserId, FEOK_ProductUserId TargetUserId)
{
	UEOK_Lobby_KickMember* Node = NewObject<UEOK_Lobby_KickMember>();
	Node->Var_LobbyId = LobbyId;
	Node->Var_ProductUserId = ProductUserId;
	Node->Var_TargetUserId = TargetUserId;
	return Node;
}

void UEOK_Lobby_KickMember::OnKickMemberComplete(const EOS_Lobby_KickMemberCallbackInfo* Data)
{
	if (Data == nullptr)
	{
		UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::OnKickMemberComplete] Callback data is null!"));
		return;
	}
	
	if (UEOK_Lobby_KickMember* Node = static_cast<UEOK_Lobby_KickMember*>(Data->ClientData))
	{
		// Copy critical data immediately before async task, as Data pointer may become invalid
		EOS_EResult ResultCode = Data->ResultCode;
		FString LobbyIdString;
		
		// Copy LobbyId string immediately - prefer Data->LobbyId if valid, otherwise use stored value
		if (Data->LobbyId != nullptr && Data->LobbyId[0] != '\0')
		{
			LobbyIdString = UTF8_TO_TCHAR(Data->LobbyId);
		}
		else if (Node->LobbyIdAnsiStorage.Num() > 0)
		{
			LobbyIdString = UTF8_TO_TCHAR(Node->LobbyIdAnsiStorage.GetData());
		}
		else if (!Node->Var_LobbyId.Value.IsEmpty())
		{
			LobbyIdString = Node->Var_LobbyId.Value;
		}
		else
		{
			LobbyIdString = TEXT("(unknown)");
		}
		
		AsyncTask(ENamedThreads::GameThread, [Node, ResultCode, LobbyIdString]()
		{
			const char* LobbyIdStr = TCHAR_TO_UTF8(*LobbyIdString);
			const char* ResultStr = "UNKNOWN_RESULT";
			
			// Check if result code is valid (EOS result codes are typically small positive integers)
			int32 ResultCodeInt = (int32)ResultCode;
			if (ResultCodeInt >= 0 && ResultCodeInt < 1000)
			{
				ResultStr = EOS_EResult_ToString(ResultCode);
			}
			else
			{
				UE_LOG(LogEOK, Warning, TEXT("[UEOK_Lobby_KickMember::OnKickMemberComplete] Invalid result code received: %d. This may indicate callback data corruption."), ResultCodeInt);
			}
			
			if (ResultCode == EOS_EResult::EOS_Success)
			{
				UE_LOG(LogEOK, Log, TEXT("[UEOK_Lobby_KickMember::OnKickMemberComplete] KickMember finished successfully for lobby %s. TargetUserId will be removed from lobby."), *LobbyIdString);
			}
			else
			{
				UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::OnKickMemberComplete] KickMember FAILED for lobby %s. EOS_EResult: %s (code: %d). TargetUserId may still be in the lobby."), 
					*LobbyIdString, ANSI_TO_TCHAR(ResultStr), ResultCodeInt);
			}
			
			// Use stored LobbyId for the callback result
			FEOK_LobbyId LobbyIdResult;
			LobbyIdResult.Value = LobbyIdString;
			if (Node->LobbyIdAnsiStorage.Num() > 0)
			{
				LobbyIdResult.Ref = (EOS_LobbyId)Node->LobbyIdAnsiStorage.GetData();
			}
			
			// Only broadcast valid result codes
			if (ResultCodeInt >= 0 && ResultCodeInt < 1000)
			{
				Node->OnCallback.Broadcast(static_cast<EEOK_Result>(ResultCode), LobbyIdResult);
			}
			else
			{
				UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::OnKickMemberComplete] Broadcasting EOS_InvalidState due to invalid result code."));
				Node->OnCallback.Broadcast(EEOK_Result::EOS_InvalidState, LobbyIdResult);
			}
			
			Node->SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
			Node->MarkAsGarbage();
#else
			Node->MarkPendingKill();
#endif
		});
	}
	else
	{
		UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::OnKickMemberComplete] Failed to cast ClientData to UEOK_Lobby_KickMember node!"));
	}
}

void UEOK_Lobby_KickMember::Activate()
{
	Super::Activate();
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("EOK"))
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			// Validate ProductUserIds before calling EOS API
			EOS_ProductUserId LocalUserId = Var_ProductUserId.GetValueAsEosType();
			EOS_ProductUserId TargetUserId = Var_TargetUserId.GetValueAsEosType();
			
			if (LocalUserId == nullptr || EOS_ProductUserId_IsValid(LocalUserId) == EOS_FALSE)
			{
				UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::Activate] Invalid LocalUserId (ProductUserId). User may not be logged in."));
				OnCallback.Broadcast(EEOK_Result::EOS_InvalidUser, FEOK_LobbyId());
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				return;
			}
			
			if (TargetUserId == nullptr || EOS_ProductUserId_IsValid(TargetUserId) == EOS_FALSE)
			{
				UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::Activate] Invalid TargetUserId. Cannot kick player."));
				OnCallback.Broadcast(EEOK_Result::EOS_InvalidUser, FEOK_LobbyId());
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				return;
			}
			
			// Log what we received for debugging
			UE_LOG(LogEOK, Verbose, TEXT("[UEOK_Lobby_KickMember::Activate] Received LobbyId - Value='%s', Ref=%p"), 
				*Var_LobbyId.Value, Var_LobbyId.GetValueAsEosType());
			
			// Get EOS_LobbyId - try Ref first, then Value string
			EOS_LobbyId LobbyId = Var_LobbyId.GetValueAsEosType();
			
			// If Ref is null, convert from Value string and store it to keep it alive
			if (LobbyId == nullptr)
			{
				if (Var_LobbyId.Value.IsEmpty())
				{
					UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::Activate] Invalid LobbyId. Value is empty and Ref is null. Cannot kick member."));
					OnCallback.Broadcast(EEOK_Result::EOS_InvalidParameters, FEOK_LobbyId());
					SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
					MarkAsGarbage();
#else
					MarkPendingKill();
#endif
					return;
				}
				
				// Convert FString to ANSI and store in member variable to keep it alive during async operation
				const FTCHARToUTF8 Utf8LobbyId(*Var_LobbyId.Value);
				int32 Utf8Len = Utf8LobbyId.Length() + 1; // +1 for null terminator
				LobbyIdAnsiStorage.SetNumUninitialized(Utf8Len);
				FMemory::Memcpy(LobbyIdAnsiStorage.GetData(), Utf8LobbyId.Get(), Utf8Len);
				LobbyId = (EOS_LobbyId)LobbyIdAnsiStorage.GetData();
				
				UE_LOG(LogEOK, Verbose, TEXT("[UEOK_Lobby_KickMember::Activate] Converted LobbyId from string '%s' to EOS_LobbyId"), *Var_LobbyId.Value);
			}
			else
			{
				// Ref is valid, store it as ANSI string to keep it alive
				const char* LobbyIdStr = LobbyId;
				int32 StrLen = FCStringAnsi::Strlen(LobbyIdStr) + 1; // +1 for null terminator
				LobbyIdAnsiStorage.SetNumUninitialized(StrLen);
				FMemory::Memcpy(LobbyIdAnsiStorage.GetData(), LobbyIdStr, StrLen);
				LobbyId = (EOS_LobbyId)LobbyIdAnsiStorage.GetData();
				
				UE_LOG(LogEOK, Verbose, TEXT("[UEOK_Lobby_KickMember::Activate] Using LobbyId from Ref: %hs"), LobbyIdStr);
			}
			
			// Validate LobbyId
			if (LobbyId == nullptr)
			{
				UE_LOG(LogEOK, Error, TEXT("[UEOK_Lobby_KickMember::Activate] Invalid LobbyId after conversion. Value='%s', Ref=%p. Cannot kick member."), 
					*Var_LobbyId.Value, Var_LobbyId.GetValueAsEosType());
				OnCallback.Broadcast(EEOK_Result::EOS_InvalidParameters, FEOK_LobbyId());
				SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
				MarkAsGarbage();
#else
				MarkPendingKill();
#endif
				return;
			}
			
			EOS_Lobby_KickMemberOptions Options = { };
			Options.ApiVersion = EOS_LOBBY_KICKMEMBER_API_LATEST;
			Options.LobbyId = LobbyId;
			Options.TargetUserId = TargetUserId;
			Options.LocalUserId = LocalUserId;
			
			UE_LOG(LogEOK, Verbose, TEXT("[UEOK_Lobby_KickMember::Activate] Calling EOS_Lobby_KickMember with LobbyId: %hs, LocalUserId: valid, TargetUserId: valid"), LobbyId);
			
			EOS_Lobby_KickMember(EOSRef->SessionInterfacePtr->LobbyHandle, &Options, this, &UEOK_Lobby_KickMember::OnKickMemberComplete);
			return;
		}
	}
	UE_LOG(LogEOK, Error, TEXT("Failed to kick member either OnlineSubsystem is not valid or EOSRef is not valid."));
	OnCallback.Broadcast(EEOK_Result::EOS_NotFound, FEOK_LobbyId());
	SetReadyToDestroy();
	#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
