// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSAntiCheatBeginSessionAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAntiCheatSessionBegin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAntiCheatSessionFail, const FString&, ErrorMessage);

/**
 * Begin an Anti-Cheat client session
 */
UCLASS()
class EOSKITWEB_API UEOSAntiCheatBeginSessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnAntiCheatSessionBegin OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnAntiCheatSessionFail OnFail;

	/**
	 * Begin an Anti-Cheat client session
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Product User ID of the local player
	 * @param Mode The mode to run Anti-Cheat in (0 = Client, 1 = Server)
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Web|AntiCheat")
	static UEOSAntiCheatBeginSessionAsync* BeginAntiCheatSession(
		UObject* WorldContextObject,
		const FString& LocalUserId,
		int32 Mode = 0
	);

	virtual void Activate() override;

protected:
	void PerformBeginSession();

	UObject* WorldContextObject;
	FString UserId;
	int32 AntiCheatMode;
};
