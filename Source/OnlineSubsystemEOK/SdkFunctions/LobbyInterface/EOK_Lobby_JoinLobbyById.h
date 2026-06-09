// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_Lobby_JoinLobbyById.generated.h"

USTRUCT(BlueprintType)
struct FEOK_Lobby_JoinLobbyByIdOptions
{
	GENERATED_BODY()

	//The ID of the lobby
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface")
	FEOK_LobbyId LobbyId;

	//The Product User ID of the local user joining the lobby
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface")
	FEOK_ProductUserId LocalUserId;

	//If true, this lobby will be associated with the user's presence information. A user can only associate one lobby at a time with their presence information. This affects the ability of the Social Overlay to show game related actions to take in the user's social graph. * using the bPresenceEnabled flags within the Sessions interface * using the bPresenceEnabled flags within the Lobby interface * using EOS_PresenceModification_SetJoinInfo
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface")
	bool bPresenceEnabled;

	//(Optional) Set this value to override the default local options for the RTC Room, if it is enabled for this lobby. Set this to NULL if your application does not use the Lobby RTC Rooms feature, or if you would like to use the default settings. This option is ignored if the specified lobby does not have an RTC Room enabled and will not cause errors.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface")
	FEOK_Lobby_LocalRTCOptions LobbyRTCOptions;

	//This value indicates whether or not the local user allows crossplay interactions. If it is false, the local user will be treated as allowing crossplay.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface")
	bool bCrossplayOptOut;

	FEOK_Lobby_JoinLobbyByIdOptions()
	{
		bPresenceEnabled = false;
		bCrossplayOptOut = false;
	}
	EOS_Lobby_JoinLobbyByIdOptions ToEOSOptions()
	{
		EOS_Lobby_JoinLobbyByIdOptions Options;
		// CRITICAL FIX: Use API version 4 instead of LATEST (5) because the EOS SDK library only supports versions 1-4
		Options.ApiVersion = 4; // EOS_LOBBY_JOINLOBBY_API_LATEST is 5, but SDK only supports up to 4
		Options.LobbyId = LobbyId.Ref;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.bPresenceEnabled = bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
		EOS_Lobby_LocalRTCOptions VarTemp = LobbyRTCOptions.GetValueAsEosType();
		Options.LocalRTCOptions = &VarTemp;
		Options.bCrossplayOptOut = bCrossplayOptOut ? EOS_TRUE : EOS_FALSE;
		return Options;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_Lobby_JoinLobbyByIdDelegate, const TEnumAsByte<EEOK_Result>&, Result, const FEOK_LobbyId&, LobbyId);

UCLASS()
class OnlineSubsystemEOK_API UEOK_Lobby_JoinLobbyById : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//This is a special case of EOS_Lobby_JoinLobby. It should only be used if the lobby has had Join-by-ID enabled. Additionally, Join-by-ID should only be enabled to support native invites on an integrated platform.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_JoinLobbyById")
	static UEOK_Lobby_JoinLobbyById* EOK_Lobby_JoinLobbyById(FEOK_Lobby_JoinLobbyByIdOptions Options);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 | SDK Functions | Lobby Interface")
	FEOK_Lobby_JoinLobbyByIdDelegate OnCallback;
private:
	static void EOS_CALL OnJoinLobbyByIdComplete(const EOS_Lobby_JoinLobbyByIdCallbackInfo* Data);
	virtual void Activate() override;
	FEOK_Lobby_JoinLobbyByIdOptions Var_Options;
};
