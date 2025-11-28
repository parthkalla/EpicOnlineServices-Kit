#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EOSKitSubsystem.h"
#include "EOSKitLibrary.generated.h"

UCLASS()
class EOSKIT_API UEOSKitLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "EOSKit", meta = (WorldContext = "WorldContextObject"))
	static UEOSKitSubsystem* GetEOSKitSubsystem(const UObject* WorldContextObject);
};
