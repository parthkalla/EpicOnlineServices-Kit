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
#include "EOSFindLobbiesAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindLobbiesCallback, const TArray<FEOSLobbyInfo>&, Lobbies);

UCLASS()
class EOSKITSESSIONS_API UEOSFindLobbiesAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnFindLobbiesCallback OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnFindLobbiesCallback OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Lobby")
	static UEOSFindLobbiesAsync* FindLobbies(UObject* WorldContextObject);

	virtual void Activate() override;

private:
	static void EOS_CALL OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data);

	UObject* WorldContextObject;
	EOS_HLobbySearch SearchHandle;
};
