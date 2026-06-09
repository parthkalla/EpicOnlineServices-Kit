// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Runtime/Json/Public/Dom/JsonValue.h"
#include "Misc/Base64.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOK_BaseWebApi.generated.h"

USTRUCT(BlueprintType)
struct FEOK_BaseWebApiResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	int32 StatusCode;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	FString Response;

	FEOK_BaseWebApiResponse()
	{
		StatusCode = -1;
		Response = "";
	}
	FEOK_BaseWebApiResponse(int32 InStatusCode, const FString& InResponse)
	{
		StatusCode = InStatusCode;
		Response = InResponse;
	}

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOK_BaseWebApiDelegate, bool, bSuccess, const FEOK_BaseWebApiResponse&, Response);
UCLASS()
class UEOK_BaseWebApi : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintAssignable, Category = "Epic Online Services-Kit V2|Web")
	FEOK_BaseWebApiDelegate OnResponse;

protected:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void DestroyAsyncTask();
	FString APIEndpoint = "https://api.epicgames.dev";
};
