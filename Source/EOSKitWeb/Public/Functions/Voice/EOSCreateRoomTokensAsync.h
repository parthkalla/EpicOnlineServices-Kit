// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSCreateRoomTokensAsync.generated.h"

/**
 * Room participant structure
 */
USTRUCT(BlueprintType)
struct FEOSKitRoomParticipant
{
	GENERATED_BODY()

	/** The EOS Product User ID */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Web|Voice")
	FString ProductUserId = TEXT("");

	/** IP address of the player (optional, used to select a server close to the player) */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Web|Voice")
	FString ClientIp = TEXT("");

	/** Initial mute status of the participant */
	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Web|Voice")
	bool bHardMuted = false;
};

/**
 * Async node to create room tokens via Web API
 * In order to join a voice conference on a media server, users need to provide a room token.
 * If the specified roomId does not exist, it is created.
 */
UCLASS()
class EOSKITWEB_API UEOSCreateRoomTokensAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Create voice room tokens
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param RoomId - Room ID
	 * @param Participants - Array of participants
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Voice",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Create Room Tokens")
	static UEOSCreateRoomTokensAsync* CreateRoomTokens(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& RoomId,
		const TArray<FEOSKitRoomParticipant>& Participants
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	TArray<FEOSKitRoomParticipant> Var_Participants;
};

