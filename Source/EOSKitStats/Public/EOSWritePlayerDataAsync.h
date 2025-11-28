// Copyright (C) 2024, All Rights Reserved.
//
// This file is part of the EOSKit Plugin.
//
// EOSKit is free software: you can redistribute it and/or modify
// it under the terms of the MIT License as published by the Open
// Source Initiative.
//
// EOSKit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// MIT License for more details.
//
// You should have received a copy of the MIT License along with
// EOSKit. If not, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitTypes.h"
#include "eos_playerdatastorage.h"
#include "EOSWritePlayerDataAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWritePlayerDataCallback, const FString&, ErrorMessage);

UCLASS()
class EOSKITSTATS_API UEOSWritePlayerDataAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnWritePlayerDataCallback OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnWritePlayerDataCallback OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|PlayerData")
	static UEOSWritePlayerDataAsync* WritePlayerData(UObject* WorldContextObject, const FString& FileName, const TArray<uint8>& Data);

	virtual void Activate() override;

private:
	static void EOS_CALL OnWriteFileCompleteCallback(const EOS_PlayerDataStorage_WriteFileCallbackInfo* Data);
	static EOS_PlayerDataStorage_EWriteResult EOS_CALL OnWriteFileDataCallback(const EOS_PlayerDataStorage_WriteFileDataCallbackInfo* Data, void* OutDataBuffer, uint32_t* OutDataWritten);
	static void EOS_CALL OnFileTransferProgressCallback(const EOS_PlayerDataStorage_FileTransferProgressCallbackInfo* Data);

	UObject* WorldContextObject;
	FString FileName;
	TArray<uint8> Data;
	uint32_t BytesWritten;
};
