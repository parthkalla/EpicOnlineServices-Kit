// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetDriverEOSKitBase.h"
#include "NetDriverEOSKit.generated.h"

class ISocketSubsystem;

/**
 * Network driver implementation for EOS Kit
 * Provides network transport using Epic Online Services
 */
UCLASS(Transient, Config=Engine)
class EOSKIT_API UNetDriverEOSKit : public UNetDriverEOSKitBase
{
	GENERATED_BODY()

public:
	UNetDriverEOSKit(const FObjectInitializer& ObjectInitializer);
};
