// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEOK/SdkFunctions/EOK_SharedFunctionFile.h"
#include "EOK_CreateParty.generated.h"

USTRUCT(BlueprintType)
struct FEOK_PartyExtraSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || Party")
	FName SessionName = "PartySession";

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || Party")
	int32 MaxPrivatePartyMembers = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || Party")
	bool bUsePresence = true;

	UPROPERTY(BlueprintReadWrite, Category = "Epic Online Services-Kit V2 || Party")
	bool bUseVoiceChat = true;
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEOK_CreatePartyComplete);

UCLASS()
class OnlineSubsystemEOK_API UEOK_CreateParty : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Epic Online Services-Kit V2 || Party", DisplayName="Create EOK Party")
	static UEOK_CreateParty* EOK_CreateParty(int32 MaxPublicPartyMembers, FEOK_PartyExtraSettings ExtraPartySettings);

	UPROPERTY(BlueprintAssignable)
	FEOK_CreatePartyComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FEOK_CreatePartyComplete OnFailure;
	
private:
	FEOK_PartyExtraSettings Var_CreatePartySettings;
	int32 Var_MaxPublicPartyMembers;
	virtual void Activate() override;
	void OnCreatePartyCompleted(FName SessionName, bool bWasSuccessful);
};
