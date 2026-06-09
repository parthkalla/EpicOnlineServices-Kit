// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_DeleteTicketData_AsyncFunct.h"

#include "EOKSettings.h"

#include "EOKSettings.h"
#include "Runtime/Online/HTTP/Public/Http.h"

void UEOK_DeleteTicketData_AsyncFunct::Activate()
{
    DeleteTicketData();
    Super::Activate();
}

UEOK_DeleteTicketData_AsyncFunct* UEOK_DeleteTicketData_AsyncFunct::DeleteEOKTicketDataAsyncFunction(FString Email)
{
    UEOK_DeleteTicketData_AsyncFunct* UEOK_DeleteEOKTicketData = NewObject<UEOK_DeleteTicketData_AsyncFunct>();

    UEOK_DeleteEOKTicketData->Var_Email = Email;

    return UEOK_DeleteEOKTicketData;
}

void UEOK_DeleteTicketData_AsyncFunct::DeleteTicketData()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL("https://dev.epicgames.com/portal/api/v1/services/tickets/datarights/" + FString(Var_Email));
    Request->SetVerb("DELETE");
    Request->SetHeader("Content-Type", "application/json");

    if (UEOKSettings* EOKSettings = GetMutableDefault<UEOKSettings>())
    {
        if (EOKSettings->ApiKey.IsEmpty())
        {
            // Log an error message
            UE_LOG(LogTemp, Error, TEXT("API key is empty. Cancelling function call."));

            Failure.Broadcast(TEXT("API key is empty."), 0, false);

            return;
        }
        else
        {
            FString APIKey = EOKSettings->ApiKey;
            Request->SetHeader("Authorization", "Token " + FString(APIKey));
        }
    }
    else
    {
        Failure.Broadcast(TEXT("Couldn't retrieve your API key from EOK settings."), 0, false);
        return;
    }

    Request->OnProcessRequestComplete().BindUObject(this, &UEOK_DeleteTicketData_AsyncFunct::OnResponseReceived);

    Request->ProcessRequest();
}

void UEOK_DeleteTicketData_AsyncFunct::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        // Handle a successful delete request here
        int32 HTTP_ResponseCode = Response->GetResponseCode();
        if (HTTP_ResponseCode == 200)
        {
            UE_LOG(LogTemp, Log, TEXT("User data deleted successfully."));
            Success.Broadcast(TEXT("User data deleted successfully."), HTTP_ResponseCode, true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to delete user data. HTTP response code: %d"), HTTP_ResponseCode);
            Failure.Broadcast(TEXT("Failed to delete user data."), HTTP_ResponseCode, false);
        }
    }
    else
    {
        // Handle a failed delete request here
        UE_LOG(LogTemp, Error, TEXT("Failed to delete user data."));
        Failure.Broadcast(TEXT("Failed to delete user data."), 0, false);
    }
}
