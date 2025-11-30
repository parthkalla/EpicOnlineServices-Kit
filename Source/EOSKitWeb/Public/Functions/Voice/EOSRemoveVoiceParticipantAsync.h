// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSRemoveVoiceParticipantAsync.generated.h"

/**
 * Async node to remove voice participant via Web API
 * Removes a player from the voice room and revokes their room token, preventing them from reusing their existing token
 */
UCLASS()
class EOSKITWEB_API UEOSRemoveVoiceParticipantAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Remove voice participant
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param RoomId - Room ID
	 * @param ProductUserId - Product User ID of the participant to remove
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Voice",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Remove Voice Participant")
	static UEOSRemoveVoiceParticipantAsync* RemoveVoiceParticipant(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& RoomId,
		const FString& ProductUserId
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	FString Var_ProductUserId;
};

