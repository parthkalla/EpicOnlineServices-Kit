// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_CreateRoomTokens.generated.h"

USTRUCT(BlueprintType)
struct FEWebApi_EosRoomParticipant
{
	GENERATED_BODY()

	//The EOS ProductUserId
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	FString ProductUserId;

	//IP address of the player (also known as the "participant"). The service uses this data to select a server for the voice session that is close to the player's location. If there is no IP address provided, the service chooses a server in a default location. This might result in a poor player experience: the server might not be close to the player's location which might result in the player experiencing high network latency during the voice session.
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	FString ClientIp;

	//Initial mute status of the participant
	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	bool bHardMuted = false;
};

UCLASS()
class EOKWeb_API UEOK_API_CreateRoomTokens : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:

	/*
	*In order to join a voice conference on a media server, users need to provide a room token.

	The createRoomToken request queries these tokens for a list of ProductUserIds, using player IP addresses to choose a server close to a player's location. The player IP address is optional, however for the best user experience, we recommend including this data. This is because a server close to a player's location has low network latency during the voice session.

	If the specified roomId does not exist, it is created.

	The result contains the players room tokens as well as the endpoint for the media server that hosts the voice session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_CreateRoomTokens* CreateVoiceRoomToken(FString Authorization, FString DeploymentId, FString RoomId, TArray<FEWebApi_EosRoomParticipant> Participants);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	TArray<FEWebApi_EosRoomParticipant> Var_Participants;
};
