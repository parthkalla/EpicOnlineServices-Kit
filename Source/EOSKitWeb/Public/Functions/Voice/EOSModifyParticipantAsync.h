// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOSKitBaseWebApi.h"
#include "EOSModifyParticipantAsync.generated.h"

/**
 * Async node to modify participant via Web API
 * Room members can be remote-muted, i.e. be muted for everyone, independent of their local mute status.
 */
UCLASS()
class EOSKITWEB_API UEOSModifyParticipantAsync : public UEOSKitBaseWebApi
{
	GENERATED_BODY()

public:
	/**
	 * Modify participant
	 * @param Authorization - Bearer token for authentication
	 * @param DeploymentId - Deployment ID
	 * @param RoomId - Room ID
	 * @param ProductUserId - Product User ID of the participant
	 * @param bHardMuted - Whether to hard mute the participant
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Web|Voice",
		meta = (BlueprintInternalUseOnly = "true"),
		DisplayName = "Modify Participant")
	static UEOSModifyParticipantAsync* ModifyParticipant(
		const FString& Authorization,
		const FString& DeploymentId,
		const FString& RoomId,
		const FString& ProductUserId,
		bool bHardMuted
	);

private:
	virtual void Activate() override;

	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	FString Var_ProductUserId;
	bool Var_bHardMuted;
};

