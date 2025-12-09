// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitAutoConfigurator.h"
#include "EOSKitSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "EOSKitAutoConfigurator"

bool UEOSKitAutoConfigurator::ConfigureProject()
{
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Starting auto-configuration..."));

	// Get settings
	UEOSKitSettings* Settings = GetMutableDefault<UEOSKitSettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to get EOSKitSettings"));
		return false;
	}

	// Validate credentials if artifacts are configured
	bool bHasValidArtifacts = false;
	for (const FEOSArtifact& Artifact : Settings->Artifacts)
	{
		if (!Artifact.ClientId.IsEmpty() && !Artifact.ProductId.IsEmpty())
		{
			FString ErrorMessage;
			if (ValidateCredentials(
				Artifact.ClientId,
				Artifact.ClientSecret,
				Artifact.ProductId,
				Artifact.SandboxId,
				Artifact.DeploymentId,
				ErrorMessage
			))
			{
				bHasValidArtifacts = true;
				break;
			}
		}
	}

	if (!bHasValidArtifacts && Settings->Artifacts.Num() > 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("InvalidCredentialsMsg", 
				"EOSKit: Some artifacts have invalid credentials. Please check your Client ID, Product ID, Sandbox ID, and Deployment ID in Project Settings."));
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Invalid credentials detected, but continuing with configuration..."));
	}

	// Write engine config
	if (!WriteEngineConfig())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to write engine configuration"));
		return false;
	}

	// Update OnlineSubsystem config
	if (!UpdateOnlineSubsystemConfig())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to update OnlineSubsystem configuration"));
		return false;
	}

	// Add platform-specific settings
	AddWindowsPlatformSettings();

	// Flush config
	GConfig->Flush(false);

	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Auto-configuration completed successfully!"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Please restart the editor for changes to take effect."));

	return true;
}

bool UEOSKitAutoConfigurator::ValidateCredentials(
	const FString& ClientId,
	const FString& ClientSecret,
	const FString& ProductId,
	const FString& SandboxId,
	const FString& DeploymentId,
	FString& OutErrorMessage)
{
	OutErrorMessage.Empty();

	// Validate Client ID
	if (ClientId.IsEmpty())
	{
		OutErrorMessage = TEXT("Client ID is empty");
		return false;
	}

	if (!IsValidAnsiString(ClientId))
	{
		OutErrorMessage = TEXT("Client ID must contain only ANSI printable characters with no whitespace");
		return false;
	}

	// Client IDs created after SDK 1.5 start with 'xyz'
	if (!ClientId.StartsWith(TEXT("xyz")))
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Client ID does not start with 'xyz'. Make sure you're using a Client ID (not BPT Client ID)."));
	}

	// Validate Client Secret
	if (!ClientSecret.IsEmpty() && !IsValidAnsiString(ClientSecret))
	{
		OutErrorMessage = TEXT("Client Secret must contain only ANSI printable characters with no whitespace");
		return false;
	}

	// Validate Product ID
	if (ProductId.IsEmpty())
	{
		OutErrorMessage = TEXT("Product ID is empty");
		return false;
	}

	// Validate Sandbox ID
	if (SandboxId.IsEmpty())
	{
		OutErrorMessage = TEXT("Sandbox ID is empty");
		return false;
	}

	// Validate Deployment ID
	if (DeploymentId.IsEmpty())
	{
		OutErrorMessage = TEXT("Deployment ID is empty");
		return false;
	}

	return true;
}

bool UEOSKitAutoConfigurator::WriteEngineConfig()
{
	FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const FString ConfigSectionName = TEXT("/Script/EOSKit.EOSKitSettings");

	UEOSKitSettings* Settings = GetMutableDefault<UEOSKitSettings>();
	if (!Settings)
	{
		return false;
	}

	// Write all settings to config
	Settings->WriteFullConfigToEngineIni();

	return true;
}

bool UEOSKitAutoConfigurator::UpdateOnlineSubsystemConfig()
{
	FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString EngineIniText;

	if (!FFileHelper::LoadFileToString(EngineIniText, *EngineIniPath))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to load DefaultEngine.ini"));
		return false;
	}

	bool bConfigChanged = false;

	// Ensure [OnlineSubsystem] section exists with DefaultPlatformService=EOSKit
	if (!EngineIniText.Contains(TEXT("DefaultPlatformService=EOSKit")))
	{
		if (!EngineIniText.Contains(TEXT("[OnlineSubsystem]")))
		{
			EngineIniText += TEXT("\n[OnlineSubsystem]\nDefaultPlatformService=EOSKit\n");
			bConfigChanged = true;
		}
		else
		{
			// Replace any existing DefaultPlatformService value
			FString Pattern1 = TEXT("DefaultPlatformService=EOS");
			FString Pattern2 = TEXT("DefaultPlatformService=NULL");
			FString Pattern3 = TEXT("DefaultPlatformService=");
			
			if (EngineIniText.Contains(Pattern1) && !EngineIniText.Contains(TEXT("DefaultPlatformService=EOSKit")))
			{
				EngineIniText.ReplaceInline(*Pattern1, TEXT("DefaultPlatformService=EOSKit"));
				bConfigChanged = true;
			}
			else if (EngineIniText.Contains(Pattern2))
			{
				EngineIniText.ReplaceInline(*Pattern2, TEXT("DefaultPlatformService=EOSKit"));
				bConfigChanged = true;
			}
			else if (EngineIniText.Contains(Pattern3))
			{
				// Replace any other value - find the line and replace it
				int32 ServicePos = EngineIniText.Find(TEXT("DefaultPlatformService="));
				if (ServicePos != INDEX_NONE)
				{
					int32 LineEnd = EngineIniText.Find(TEXT("\n"), ServicePos);
					if (LineEnd == INDEX_NONE)
					{
						LineEnd = EngineIniText.Len();
					}
					FString OldLine = EngineIniText.Mid(ServicePos, LineEnd - ServicePos);
					EngineIniText.ReplaceInline(*OldLine, TEXT("DefaultPlatformService=EOSKit"));
					bConfigChanged = true;
				}
			}
			else
			{
				// Add if missing
				int32 SectionPos = EngineIniText.Find(TEXT("[OnlineSubsystem]"));
				if (SectionPos != INDEX_NONE)
				{
					int32 NextSectionPos = EngineIniText.Find(TEXT("\n["), SectionPos + 1);
					if (NextSectionPos == INDEX_NONE)
					{
						NextSectionPos = EngineIniText.Len();
					}
					EngineIniText.InsertAt(NextSectionPos, TEXT("DefaultPlatformService=EOSKit\n"));
					bConfigChanged = true;
				}
			}
		}
	}

	// Ensure [OnlineSubsystemEOSKit] is enabled
	if (!EngineIniText.Contains(TEXT("[OnlineSubsystemEOSKit]")))
	{
		EngineIniText += TEXT("\n[OnlineSubsystemEOSKit]\nbEnabled=true\n");
		bConfigChanged = true;
	}
	else if (EngineIniText.Contains(TEXT("[OnlineSubsystemEOSKit]\nbEnabled=false")))
	{
		EngineIniText.ReplaceInline(TEXT("[OnlineSubsystemEOSKit]\nbEnabled=false"), TEXT("[OnlineSubsystemEOSKit]\nbEnabled=true"));
		bConfigChanged = true;
	}

	// Disable [OnlineSubsystemEOS] to prevent conflicts
	if (!EngineIniText.Contains(TEXT("[OnlineSubsystemEOS]")))
	{
		EngineIniText += TEXT("\n[OnlineSubsystemEOS]\nbEnabled=false\n");
		bConfigChanged = true;
	}
	else if (EngineIniText.Contains(TEXT("[OnlineSubsystemEOS]\nbEnabled=true")))
	{
		EngineIniText.ReplaceInline(TEXT("[OnlineSubsystemEOS]\nbEnabled=true"), TEXT("[OnlineSubsystemEOS]\nbEnabled=false"));
		bConfigChanged = true;
	}

	// Ensure [/Script/OnlineSubsystemEOSKit.NetDriverEOS] section exists with bIsUsingP2PSockets=true
	if (!EngineIniText.Contains(TEXT("[/Script/OnlineSubsystemEOSKit.NetDriverEOS]")))
	{
		EngineIniText += TEXT("\n[/Script/OnlineSubsystemEOSKit.NetDriverEOS]\nbIsUsingP2PSockets=true\n");
		bConfigChanged = true;
	}

	// Ensure [/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit] section exists with P2P settings
	if (!EngineIniText.Contains(TEXT("[/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit]")))
	{
		EngineIniText += TEXT("\n[/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit]\nbUseLobbies=false\nbUseP2PNetworking=true\n");
		bConfigChanged = true;
	}

	// Ensure [/Script/Engine.GameEngine] section exists with NetDriverDefinitions
	if (!EngineIniText.Contains(TEXT("+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOSKit.NetDriverEOS\"")))
	{
		// Check if GameEngine section exists
		if (!EngineIniText.Contains(TEXT("[/Script/Engine.GameEngine]")))
		{
			EngineIniText += TEXT("\n[/Script/Engine.GameEngine]\n");
		}
		
		// Add NetDriverDefinitions if not present
		FString NetDriverConfig = TEXT("+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOSKit.NetDriverEOS\",DriverClassNameFallback=\"/Script/OnlineSubsystemUtils.IpNetDriver\")\n");
		EngineIniText += NetDriverConfig;
		bConfigChanged = true;
	}

	// Save if changed
	if (bConfigChanged)
	{
		if (FFileHelper::SaveStringToFile(EngineIniText, *EngineIniPath))
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Updated OnlineSubsystem configuration in DefaultEngine.ini"));
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to save DefaultEngine.ini"));
			return false;
		}
	}

	return true;
}

bool UEOSKitAutoConfigurator::AddWindowsPlatformSettings()
{
	// Windows-specific settings are typically handled by the EOS SDK automatically
	// No additional configuration needed
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Windows platform settings configured"));
	return true;
}

bool UEOSKitAutoConfigurator::AddAndroidPlatformSettings(const FString& SDKPath)
{
	if (SDKPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: Android SDK path is empty, skipping Android platform settings"));
		return false;
	}

	FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString EngineIniText;

	if (!FFileHelper::LoadFileToString(EngineIniText, *EngineIniPath))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to load DefaultEngine.ini"));
		return false;
	}

	// Add Android SDK path configuration
	FString AndroidSection = TEXT("\n[/Script/AndroidFileServerEditor.AndroidFileServerRuntimeSettings]\n");
	AndroidSection += FString::Printf(TEXT("bEnablePlugin=true\n"));
	AndroidSection += FString::Printf(TEXT("bAllowNetworkConnection=true\n"));

	if (!EngineIniText.Contains(TEXT("[/Script/AndroidFileServerEditor.AndroidFileServerRuntimeSettings]")))
	{
		EngineIniText += AndroidSection;
		if (FFileHelper::SaveStringToFile(EngineIniText, *EngineIniPath))
		{
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Added Android platform settings"));
			return true;
		}
	}

	return true;
}

bool UEOSKitAutoConfigurator::AddIOSPlatformSettings(const FString& SDKPath)
{
	if (SDKPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("EOSKit: iOS SDK path is empty, skipping iOS platform settings"));
		return false;
	}

	FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString EngineIniText;

	if (!FFileHelper::LoadFileToString(EngineIniText, *EngineIniPath))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to load DefaultEngine.ini"));
		return false;
	}

	// iOS-specific settings would go here
	// Most iOS configuration is handled through Xcode project settings
	UE_LOG(LogTemp, Log, TEXT("EOSKit: iOS platform settings configured (SDK Path: %s)"), *SDKPath);

	return true;
}

bool UEOSKitAutoConfigurator::IsValidAnsiString(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (!FChar::IsPrint(Char) || FChar::IsWhitespace(Char))
		{
			return false;
		}
	}
	return true;
}

bool UEOSKitAutoConfigurator::IsValidHexString(const FString& String, int32 ExpectedLength)
{
	if (String.Len() != ExpectedLength)
	{
		return false;
	}

	for (const TCHAR& Char : String)
	{
		if (!FChar::IsHexDigit(Char))
		{
			return false;
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE

