// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "eos_common.h"
#include "eos_sdk.h"
#include "eos_anticheatclient.h"
#include "EOKSettings.h"
#include "eos_anticheatcommon_types.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOK/SdkFunctions/ConnectInterface/EOK_ConnectSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AntiCheatClient.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSendMessageToServer, const TArray<uint8>&, Message);
UCLASS()
class OnlineSubsystemEOK_API UAntiCheatClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Epic Online Services-Kit V2|AntiCheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsAntiCheatClientAvailable(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|AntiCheat")
	bool RegisterAntiCheatClient(FString ClientProductID);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|AntiCheat")
	bool RecievedMessageFromServer(const TArray<uint8>& Message);

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|AntiCheat", meta = (Keywords = "EOS_AntiCheatClient_EndSession"))
	bool UnregisterAntiCheatClient();
	
	static void EOS_CALL OnMessageToServerCallback(const EOS_AntiCheatClient_OnMessageToServerCallbackInfo* Data);

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2|AntiCheat")
	FSendMessageToServer OnSendMessageToServer;

	void PrintAdvancedLogs(const FString& Log) const
	{
		if(const UEOKSettings* EOKSettings = GetMutableDefault<UEOKSettings>())
		{
			{
				UE_LOG(LogEOK,Warning, TEXT("EOK: %s"), *Log);
			}
		}
	}

	EOS_NotificationId MessageToServerId;
};
