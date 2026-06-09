// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

class FSocketSubsystemEOKModule: public IModuleInterface
{
public:
	FSocketSubsystemEOKModule() = default;
	~FSocketSubsystemEOKModule() = default;

private:
	// ~Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// ~End IModuleInterface
};