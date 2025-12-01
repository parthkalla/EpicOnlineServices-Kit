// Restored from original EOSKitVoice module
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
	NumChannels = 2;
	SampleRate = 48000;
	CurrentSampleRate = SampleRate;

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

	FScopeLock Lock(&AudioBufferCS);

	int32 SamplesToGenerate = FMath::Min(NumSamples, AudioBuffer.Num());

	if (SamplesToGenerate > 0)
	{
		FMemory::Memcpy(OutAudio, AudioBuffer.GetData(), SamplesToGenerate * sizeof(float));

		if (VolumeMultiplier != 1.0f)
		{
			for (int32 i = 0; i < SamplesToGenerate; ++i)
			{
				OutAudio[i] *= VolumeMultiplier;
			}
		}

		AudioBuffer.RemoveAt(0, SamplesToGenerate, EAllowShrinking::No);

		if (SamplesToGenerate < NumSamples)
		{
			FMemory::Memzero(OutAudio + SamplesToGenerate, (NumSamples - SamplesToGenerate) * sizeof(float));
		}
	}
	else
	{
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

	if (AudioBuffer.Num() >= MaxBufferSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSVoiceChatSynthComponent: Audio buffer overflow, clearing old data"));
		AudioBuffer.RemoveAt(0, AudioBuffer.Num() / 2, EAllowShrinking::No);
	}

	if (SampleRate != CurrentSampleRate)
	{
		float Ratio = static_cast<float>(CurrentSampleRate) / static_cast<float>(SampleRate);
		int32 NewNumSamples = static_cast<int32>(NumSamples * Ratio);

		UE_LOG(LogTemp, Warning, TEXT("EOSVoiceChatSynthComponent: Sample rate mismatch (got %d, expected %d)"),
			SampleRate, CurrentSampleRate);
	}

	if (InNumChannels == 1 && this->NumChannels == 2)
	{
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
		int32 StartIndex = AudioBuffer.Num();
		AudioBuffer.AddUninitialized(NumSamples);
		FMemory::Memcpy(&AudioBuffer[StartIndex], Buffer, NumSamples * sizeof(float));
	}
	else
	{
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


