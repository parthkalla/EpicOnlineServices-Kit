// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "EOKSettings.h"
#include "Runtime/Online/HTTP/Public/Http.h"

#include "EOK_DeleteTicketData_AsyncFunct.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTicketDeleteComplete, const FString&, ResponseStr, const int32&, HTTP_ResponseCode, bool, bWasSuccessful);

UCLASS()
class OnlineSubsystemEOK_API UEOK_DeleteTicketData_AsyncFunct : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    // Variables
    FString Var_Email;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || Support Tickets")
    FOnTicketDeleteComplete Success;

    UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2 || Support Tickets")
    FOnTicketDeleteComplete Failure;

    // Functions
    void Activate() override;

    UFUNCTION(BlueprintCallable, DisplayName = "Delete EOK Support Ticket Data", meta = (BlueprintInternalUseOnly = "true"), Category = "Epic Online Services-Kit V2 | Support Tickets ")
    static UEOK_DeleteTicketData_AsyncFunct* DeleteEOKTicketDataAsyncFunction(FString Email);

    void DeleteTicketData();
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

