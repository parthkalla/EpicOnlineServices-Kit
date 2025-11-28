// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SynthComponent.h"
#include "DSP/BufferVectorOperations.h"
#include "EOSVoiceChatSynthComponent.generated.h"

/**
 * Synth component that renders incoming voice chat audio
 * Receives audio data from EOS RTC callbacks and plays it back
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class EOSKITVOICE_API UEOSVoiceChatSynthComponent : public USynthComponent
{
	GENERATED_BODY()

public:
	UEOSVoiceChatSynthComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Add audio buffer data from EOS callback
	 * Thread-safe method to push incoming audio data
	 * @param Buffer The audio data buffer
	 * @param NumSamples Number of samples in the buffer
	 * @param SampleRate Sample rate of the audio
	 * @param InNumChannels Number of audio channels
	 */
	void AddAudioBuffer(const float* Buffer, int32 NumSamples, int32 SampleRate, int32 InNumChannels);

	/**
	 * Add audio buffer from TArray
	 * @param Buffer The audio data buffer
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Voice")
	void AddAudioBufferArray(const TArray<float>& Buffer);

	/**
	 * Clear all pending audio data
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Voice")
	void ClearAudioBuffer();

	/**
	 * Get the current buffer size
	 * @return Number of samples currently buffered
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Voice")
	int32 GetBufferSize() const;

	/**
	 * Set the volume multiplier for this voice
	 * @param InVolume Volume multiplier (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Voice")
	void SetVoiceVolume(float InVolume);

protected:
	// USynthComponent interface
	virtual bool Init(int32& SampleRate) override;
	virtual int32 OnGenerateAudio(float* OutAudio, int32 NumSamples) override;

	// Audio buffer management
	TArray<float> AudioBuffer;
	mutable FCriticalSection AudioBufferCS;

	// Audio settings
	int32 CurrentSampleRate;
	int32 CurrentNumChannels;
	float VolumeMultiplier;

	// Buffer thresholds
	static constexpr int32 MaxBufferSize = 48000 * 2; // 2 seconds at 48kHz
	static constexpr int32 MinBufferSize = 4800; // 100ms at 48kHz
};
