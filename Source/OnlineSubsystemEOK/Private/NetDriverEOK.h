// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetDriverEOKBase.h"
#include "NetDriverEOKBase.h"
#include "NetDriverEOK.generated.h"

class ISocketSubsystem;

UCLASS(Transient, Config=Engine)
class OnlineSubsystemEOK_API UNetDriverEOK
	: public UNetDriverEOKBase
{
	GENERATED_BODY()

public:
	UNetDriverEOK(const FObjectInitializer& ObjectInitializer);
};