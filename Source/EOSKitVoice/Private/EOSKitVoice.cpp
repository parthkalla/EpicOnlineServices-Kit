// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitVoice.h"

#define LOCTEXT_NAMESPACE "FEOSKitVoiceModule"

void FEOSKitVoiceModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Module started"));
}

void FEOSKitVoiceModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKitVoice: Module shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSKitVoiceModule, EOSKitVoice)
