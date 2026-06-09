// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EOK_BaseWebApi.h"
#include "EOK_API_UpdatingSanctions.generated.h"

USTRUCT(BlueprintType)
struct FEOK_UpdatableFields
{
	GENERATED_BODY()

	//List of tags associated with this sanction. Items are case insensitive and unique. Item format: [a-zA-Z0-9_-]+. Item max length: 16
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	TArray<FString> Tags;

	//Arbitrary metadata key/value pairs associated with this sanction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	TMap<FString, FString> Metadata;

	//Justification string associated with this sanction. Min length: 1 Max length: 2048
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	FString Justification;
	
};

USTRUCT(BlueprintType)
struct FEOK_SanctionPatchPayload
{
	GENERATED_BODY()

	//Unique identifier for this sanction	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	FString ReferenceId;

	//Fields to be updated and their new values.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Epic Online Services-Kit V2|Web")
	TArray<FEOK_UpdatableFields> Updates;
};
UCLASS()
class EOKWeb_API UEOK_API_UpdatingSanctions : public UEOK_BaseWebApi
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2|Web")
	static UEOK_API_UpdatingSanctions* UpdatingSanctions(FString Authorization, FString DeploymentId, TArray<FEOK_SanctionPatchPayload> SanctionPatchPayload);

private:
	virtual void Activate() override;
	FString Var_Authorization;
	FString Var_DeploymentId;
	TArray<FEOK_SanctionPatchPayload> Var_SanctionPatchPayload;
};
