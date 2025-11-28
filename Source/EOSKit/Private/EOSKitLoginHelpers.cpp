// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitLoginHelpers.h"
#include "EOSKitSubsystem.h"
#include "EOSKitSettings.h"
#include "Kismet/GameplayStatics.h"
#include "eos_common.h"

bool UEOSKitLoginHelpers::IsLoggedInToEOS(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		return false;
	}

	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem)
	{
		return false;
	}

	EOS_ProductUserId ProductUserId = EOSKitSubsystem->GetProductUserId();
	return ProductUserId != nullptr;
}

FString UEOSKitLoginHelpers::GetProductUserIdString(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return TEXT("");
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		return TEXT("");
	}

	UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
	if (!EOSKitSubsystem)
	{
		return TEXT("");
	}

	EOS_ProductUserId ProductUserId = EOSKitSubsystem->GetProductUserId();
	if (!ProductUserId)
	{
		return TEXT("");
	}

	// Convert ProductUserId to string
	char ProductUserIdStr[256];
	int32 ProductUserIdStrSize = sizeof(ProductUserIdStr);
	EOS_ProductUserId_ToString(ProductUserId, ProductUserIdStr, &ProductUserIdStrSize);

	return FString(UTF8_TO_TCHAR(ProductUserIdStr));
}

bool UEOSKitLoginHelpers::CheckLoginStatusWithWarning(UObject* WorldContextObject, const FString& OperationName)
{
	bool bIsLoggedIn = IsLoggedInToEOS(WorldContextObject);

	if (!bIsLoggedIn)
	{
		UE_LOG(LogTemp, Error, TEXT("====================================================="));
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Cannot perform '%s' - User not logged in!"), *OperationName);
		UE_LOG(LogTemp, Error, TEXT("====================================================="));
		UE_LOG(LogTemp, Error, TEXT(""));
		UE_LOG(LogTemp, Error, TEXT("You need to log in to EOS before using session features."));
		UE_LOG(LogTemp, Error, TEXT(""));
		UE_LOG(LogTemp, Error, TEXT("Option 1: Enable Auto-Login"));
		UE_LOG(LogTemp, Error, TEXT("  - Go to: Edit > Project Settings > Plugins > EOS Kit"));
		UE_LOG(LogTemp, Error, TEXT("  - Enable: bEnableAutoLogin"));
		UE_LOG(LogTemp, Error, TEXT("  - Configure Developer credentials"));
		UE_LOG(LogTemp, Error, TEXT(""));
		UE_LOG(LogTemp, Error, TEXT("Option 2: Manual Login in Blueprint/C++"));
		UE_LOG(LogTemp, Error, TEXT("  - Use: EOSLoginAsync node before creating sessions"));
		UE_LOG(LogTemp, Error, TEXT("  - Or call: UEOSLoginUsingAuthInterface::EOSLogin()"));
		UE_LOG(LogTemp, Error, TEXT(""));
		UE_LOG(LogTemp, Error, TEXT("Option 3: Check Login Status"));
		UE_LOG(LogTemp, Error, TEXT("  - Use: IsLoggedInToEOS() to check before operations"));
		UE_LOG(LogTemp, Error, TEXT("  - Use: CheckLoginStatusWithWarning() for detailed feedback"));
		UE_LOG(LogTemp, Error, TEXT(""));
		UE_LOG(LogTemp, Error, TEXT("See: Plugins/EOSKit/Documentation/AutoLogin_Configuration.md"));
		UE_LOG(LogTemp, Error, TEXT("====================================================="));
	}

	return bIsLoggedIn;
}

FString UEOSKitLoginHelpers::GetLoginRequiredMessage()
{
	return TEXT("You must login to EOS before using this feature.\n\n"
				"Options:\n"
				"1. Enable Auto-Login in Project Settings > Plugins > EOS Kit\n"
				"2. Use the 'EOSLoginAsync' node in Blueprint\n"
				"3. Call UEOSLoginUsingAuthInterface::EOSLogin() in C++\n\n"
				"See Documentation: Plugins/EOSKit/Documentation/AutoLogin_Configuration.md");
}

bool UEOSKitLoginHelpers::IsAutoLoginEnabled()
{
	const UEOSKitSettings* Settings = GetDefault<UEOSKitSettings>();
	if (!Settings)
	{
		return false;
	}

	return Settings->bEnableAutoLogin;
}
