// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_ModifyParticipant.generated.h"

/**
 * 
 */
UCLASS()
class EOKWeb_API UEOK_API_ModifyParticipant : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:
	
	//Room members can be remote-muted, i.e. be muted for everyone, independent of their local mute status.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_ModifyParticipant* ModifyParticipant(FString Authorization, FString DeploymentId, FString RoomId, FString ProductUserId, bool bHardMuted);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	FString Var_RoomId;
	FString Var_ProductUserId;
	bool Var_bHardMuted;
};
