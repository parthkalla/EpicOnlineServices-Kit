// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "EOK_EngineSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class OnlineSubsystemEOK_API UEOK_EngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize( FSubsystemCollectionBase& Collection ) override;

	void BeginPIE( const bool bIsSimulating );
	
};
