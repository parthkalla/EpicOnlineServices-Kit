// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSQuerySanctionsAsync.generated.h"

USTRUCT(BlueprintType)
struct FEOSSanctionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Sanctions")
	FString ReferenceId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Sanctions")
	FString Action;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Sanctions")
	FDateTime TimePlaced;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Sanctions")
	FDateTime TimeExpires;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Sanctions")
	bool bIsPermanent;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuerySanctionsComplete, const TArray<FEOSSanctionInfo>&, Sanctions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuerySanctionsFail, const FString&, ErrorMessage);

/**
 * Query active sanctions/bans for a player
 */
UCLASS()
class EOSKITWEB_API UEOSQuerySanctionsAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnQuerySanctionsComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnQuerySanctionsFail OnFail;

	/**
	 * Query active player sanctions (bans) from EOS
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Product User ID of the player to check for sanctions
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|Web|Sanctions")
	static UEOSQuerySanctionsAsync* QuerySanctions(
		UObject* WorldContextObject,
		const FString& LocalUserId
	);

	virtual void Activate() override;

private:
	void PerformQuery();

	UObject* WorldContextObject;
	FString UserId;
};
