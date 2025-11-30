// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitBaseWebApi.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UEOSKitBaseWebApi::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		OnResponse.Broadcast(false, FEOSKitWebApiResponse(-1, TEXT("Request failed")));
		DestroyAsyncTask();
		return;
	}

	if (!Response.IsValid())
	{
		OnResponse.Broadcast(false, FEOSKitWebApiResponse(-1, TEXT("Response is invalid")));
		DestroyAsyncTask();
		return;
	}

	OnResponse.Broadcast(true, FEOSKitWebApiResponse(Response->GetResponseCode(), Response->GetContentAsString()));
	DestroyAsyncTask();
}

void UEOSKitBaseWebApi::DestroyAsyncTask()
{
	SetReadyToDestroy();
}

