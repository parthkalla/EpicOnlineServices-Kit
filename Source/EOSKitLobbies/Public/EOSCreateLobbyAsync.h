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
#include "eos_lobby.h"
#include "EOSCreateLobbyAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateLobbyCallback, const FString&, LobbyId);

UCLASS()
class EOSKITLOBBIES_API UEOSCreateLobbyAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnCreateLobbyCallback OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnCreateLobbyCallback OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Lobby")
	static UEOSCreateLobbyAsync* CreateLobby(UObject* WorldContextObject, int32 MaxPlayers, const FString& BucketId, bool bIsPublic);

	virtual void Activate() override;

private:
	static void EOS_CALL OnCreateLobbyComplete(const EOS_Lobby_CreateLobbyCallbackInfo* Data);

	UObject* WorldContextObject;
	int32 MaxPlayers;
	FString BucketId;
	bool bIsPublic;
};
