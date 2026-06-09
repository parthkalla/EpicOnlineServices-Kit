// Copyright 2023 Betide Studio. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoiceChat.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializerMacros.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Misc/Base64.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EOK_Voice_Subsystem.generated.h"

UENUM(BlueprintType)
enum class EEVIKResultCodes :uint8
{
	Success = 0,
	Failed = 1
};

struct FEVIKChannelCredentials : public FJsonSerializable
{
	FString OverrideUserId;
	FString ClientBaseUrl;
	FString ParticipantToken;

	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE("override_userid", OverrideUserId);
	JSON_SERIALIZE("client_base_url", ClientBaseUrl);
	JSON_SERIALIZE("participant_token", ParticipantToken);
	END_JSON_SERIALIZER
};

USTRUCT(BlueprintType)
struct FDeviceEVIKSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Epic Online Services-Kit V2|Voice Chat")
	FString ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Epic Online Services-Kit V2|Voice Chat")
	FString DisplayName;
	
};

USTRUCT(BlueprintType)
struct FEVIKPlayerList
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Epic Online Services-Kit V2|Voice Chat")
	FString PlayerEOSVoiceChatName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Epic Online Services-Kit V2|Voice Chat")
	AActor* PlayerActor = nullptr;
	
};


USTRUCT(BlueprintType)
struct FEOK_PositionalVoiceChat
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Epic Online Services-Kit V2|Voice Chat")
	FString ChannelName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Epic Online Services-Kit V2|Voice Chat")
	float MaxHearingDistance = 1000.0f;	
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOKResultDelegate, bool, bWasSuccess, EEVIKResultCodes, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEOKRoomTokenResultDelegate, bool, bWasSuccess, FString, RoomData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_OnPlayerAdded, FString, ChannelName, FString, PlayerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_OnPlayerRemoved, FString, ChannelName, FString, PlayerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_OnChannelExited, FString, ChannelName, FString, Result);
/**
 * 
 */
UCLASS()
class EOKVoiceChat_API UEOK_Voice_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(Category="Epic Online Services-Kit V2|Voice Chat")
	bool EVOK_Local_Initialize();
	
	IVoiceChat* EVOK_Local_GetVoiceChat();

	UFUNCTION(Category="Epic Online Services-Kit V2|Voice Chat")
	void EVOK_Local_Connect(const FEOKResultDelegate& ResultDelegate);

	FTimerHandle UpdatePositionalVoiceChatTimerHandle;

	UFUNCTION()
	void PlayerListUpdated();
	
	IVoiceChat* EVOK_VoiceChat;

	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Voice Chat")
	FEOK_OnPlayerRemoved OnPlayerRemoved;

	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Voice Chat")
	FEOK_OnPlayerAdded OnPlayerAdded;

	UPROPERTY(BlueprintAssignable, Category="Epic Online Services-Kit V2|Voice Chat")
	FEOK_OnChannelExited OnChannelExited;

	float OutputVolume = 1.0f;
	bool bUseOutputVolume = false;
	bool bUseOutputVolumeWithPositionalChat = false;
};
