// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitVoiceSubsystem.generated.h"

#if WITH_EOS_RTC
class FEOSVoiceChat;
#endif

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceConnectionComplete, EEOSResult, Result, const FString&, RoomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceUserJoined, const FString&, UserId, const FString&, RoomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceUserLeft, const FString&, UserId, const FString&, RoomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceUserTalking, const FString&, UserId, bool, bIsTalking);

/**
 * Manages EOS RTC voice chat connections and state
 */
UCLASS()
class EOSKITVOICE_API UEOSKitVoiceSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Voice")
	FOnVoiceConnectionComplete OnVoiceConnectionComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Voice")
	FOnVoiceUserJoined OnVoiceUserJoined;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Voice")
	FOnVoiceUserLeft OnVoiceUserLeft;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Voice")
	FOnVoiceUserTalking OnVoiceUserTalking;

	/**
	 * Connect to a voice room
	 * @param LocalUserId The Product User ID of the local player
	 * @param RoomName The name of the voice room to join
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Voice")
	void ConnectVoice(const FString& LocalUserId, const FString& RoomName);

	/**
	 * Disconnect from a voice room
	 * @param LocalUserId The Product User ID of the local player
	 * @param RoomName The name of the voice room to leave
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Voice")
	void DisconnectVoice(const FString& LocalUserId, const FString& RoomName);

	/**
	 * Mute/unmute local microphone
	 * @param bMuted true to mute, false to unmute
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Voice")
	void SetMicrophoneMuted(bool bMuted);

	/**
	 * Get voice chat manager
	 */
#if WITH_EOS_RTC
	TSharedPtr<FEOSVoiceChat> GetVoiceChatManager() const { return VoiceChatManager; }
#endif

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
#if WITH_EOS_RTC
	TSharedPtr<FEOSVoiceChat> VoiceChatManager;
#endif

	FString CurrentRoomName;
	FString CurrentLocalUserId;
	bool bIsMuted;
};
