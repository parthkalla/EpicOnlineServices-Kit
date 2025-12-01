// Copyright (C) 2024, All Rights Reserved.

#include "OnlineVoiceEOSKit.h"
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK

FOnlineVoiceEOSKit::FOnlineVoiceEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
{
}

FOnlineVoiceEOSKit::~FOnlineVoiceEOSKit()
{
}

void FOnlineVoiceEOSKit::StartNetworkedVoice(uint8 LocalUserNum)
{
	// Voice is handled through EOSKitRTC subsystem
}

void FOnlineVoiceEOSKit::StopNetworkedVoice(uint8 LocalUserNum)
{
	// Voice is handled through EOSKitRTC subsystem
}

bool FOnlineVoiceEOSKit::RegisterLocalTalker(uint8 LocalUserNum)
{
	return false;
}

void FOnlineVoiceEOSKit::UnregisterLocalTalker(uint8 LocalUserNum)
{
}

bool FOnlineVoiceEOSKit::RegisterRemoteTalker(const FUniqueNetId& UniqueId)
{
	return false;
}

bool FOnlineVoiceEOSKit::UnregisterRemoteTalker(const FUniqueNetId& UniqueId)
{
	return false;
}

bool FOnlineVoiceEOSKit::IsHeadsetPresent(uint8 LocalUserNum)
{
	return false;
}

bool FOnlineVoiceEOSKit::IsLocalPlayerTalking(uint8 LocalUserNum)
{
	return false;
}

bool FOnlineVoiceEOSKit::IsRemotePlayerTalking(const FUniqueNetId& UniqueId)
{
	return false;
}

bool FOnlineVoiceEOSKit::IsLocalPlayerMuted(uint8 LocalUserNum)
{
	return false;
}

bool FOnlineVoiceEOSKit::MuteRemoteTalker(uint8 LocalUserNum, const FUniqueNetId& PlayerId, bool bIsSystemWide)
{
	return false;
}

bool FOnlineVoiceEOSKit::UnmuteRemoteTalker(uint8 LocalUserNum, const FUniqueNetId& PlayerId, bool bIsSystemWide)
{
	return false;
}

TSharedPtr<class FVoicePacket> FOnlineVoiceEOSKit::SerializeRemotePacket(FArchive& Ar)
{
	return nullptr;
}

IVoiceEnginePtr FOnlineVoiceEOSKit::CreateVoiceEngine()
{
	// Create voice engine - not implemented yet
	return nullptr;
}

void FOnlineVoiceEOSKit::ProcessMuteChangeNotification()
{
	// Process mute change notification - not implemented yet
}

bool FOnlineVoiceEOSKit::RegisterLocalTalker(uint32 LocalUserNum)
{
	// Register local talker - not implemented yet
	return false;
}

void FOnlineVoiceEOSKit::RegisterLocalTalkers()
{
	// Register local talkers - not implemented yet
}

bool FOnlineVoiceEOSKit::UnregisterLocalTalker(uint32 LocalUserNum)
{
	// Unregister local talker - not implemented yet
	return false;
}

void FOnlineVoiceEOSKit::UnregisterLocalTalkers()
{
	// Unregister local talkers - not implemented yet
}

void FOnlineVoiceEOSKit::RemoveAllRemoteTalkers()
{
	// Remove all remote talkers - not implemented yet
}

bool FOnlineVoiceEOSKit::IsHeadsetPresent(uint32 LocalUserNum)
{
	// Is headset present - not implemented yet
	return false;
}

bool FOnlineVoiceEOSKit::IsLocalPlayerTalking(uint32 LocalUserNum)
{
	// Is local player talking - not implemented yet
	return false;
}

bool FOnlineVoiceEOSKit::IsMuted(uint32 LocalUserNum, const FUniqueNetId& PlayerId) const
{
	// Is muted - not implemented yet
	return false;
}

TSharedPtr<FVoicePacket, ESPMode::ThreadSafe> FOnlineVoiceEOSKit::GetLocalPacket(uint32 LocalUserNum)
{
	// Get local packet - not implemented yet
	return nullptr;
}

int32 FOnlineVoiceEOSKit::GetNumLocalTalkers()
{
	// Get num local talkers - not implemented yet
	return 0;
}

void FOnlineVoiceEOSKit::Tick(float DeltaTime)
{
	// Tick - not implemented yet
}

FString FOnlineVoiceEOSKit::GetVoiceDebugState() const
{
	// Get voice debug state - not implemented yet
	return TEXT("Not implemented");
}

#endif // WITH_EOS_SDK

