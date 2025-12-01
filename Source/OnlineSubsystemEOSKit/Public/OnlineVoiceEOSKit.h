// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#if __has_include("Interfaces/OnlineVoiceInterface.h")
#include "Interfaces/OnlineVoiceInterface.h"
#elif __has_include("Interfaces/VoiceInterface.h")
#include "Interfaces/VoiceInterface.h"
#else
// Forward declare if header not available
class IOnlineVoice;
#endif
#include "OnlineSubsystemEOSKit.h"

#if WITH_EOS_SDK

class FOnlineSubsystemEOSKit;

/**
 * Interface for EOS voice (stub - voice is handled through RTC subsystem)
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineVoiceEOSKit :
	public IOnlineVoice
	, public TSharedFromThis<FOnlineVoiceEOSKit, ESPMode::ThreadSafe>
{
public:
	FOnlineVoiceEOSKit() = delete;
	explicit FOnlineVoiceEOSKit(FOnlineSubsystemEOSKit* InSubsystem);
	virtual ~FOnlineVoiceEOSKit();

	// IOnlineVoice interface
	virtual IVoiceEnginePtr CreateVoiceEngine() override;
	virtual void ProcessMuteChangeNotification() override;
	virtual bool RegisterLocalTalker(uint32 LocalUserNum) override;
	virtual void RegisterLocalTalkers() override;
	virtual bool UnregisterLocalTalker(uint32 LocalUserNum) override;
	virtual void UnregisterLocalTalkers() override;
	virtual void RemoveAllRemoteTalkers() override;
	virtual bool IsHeadsetPresent(uint32 LocalUserNum) override;
	virtual bool IsLocalPlayerTalking(uint32 LocalUserNum) override;
	virtual bool IsMuted(uint32 LocalUserNum, const FUniqueNetId& PlayerId) const override;
	virtual TSharedPtr<FVoicePacket, ESPMode::ThreadSafe> GetLocalPacket(uint32 LocalUserNum) override;
	virtual int32 GetNumLocalTalkers() override;
	virtual void Tick(float DeltaTime) override;
	virtual FString GetVoiceDebugState() const override;
	// Some methods were removed from IOnlineVoice in UE 5.5, but kept for backward compatibility
	void StartNetworkedVoice(uint8 LocalUserNum);
	void StopNetworkedVoice(uint8 LocalUserNum);
	bool RegisterLocalTalker(uint8 LocalUserNum);
	void UnregisterLocalTalker(uint8 LocalUserNum);
	bool RegisterRemoteTalker(const FUniqueNetId& UniqueId);
	bool UnregisterRemoteTalker(const FUniqueNetId& UniqueId);
	bool IsHeadsetPresent(uint8 LocalUserNum);
	bool IsLocalPlayerTalking(uint8 LocalUserNum);
	bool IsRemotePlayerTalking(const FUniqueNetId& UniqueId);
	bool IsLocalPlayerMuted(uint8 LocalUserNum);
	bool MuteRemoteTalker(uint8 LocalUserNum, const FUniqueNetId& PlayerId, bool bIsSystemWide = false);
	bool UnmuteRemoteTalker(uint8 LocalUserNum, const FUniqueNetId& PlayerId, bool bIsSystemWide = false);
	TSharedPtr<class FVoicePacket> SerializeRemotePacket(FArchive& Ar);
	void ClearVoicePackets() {}

private:
	FOnlineSubsystemEOSKit* EOSKitSubsystem;
};

#endif // WITH_EOS_SDK

