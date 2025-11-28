// Copyright (C) 2024, All Rights Reserved.

#include "EOSFindEOKSessionByIDAsync.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/OnlineSessionNames.h"
#endif

UEOSFindEOKSessionByIDAsync* UEOSFindEOKSessionByIDAsync::FindEOKSessionByID(FString SessionID)
{
	UEOSFindEOKSessionByIDAsync* Ueik_FindSessionObject = NewObject<UEOSFindEOKSessionByIDAsync>();
	Ueik_FindSessionObject->Var_SessionID = SessionID;
	return Ueik_FindSessionObject;
}

void UEOSFindEOKSessionByIDAsync::Activate()
{
	FindSession();
	Super::Activate();
}

void UEOSFindEOKSessionByIDAsync::FindSession()
{
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: FindSessionByID called (standalone - no IOnlineSubsystem)"));
	
	// EOSKit finds sessions directly via EOS SDK
	// Return empty result for now
	FEOSKitSessionFindResult EmptyResult;
	OnFail.Broadcast(EmptyResult);
	
	bDelegateCalled = true;
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEOSFindEOKSessionByIDAsync::OnFindSessionCompleted(int I, bool bWasSuccessful, const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
	// Not used in standalone mode
}
