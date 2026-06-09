// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_RemoveVoiceParticipant.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_RemoveVoiceParticipant : public UEOK_BaseWebApi
{
	GENERATED_BODY()
	
public:

	//The removeParticipant request will remove a player from the voice room and revoke their room token, preventing them from reusing their existing token to join the same room again.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_RemoveVoiceParticipant* RemoveVoiceParticipant(FString Authorization, FString DeploymentId, FString RoomId, FString ProductUserId);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	FString Var_ProductUserId;
};
