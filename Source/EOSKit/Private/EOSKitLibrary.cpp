#include "EOSKitLibrary.h"
#include "Kismet/GameplayStatics.h"

UEOSKitSubsystem* UEOSKitLibrary::GetEOSKitSubsystem(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (GameInstance)
	{
		return GameInstance->GetSubsystem<UEOSKitSubsystem>();
	}
	return nullptr;
}
