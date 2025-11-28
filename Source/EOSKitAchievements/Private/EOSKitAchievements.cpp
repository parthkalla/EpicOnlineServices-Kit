// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitAchievements.h"

#define LOCTEXT_NAMESPACE "FEOSKitAchievementsModule"

void FEOSKitAchievementsModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Module started"));
}

void FEOSKitAchievementsModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitAchievements: Module shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSKitAchievementsModule, EOSKitAchievements)
