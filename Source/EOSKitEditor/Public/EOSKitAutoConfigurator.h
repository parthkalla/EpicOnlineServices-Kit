// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EOSKitAutoConfigurator.generated.h"

/**
 * Auto-configuration utility for EOSKit
 * Handles automatic setup of EOS configuration in DefaultEngine.ini
 */
UCLASS()
class EOSKITEDITOR_API UEOSKitAutoConfigurator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Configure the project for EOSKit
	 * This will:
	 * - Write EOS config entries to DefaultEngine.ini
	 * - Configure OnlineSubsystem settings
	 * - Set up platform-specific settings
	 * - Validate credentials
	 * 
	 * @return true if configuration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|AutoConfig")
	static bool ConfigureProject();

	/**
	 * Validate EOS credentials
	 * Checks if Client ID, Client Secret, Product ID, Sandbox ID, and Deployment ID are valid
	 * 
	 * @param ClientId - EOS Client ID
	 * @param ClientSecret - EOS Client Secret
	 * @param ProductId - EOS Product ID
	 * @param SandboxId - EOS Sandbox ID
	 * @param DeploymentId - EOS Deployment ID
	 * @param OutErrorMessage - Error message if validation fails
	 * @return true if credentials are valid
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|AutoConfig")
	static bool ValidateCredentials(
		const FString& ClientId,
		const FString& ClientSecret,
		const FString& ProductId,
		const FString& SandboxId,
		const FString& DeploymentId,
		FString& OutErrorMessage
	);

	/**
	 * Write EOS configuration to DefaultEngine.ini
	 * 
	 * @return true if write was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|AutoConfig")
	static bool WriteEngineConfig();

	/**
	 * Update OnlineSubsystem configuration
	 * Sets DefaultPlatformService to EOSKit
	 * 
	 * @return true if update was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|AutoConfig")
	static bool UpdateOnlineSubsystemConfig();

	/**
	 * Add platform-specific settings for Windows
	 * 
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|AutoConfig")
	static bool AddWindowsPlatformSettings();

	/**
	 * Add platform-specific settings for Android
	 * 
	 * @param SDKPath - Path to Android SDK folder
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|AutoConfig")
	static bool AddAndroidPlatformSettings(const FString& SDKPath);

	/**
	 * Add platform-specific settings for iOS
	 * 
	 * @param SDKPath - Path to iOS SDK folder
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|AutoConfig")
	static bool AddIOSPlatformSettings(const FString& SDKPath);

private:
	/**
	 * Helper to check if a string is valid ANSI (printable, no whitespace)
	 */
	static bool IsValidAnsiString(const FString& String);

	/**
	 * Helper to check if a string is valid hex (64 characters)
	 */
	static bool IsValidHexString(const FString& String, int32 ExpectedLength = 64);
};

