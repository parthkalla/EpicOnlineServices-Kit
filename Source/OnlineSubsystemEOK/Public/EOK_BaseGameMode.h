// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EOK_BaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class OnlineSubsystemEOK_API AEOK_BaseGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	
};
