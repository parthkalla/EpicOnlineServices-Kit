// Copyright 2023 Betide Studio. All rights reserved.


#include "EVOK_Functions.h"

#include "EOKSettings.h"
#include "Engine/Engine.h"
#include "Interfaces/IHttpResponse.h"


bool UEVOK_Functions::InitializeEOSVoiceChat(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				return LocalVoiceSubsystem->EVOK_Local_Initialize();
			}
		}
	}
	return false;
}

void UEVOK_Functions::ConnectVoiceChat(const UObject* WorldContextObject, const FEOKResultDelegate& Result)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if(LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->Connect(FOnVoiceChatConnectCompleteDelegate::CreateLambda([Result](const FVoiceChatResult& Result1)
					{
						if(Result1.IsSuccess())
						{
							Result.ExecuteIfBound(true, EEVIKResultCodes::Success);
						}
						else
						{
							Result.ExecuteIfBound(false, EEVIKResultCodes::Failed);
						}
					}
					));
				}	
			}
		}
	}
}

bool UEVOK_Functions::IsVoiceChatConnected(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if(LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					return LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->IsConnected();
				}
			}
		}
	}
	return false;
}

void UEVOK_Functions::LoginEOSVoiceChat(const UObject* WorldContextObject, FString PlayerName, const FEOKResultDelegate& Result)
{
	// Remove spaces from PlayerName
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT(""));
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>())
			{
				if(LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
#if ENGINE_MAJOR_VERSION == 5
					const FPlatformUserId PlatformUserId = FPlatformMisc::GetPlatformUserForUserIndex(0);
#else
					const FPlatformUserId PlatformUserId = PLATFORMUSERID_NONE;
#endif
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->Login(PlatformUserId, PlayerName, "", FOnVoiceChatLoginCompleteDelegate::CreateLambda([Result](const FString& PlayerName, const FVoiceChatResult& Result1)
					{
						if(Result1.IsSuccess())
						{
							EOS_ProductUserId ProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*PlayerName));
							if(ProductUserId != nullptr)
							{
								UE_LOG(LogTemp, Log, TEXT("ProductUserId: %p"), ProductUserId);
							}
							UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *PlayerName);
							Result.ExecuteIfBound(true, EEVIKResultCodes::Success);
						}
						else
						{
							Result.ExecuteIfBound(false, EEVIKResultCodes::Failed);
						}
					}
					));
					return;
				}	
			}
		}
	}
	Result.ExecuteIfBound(false, EEVIKResultCodes::Failed);
}

void UEVOK_Functions::LogoutEOSVoiceChat(const UObject* WorldContextObject, FString PlayerName,
	const FEOKResultDelegate& Result)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if(LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->Logout(FOnVoiceChatLogoutCompleteDelegate::CreateLambda([Result](const FString& PlayerName, const FVoiceChatResult& Result1)
					{
						if(Result1.IsSuccess())
						{
							Result.ExecuteIfBound(true, EEVIKResultCodes::Success);
						}
						else
						{
							Result.ExecuteIfBound(false, EEVIKResultCodes::Failed);
						}
					}
					));
				}	
			}
		}
	}
}

bool UEVOK_Functions::IsEOSVoiceChatLoggingIn(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if(LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					return LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->IsLoggingIn();
				}
			}
		}
	}
	return false;

}

bool UEVOK_Functions::IsEOSVoiceChatLoggedIn(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if(LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					return LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->IsLoggedIn();
				}
			}
		}
	}
	return false;
}

FString UEVOK_Functions::LoggedInUser(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if(LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					FString PlayerName = LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->GetLoggedInPlayerName();
					UE_LOG(LogTemp, Warning, TEXT("Playername is %s"), *PlayerName);
					return PlayerName;
				}
			}
		}
	}
	return FString();
}

#include "Misc/ConfigCacheIni.h"
#include "Http.h"

void UEVOK_Functions::EOSRoomToken(FString VoiceRoomName, FString PlayerName, FString ClientIP, const FEOKRoomTokenResultDelegate& Result)
{
    FString ProductId, SandboxId, DeploymentId, ClientId, ClientSecret, EncryptionKey;
	const UEOKSettings* EOKSettings = GetMutableDefault<UEOKSettings>();
	if(!EOKSettings || EOKSettings->VoiceArtifactName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Missing values in EOK Settings"));
		Result.ExecuteIfBound(false, "Error");
		return;
	}

    bool bSuccess = true;
    if (EOKSettings && EOKSettings->VoiceArtifactName.Len() > 0)
	{
		FEOSArtifactSettings ArtifactSettingsForVoice;
    	EOKSettings->GetSettingsForArtifact(EOKSettings->VoiceArtifactName, ArtifactSettingsForVoice);
    	ProductId = ArtifactSettingsForVoice.ProductId;
    	SandboxId = ArtifactSettingsForVoice.SandboxId;
    	DeploymentId = ArtifactSettingsForVoice.DeploymentId;
    	ClientId = ArtifactSettingsForVoice.ClientId;
    	ClientSecret = ArtifactSettingsForVoice.ClientSecret;
    	EncryptionKey = ArtifactSettingsForVoice.EncryptionKey;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Missing values in EOK Settings"));
		Result.ExecuteIfBound(false, "Error");
		return;
	}
	if(ProductId.IsEmpty() || SandboxId.IsEmpty() || DeploymentId.IsEmpty() || ClientId.IsEmpty() || ClientSecret.IsEmpty() || EncryptionKey.IsEmpty())
	{
		bSuccess = false;
	}

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing values for Artifact Settings"));
    	Result.ExecuteIfBound(false, "Error");
        return;
    }

    auto HttpRequest = FHttpModule::Get().CreateRequest();
    const FString Base64Credentials = FBase64::Encode(FString::Printf(TEXT("%s:%s"), *ClientId, *ClientSecret));
    const FString ContentString = FString::Printf(TEXT("grant_type=client_credentials&deployment_id=%s"), *DeploymentId);

    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
    HttpRequest->AppendToHeader(TEXT("Accept"), TEXT("application/json"));
    HttpRequest->AppendToHeader(TEXT("Authorization"), *FString::Printf(TEXT("Basic %s"), *Base64Credentials));
    HttpRequest->SetContentAsString(ContentString);
    HttpRequest->SetURL("https://api.epicgames.dev/auth/v1/oauth/token");
    HttpRequest->SetVerb("POST");

    HttpRequest->OnProcessRequestComplete().BindLambda([=](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bConnectedSuccessfully)
    {
    	UE_LOG(LogTemp, Warning, TEXT("Response -> %s"), *ResponsePtr->GetContentAsString());
        FString AccessTokenString;
        TSharedPtr<FJsonObject> JsonObject;

        if (bConnectedSuccessfully && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(ResponsePtr->GetContentAsString()), JsonObject) && JsonObject.IsValid())
        {
            TSharedPtr<FJsonValue> AccessTokenObject = JsonObject->TryGetField(TEXT("access_token"));
            AccessTokenString = AccessTokenObject ? AccessTokenObject->AsString() : FString();
        }
        else
        {
        	Result.ExecuteIfBound(false, "Error");
        	return;
        }

		if (!AccessTokenString.IsEmpty())
		{
			auto RoomTokenRequest = FHttpModule::Get().CreateRequest();

			RoomTokenRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
			RoomTokenRequest->AppendToHeader(TEXT("Accept"), TEXT("application/json"));
			RoomTokenRequest->AppendToHeader(TEXT("Authorization"), *FString::Printf(TEXT("Bearer %s"), *AccessTokenString));

			const FString ProductUserId = PlayerName;
			bool bHardMuted = false; 

			const FString JsonRequestString = FString::Printf(TEXT("{\"participants\":[{\"puid\":\"%s\",\"clientIp\":\"%s\",\"hardMuted\":%s}] }"), *ProductUserId, *ClientIP, bHardMuted ? TEXT("true") : TEXT("false"));
			RoomTokenRequest->SetContentAsString(JsonRequestString);
			RoomTokenRequest->SetURL(FString::Printf(TEXT("https://api.epicgames.dev/rtc/v1/%s/room/%s"), *DeploymentId, *VoiceRoomName));
			RoomTokenRequest->SetVerb("POST");

			RoomTokenRequest->OnProcessRequestComplete().BindLambda([=](FHttpRequestPtr HttpRequestPtr, FHttpResponsePtr HttpResponsePtr, bool bConnectedSuccessfully)
			{
				UE_LOG(LogTemp, Warning, TEXT("Response -> %s"), *HttpResponsePtr->GetContentAsString());
				FString TokenString, ClientBaseUrlString;

				if (bConnectedSuccessfully)
				{
					TSharedPtr<FJsonObject> JsonObject;
					const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HttpResponsePtr->GetContentAsString());
					if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
					{
						const TSharedPtr<FJsonValue> ClientBaseUrlObject = JsonObject->TryGetField(TEXT("clientBaseUrl"));
						const TSharedPtr<FJsonValue> ParticipantsObject = JsonObject->TryGetField(TEXT("participants"));

						if (ParticipantsObject)
						{
							ClientBaseUrlString = ClientBaseUrlObject->AsString();
							UE_LOG(LogTemp, Warning, TEXT("ClientUrl -> %s"), *ClientBaseUrlString);
							TArray<TSharedPtr<FJsonValue>> ParticipantsArray = ParticipantsObject->AsArray();

							for (const auto& Element : ParticipantsArray)
							{
								if (Element->Type != EJson::Object)
									continue;

								auto& Object = Element->AsObject();
								TokenString = Object->TryGetField(TEXT("token"))->AsString();
								UE_LOG(LogTemp, Warning, TEXT("Token -> %s"), *TokenString);
							}
						}
					}
					else
					{
						Result.ExecuteIfBound(false, "Error");
						return;
					}
				}
				else
				{
					Result.ExecuteIfBound(false, "Error");
					return;
				}

				if (!TokenString.IsEmpty())
				{
					FEVIKChannelCredentials ChannelCredentials;
					ChannelCredentials.OverrideUserId = PlayerName;
					ChannelCredentials.ClientBaseUrl = ClientBaseUrlString;
					ChannelCredentials.ParticipantToken = TokenString;
					Result.ExecuteIfBound(true, ChannelCredentials.ToJson(false));
					return;
				}
				else
				{
					Result.ExecuteIfBound(false, "Error");
				}
			});

			RoomTokenRequest->ProcessRequest();
		}
		else
		{
			Result.ExecuteIfBound(false, "Error");
		}
    });

    HttpRequest->ProcessRequest();
}

void UEVOK_Functions::JoinEOSRoom(const UObject* WorldContextObject, FString VoiceRoomName, FString ChannelCredentialsJson,bool bEnableEcho, const FEOKResultDelegate& Result)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
        {
            UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
            if (LocalVoiceSubsystem)
            {
                if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
                {
                    FVoiceChatChannel3dProperties Properties;
                    Properties.AttenuationModel = EVoiceChatAttenuationModel::InverseByDistance;
                    Properties.MaxDistance = 100.f;
                    Properties.MinDistance = 1.f;
                    Properties.Rolloff = 0.5f;

                    TSharedPtr<FJsonObject> ChannelCredentialsObject;
                    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ChannelCredentialsJson);
                    if (FJsonSerializer::Deserialize(JsonReader, ChannelCredentialsObject) && ChannelCredentialsObject.IsValid())
                    {
                        FString ClientBaseUrl = ChannelCredentialsObject->GetStringField(TEXT("ClientBaseUrl"));
                        FString ParticipantToken = ChannelCredentialsObject->GetStringField(TEXT("ParticipantToken"));
                    	EVoiceChatChannelType ChannelType = EVoiceChatChannelType::NonPositional;
                    	if(bEnableEcho)
                    	{
                    		ChannelType = EVoiceChatChannelType::Echo;
                    		UE_LOG(LogTemp, Warning, TEXT("Setting Echo -> %hhd"), bEnableEcho);
                    	}
                    	UE_LOG(LogTemp, Warning, TEXT("bEnableEcho -> %hhd"), bEnableEcho);

                        LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->JoinChannel(VoiceRoomName, ChannelCredentialsJson, ChannelType, FOnVoiceChatChannelJoinCompleteDelegate::CreateLambda([Result](const FString& ChannelName, const FVoiceChatResult& JoinResult)
                        {
                        	if(JoinResult.IsSuccess())
                        	{
								Result.ExecuteIfBound(true,EEVIKResultCodes::Success);
							}
							else
							{
								Result.ExecuteIfBound(false,EEVIKResultCodes::Failed);
							}
                        }), Properties);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to parse ChannelCredentialsJson"));
                    }
                }
            }
        }
    }
}

void UEVOK_Functions::LeaveEOSRoom(const UObject* WorldContextObject, FString VoiceRoomName,
	const FEOKResultDelegate& Result)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->LeaveChannel(VoiceRoomName,FOnVoiceChatChannelLeaveCompleteDelegate::CreateLambda([Result](const FString& ChannelName, const FVoiceChatResult& JoinResult)
						{
							if(JoinResult.IsSuccess())
							{
								Result.ExecuteIfBound(true,EEVIKResultCodes::Success);
							}
							else
							{
								Result.ExecuteIfBound(false,EEVIKResultCodes::Failed);
							}
						}));
				}
			}
		}
	}
}

TArray<FString> UEVOK_Functions::GetPlayersInRoom(const UObject* WorldContextObject, FString VoiceRoomName)
{
	TArray<FString> Local_PlayersInRoom;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					Local_PlayersInRoom = LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->GetPlayersInChannel(VoiceRoomName);
				}
			}
		}
	}
	return Local_PlayersInRoom;
}

TArray<FString> UEVOK_Functions::GetAllRooms(const UObject* WorldContextObject)
{
	TArray<FString> Local_PlayersJoinedRooms;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					Local_PlayersJoinedRooms = LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->GetChannels();
				}
			}
		}
	}
	return Local_PlayersJoinedRooms;
}

float UEVOK_Functions::GetPlayerVolume(const UObject* WorldContextObject, const FString& PlayerName)
{
	float Local_PlayerVolume = 0;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					Local_PlayerVolume = LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->GetPlayerVolume(PlayerName);
				}
			}
		}
	}
	return Local_PlayerVolume;
}

char* UEVOK_Functions::GetProductUserID(const FString& PlayerName)
{
	EOS_ProductUserId ProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*PlayerName));
	if(ProductUserId != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProductUserId: %p"), ProductUserId);
	}
	char* Local_ProductUserID = nullptr;
	int32_t myInt = 128;
	EOS_ProductUserId_ToString(ProductUserId, Local_ProductUserID,&myInt);

	return Local_ProductUserID;
}

bool UEVOK_Functions::SetPlayerVolume(const UObject* WorldContextObject, const FString& PlayerName,
                                      float NewVolume)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>())
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->SetPlayerVolume(PlayerName, NewVolume);
					return true;
				}
			}
		}
	}
	return false;
}

bool UEVOK_Functions::IsPlayerMuted(const UObject* WorldContextObject, const FString& PlayerName)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					return LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->IsPlayerMuted(PlayerName);
				}
			}
		}
	}
	return false;
}

bool UEVOK_Functions::SetPlayerMuted(const UObject* WorldContextObject, const FString& PlayerName, bool MutePlayer)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->SetPlayerMuted(PlayerName, MutePlayer);
					return true;
				}
			}
		}
	}
	return false;
}

bool UEVOK_Functions::TransmitToAllRooms(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->TransmitToAllChannels();
					return true;
				}
			}
		}
	}
	return false;
}

bool UEVOK_Functions::TransmitToSelectedRoom(const UObject* WorldContextObject, FString RoomName)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					const TSet<FString> Var_RoomName = {RoomName};
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >=3
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->TransmitToSpecificChannels(Var_RoomName);
#else
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->TransmitToSpecificChannel(RoomName);
#endif
					return true;
				}
			}
		}
	}
	return false;
}

bool UEVOK_Functions::TransmitToNoRoom(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->TransmitToNoChannels();
					return true;
				}
			}
		}
	}
	return false;
}

TArray<FDeviceEVIKSettings> UEVOK_Functions::GetInputMethods(const UObject* WorldContextObject)
{
	TArray<FDeviceEVIKSettings> Local_DeviceSettingArray;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					TArray<FVoiceChatDeviceInfo> AvailableMethods = LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->GetAvailableInputDeviceInfos();
					for(int i=0;i<AvailableMethods.Num();i++)
					{
						FDeviceEVIKSettings DeviceEvikSettings;
						DeviceEvikSettings.DisplayName = AvailableMethods[i].DisplayName;
						DeviceEvikSettings.ID = AvailableMethods[i].Id;
						Local_DeviceSettingArray.Add(DeviceEvikSettings);
					}
					return Local_DeviceSettingArray;
				}
			}
		}
	}
	return Local_DeviceSettingArray;
}

TArray<FDeviceEVIKSettings> UEVOK_Functions::GetOutputMethods(const UObject* WorldContextObject)
{
	TArray<FDeviceEVIKSettings> Local_DeviceSettingArray;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					TArray<FVoiceChatDeviceInfo> AvailableMethods = LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->GetAvailableOutputDeviceInfos();
					for(int i=0;i<AvailableMethods.Num();i++)
					{
						FDeviceEVIKSettings DeviceEvikSettings;
						DeviceEvikSettings.DisplayName = AvailableMethods[i].DisplayName;
						DeviceEvikSettings.ID = AvailableMethods[i].Id;
						Local_DeviceSettingArray.Add(DeviceEvikSettings);
					}
					return Local_DeviceSettingArray;
				}
			}
		}
	}
	return Local_DeviceSettingArray;
}

bool UEVOK_Functions::SetOutputMethods(const UObject* WorldContextObject, FString MethodID)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->SetOutputDeviceId(MethodID);
					return true;
				}
			}
		}
	}
	return false;
}

bool UEVOK_Functions::SetInputMethods(const UObject* WorldContextObject, FString MethodID)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->SetInputDeviceId(MethodID);
					return true;
				}
			}
		}
	}
	return false;
}

bool UEVOK_Functions::IsPlayerTalking(const UObject* WorldContextObject, FString PlayerName)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					return LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->IsPlayerTalking(PlayerName);
					
				}
			}
		}
	}
	return false;
}

void UEVOK_Functions::MuteInputDevice(const UObject* WorldContextObject, bool Mute, bool &bWasSuccess)
{
	bWasSuccess = false;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (World)
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			UEOK_Voice_Subsystem* LocalVoiceSubsystem = GameInstance->GetSubsystem<UEOK_Voice_Subsystem>();
			if (LocalVoiceSubsystem)
			{
				if (LocalVoiceSubsystem->EVOK_Local_GetVoiceChat())
				{
					LocalVoiceSubsystem->EVOK_Local_GetVoiceChat()->SetAudioInputDeviceMuted(Mute);
					bWasSuccess = true;
					return;
				}
			}
		}
	}
	return;
}
