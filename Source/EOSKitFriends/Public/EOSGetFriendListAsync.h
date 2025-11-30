// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSKitFriendsTypes.h"
#include "EOSGetFriendListAsync.generated.h"

/**
 * Delegate for friend list query completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSGetFriendListComplete, const TArray<FEOSKitFriendData>&, FriendList, const FString&, Error);

/**
 * Async node to query and retrieve the friends list
 */
UCLASS()
class EOSKITFRIENDS_API UEOSGetFriendListAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSGetFriendListComplete OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "EOSKit|Friends")
	FOnEOSGetFriendListComplete OnFailure;

	/**
	 * Query friends list for the logged-in user
	 * @param WorldContextObject - World context object
	 * @return Async node instance
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get EOS Friend List",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
		ToolTip = "Query and retrieve the friends list for the logged-in user"),
		Category = "EOSKit|Friends")
	static UEOSGetFriendListAsync* GetFriendList(UObject* WorldContextObject);

	virtual void Activate() override;

private:
	static void EOS_CALL OnQueryFriendsComplete(const EOS_Friends_QueryFriendsCallbackInfo* Data);

	UObject* WorldContextObject;
};

