// Copyright (c) 2023 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/Subsystem/EOK_Subsystem.h"
#include "EOK_GetFriendList_AsyncFunction.generated.h"


USTRUCT(BlueprintType)
struct FEOK_FriendData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Friends")
	FString DisplayName = ""; 

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Friends")
	bool bIsOnline = false;
	
	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Friends")
	FString InviteStatus = "";

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Friends")
	FEOKUniqueNetId UserId = FEOKUniqueNetId();

	UPROPERTY(BlueprintReadWrite, Category="Epic Online Services-Kit V2 || Friends")
	FString PresenceStatus = "";
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetFriendListComplete,const TArray<FEOK_FriendData>&, FriendList, FString, Error);

UCLASS()
class OnlineSubsystemEOK_API UEOK_GetFriendList_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName="Get EOK Friend List", meta = (BlueprintInternalUseOnly = "true"), Category="Epic Online Services-Kit V2 || Friends")
	static UEOK_GetFriendList_AsyncFunction* GetFriendList();
	
private:
	void OnFriendsReadComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FOnGetFriendListComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnGetFriendListComplete OnFailure;
	
};
