// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EOK_BaseWebApi.h"
#include "Runtime/Launch/Resources/Version.h"


void UEOK_BaseWebApi::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		OnResponse.Broadcast(false, FEOK_BaseWebApiResponse(-1, "Request failed"));
		DestroyAsyncTask();
		return;
	}
	if(!Response.IsValid())
	{
		OnResponse.Broadcast(false, FEOK_BaseWebApiResponse(-1, "Response is invalid"));
		DestroyAsyncTask();
		return;
	}
	OnResponse.Broadcast(true, FEOK_BaseWebApiResponse(Response->GetResponseCode(), Response->GetContentAsString()));
	DestroyAsyncTask();
}

void UEOK_BaseWebApi::DestroyAsyncTask()
{
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}
