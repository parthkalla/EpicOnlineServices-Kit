// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystemEOS.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
THIRD_PARTY_INCLUDES_START
#include "eos_sanctions.h"
#include "eos_sanctions_types.h"
THIRD_PARTY_INCLUDES_END
#include "EOK_SanctionsSubsystem.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Sanctions Interface", meta=(DisplayName="Sanctions Interface"))
class OnlineSubsystemEOK_API UEOK_SanctionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	//Copies an active player sanction. You must call QueryActivePlayerSanctions first to retrieve the data from the service backend. On success, EOS_Sanctions_PlayerSanction_Release must be called on OutSanction to free memory.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sanctions Interface", DisplayName="EOS_Sanctions_CopyPlayerSanctionByIndex")
	static TEnumAsByte<EEOK_Result> EOK_Sanctions_CopyPlayerSanctionByIndex(FEOK_ProductUserId LocalUserId, int32 Index, FEOK_Sanctions_PlayerSanction& OutSanction);

	//Fetch the number of player sanctions that have been retrieved for a given player. You must call QueryActivePlayerSanctions first to retrieve the data from the service backend.
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 | SDK Functions | Sanctions Interface", DisplayName="EOS_Sanctions_GetPlayerSanctionCount")
	static int32 EOK_Sanctions_GetPlayerSanctionCount(FEOK_ProductUserId LocalUserId);

};
