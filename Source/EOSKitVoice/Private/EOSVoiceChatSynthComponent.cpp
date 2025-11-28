// Copyright (C) 2024, All Rights Reserved.

#include "EOSVoiceChatSynthComponent.h"
#include "AudioDevice.h"
#include "DSP/FloatArrayMath.h"

UEOSVoiceChatSynthComponent::UEOSVoiceChatSynthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentSampleRate(48000)
	, CurrentNumChannels(1)
	, VolumeMultiplier(1.0f)
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

bool UEOSVoiceChatSynthComponent::Init(int32& SampleRate)
{
	NumChannels = 2; // Stereo output
	SampleRate = 48000; // Standard sample rate for voice
	CurrentSampleRate = SampleRate;

	// Pre-allocate buffer
	AudioBuffer.Reserve(MaxBufferSize);

	UE_LOG(LogTemp, Log, TEXT("EOSVoiceChatSynthComponent: Initialized with SampleRate=%d, NumChannels=%d"), 
		SampleRate, NumChannels);

	return true;
}

int32 UEOSVoiceChatSynthComponent::OnGenerateAudio(float* OutAudio, int32 NumSamples)
{
	if (!OutAudio || NumSamples <= 0)
	{
		return NumSamples;
	}

	// Lock the audio buffer
	FScopeLock Lock(&AudioBufferCS);

	// Calculate how many samples we can actually provide
	int32 SamplesToGenerate = FMath::Min(NumSamples, AudioBuffer.Num());

	if (SamplesToGenerate > 0)
	{
		// Copy audio data to output buffer
		FMemory::Memcpy(OutAudio, AudioBuffer.GetData(), SamplesToGenerate * sizeof(float));

		// Apply volume
		if (VolumeMultiplier != 1.0f)
		{
			for (int32 i = 0; i < SamplesToGenerate; ++i)
			{
				OutAudio[i] *= VolumeMultiplier;
			}
		}

		// Remove consumed samples from buffer
		AudioBuffer.RemoveAt(0, SamplesToGenerate, EAllowShrinking::No);

		// Fill remaining with silence if needed
		if (SamplesToGenerate < NumSamples)
		{
			FMemory::Memzero(OutAudio + SamplesToGenerate, (NumSamples - SamplesToGenerate) * sizeof(float));
		}
	}
	else
	{
		// No data available, output silence
		FMemory::Memzero(OutAudio, NumSamples * sizeof(float));
	}

	return NumSamples;
}

void UEOSVoiceChatSynthComponent::AddAudioBuffer(const float* Buffer, int32 NumSamples, int32 SampleRate, int32 InNumChannels)
{
	if (!Buffer || NumSamples <= 0)
	{
		return;
	}

	FScopeLock Lock(&AudioBufferCS);

	// Check if buffer is getting too large
	if (AudioBuffer.Num() >= MaxBufferSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSVoiceChatSynthComponent: Audio buffer overflow, clearing old data"));
		AudioBuffer.RemoveAt(0, AudioBuffer.Num() / 2, EAllowShrinking::No); // Remove half the buffer
	}

	// Handle sample rate conversion if needed
	if (SampleRate != CurrentSampleRate)
	{
		// Simple resampling (in production, use a proper resampler)
		float Ratio = static_cast<float>(CurrentSampleRate) / static_cast<float>(SampleRate);
		int32 NewNumSamples = static_cast<int32>(NumSamples * Ratio);
		
		// For now, just log a warning - proper resampling would be needed here
		UE_LOG(LogTemp, Warning, TEXT("EOSVoiceChatSynthComponent: Sample rate mismatch (got %d, expected %d)"), 
			SampleRate, CurrentSampleRate);
	}

	// Handle channel conversion (mono to stereo if needed)
	if (InNumChannels == 1 && this->NumChannels == 2)
	{
		// Mono to stereo - duplicate samples
		int32 StartIndex = AudioBuffer.Num();
		AudioBuffer.AddUninitialized(NumSamples * 2);
		
		for (int32 i = 0; i < NumSamples; ++i)
		{
			float Sample = Buffer[i];
			AudioBuffer[StartIndex + (i * 2)] = Sample;
			AudioBuffer[StartIndex + (i * 2) + 1] = Sample;
		}
	}
	else if (InNumChannels == 2 && this->NumChannels == 2)
	{
		// Stereo to stereo - direct copy
		int32 StartIndex = AudioBuffer.Num();
		AudioBuffer.AddUninitialized(NumSamples);
		FMemory::Memcpy(&AudioBuffer[StartIndex], Buffer, NumSamples * sizeof(float));
	}
	else
	{
		// Other channel configurations - just copy as is
		int32 StartIndex = AudioBuffer.Num();
		AudioBuffer.AddUninitialized(NumSamples);
		FMemory::Memcpy(&AudioBuffer[StartIndex], Buffer, NumSamples * sizeof(float));
	}
}

void UEOSVoiceChatSynthComponent::AddAudioBufferArray(const TArray<float>& Buffer)
{
	if (Buffer.Num() > 0)
	{
		AddAudioBuffer(Buffer.GetData(), Buffer.Num(), CurrentSampleRate, NumChannels);
	}
}

void UEOSVoiceChatSynthComponent::ClearAudioBuffer()
{
	FScopeLock Lock(&AudioBufferCS);
	AudioBuffer.Empty();
	UE_LOG(LogTemp, Log, TEXT("EOSVoiceChatSynthComponent: Audio buffer cleared"));
}

int32 UEOSVoiceChatSynthComponent::GetBufferSize() const
{
	FScopeLock Lock(const_cast<FCriticalSection*>(&AudioBufferCS));
	return AudioBuffer.Num();
}

void UEOSVoiceChatSynthComponent::SetVoiceVolume(float InVolume)
{
	VolumeMultiplier = FMath::Clamp(InVolume, 0.0f, 1.0f);
	UE_LOG(LogTemp, Log, TEXT("EOSVoiceChatSynthComponent: Volume set to %.2f"), VolumeMultiplier);
}
