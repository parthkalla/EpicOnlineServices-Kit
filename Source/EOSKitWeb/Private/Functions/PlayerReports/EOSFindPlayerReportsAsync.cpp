// Copyright (C) 2024, All Rights Reserved.

#include "Functions/PlayerReports/EOSFindPlayerReportsAsync.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

UEOSFindPlayerReportsAsync* UEOSFindPlayerReportsAsync::FindPlayerReports(
	const FString& Authorization,
	const FString& DeploymentId,
	const FString& ReportingPlayerId,
	const FString& ReportedPlayerId,
	int32 ReasonId,
	const FString& StartTime,
	const FString& EndTime,
	bool bPagination,
	int32 Offset,
	int32 Limit,
	const FString& Order)
{
	UEOSFindPlayerReportsAsync* Node = NewObject<UEOSFindPlayerReportsAsync>();
	Node->Var_Authorization = Authorization;
	Node->Var_DeploymentId = DeploymentId;
	Node->Var_ReportingPlayerId = ReportingPlayerId;
	Node->Var_ReportedPlayerId = ReportedPlayerId;
	Node->Var_ReasonId = ReasonId;
	Node->Var_StartTime = StartTime;
	Node->Var_EndTime = EndTime;
	Node->Var_bPagination = bPagination;
	Node->Var_Offset = Offset;
	Node->Var_Limit = Limit;
	Node->Var_Order = Order;
	return Node;
}

void UEOSFindPlayerReportsAsync::Activate()
{
	Super::Activate();

	FString URL = FString::Printf(TEXT("%s/player-reports/v1/report/%s"), *APIEndpoint, *Var_DeploymentId);
	FString QueryString = TEXT("?");
	bool bFirstParam = true;

	if (!Var_ReportingPlayerId.IsEmpty())
	{
		QueryString += FString::Printf(TEXT("reportingPlayerId=%s"), *Var_ReportingPlayerId);
		bFirstParam = false;
	}

	if (!Var_ReportedPlayerId.IsEmpty())
	{
		if (!bFirstParam) QueryString += TEXT("&");
		QueryString += FString::Printf(TEXT("reportedPlayerId=%s"), *Var_ReportedPlayerId);
		bFirstParam = false;
	}

	if (Var_ReasonId > 0)
	{
		if (!bFirstParam) QueryString += TEXT("&");
		QueryString += FString::Printf(TEXT("reasonId=%d"), Var_ReasonId);
		bFirstParam = false;
	}

	if (!Var_StartTime.IsEmpty())
	{
		if (!bFirstParam) QueryString += TEXT("&");
		QueryString += FString::Printf(TEXT("startTime=%s"), *Var_StartTime);
		bFirstParam = false;
	}

	if (!Var_EndTime.IsEmpty())
	{
		if (!bFirstParam) QueryString += TEXT("&");
		QueryString += FString::Printf(TEXT("endTime=%s"), *Var_EndTime);
		bFirstParam = false;
	}

	if (Var_bPagination)
	{
		if (!bFirstParam) QueryString += TEXT("&");
		QueryString += FString::Printf(TEXT("offset=%d&limit=%d&order=%s"), Var_Offset, Var_Limit, *Var_Order);
	}

	URL += QueryString;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(URL);
	
	FString AuthHeader = Var_Authorization.Contains(TEXT("Bearer")) 
		? Var_Authorization 
		: FString::Printf(TEXT("Bearer %s"), *Var_Authorization);
	HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UEOSFindPlayerReportsAsync::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

