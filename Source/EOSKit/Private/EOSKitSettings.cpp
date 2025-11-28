#include "EOSKitSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "UnrealEdMisc.h"
#endif

#define LOCTEXT_NAMESPACE "EOSKit"

#define INI_SECTION TEXT("/Script/EOSKit.EOSKitSettings")

inline bool IsAnsi(const FString& Source)
{
	for (const TCHAR& IterChar : Source)
	{
		if (!FChar::IsPrint(IterChar))
		{
			return false;
		}
	}
	return true;
}

inline bool IsHex(const FString& Source)
{
	for (const TCHAR& IterChar : Source)
	{
		if (!FChar::IsHexDigit(IterChar))
		{
			return false;
		}
	}
	return true;
}

inline bool ContainsWhitespace(const FString& Source)
{
	for (const TCHAR& IterChar : Source)
	{
		if (FChar::IsWhitespace(IterChar))
		{
			return true;
		}
	}
	return false;
}

inline FString StripQuotes(const FString& Source)
{
	if (Source.StartsWith(TEXT("\"")))
	{
		return Source.Mid(1, Source.Len() - 2);
	}
	return Source;
}

void FEOSArtifact::ParseRawArrayEntry(const FString& RawLine)
{
	const TCHAR* Delims[4] = { TEXT("("), TEXT(")"), TEXT("="), TEXT(",") };
	TArray<FString> Values;
	RawLine.ParseIntoArray(Values, Delims, 4, false);
	for (int32 ValueIndex = 0; ValueIndex < Values.Num(); ValueIndex++)
	{
		if (Values[ValueIndex].IsEmpty())
		{
			continue;
		}

		// Parse which struct field
		if (Values[ValueIndex] == TEXT("ArtifactName"))
		{
			ArtifactName = StripQuotes(Values[ValueIndex + 1]);
		}
		else if (Values[ValueIndex] == TEXT("ClientId"))
		{
			ClientId = StripQuotes(Values[ValueIndex + 1]);
		}
		else if (Values[ValueIndex] == TEXT("ClientSecret"))
		{
			ClientSecret = StripQuotes(Values[ValueIndex + 1]);
		}
		else if (Values[ValueIndex] == TEXT("ProductId"))
		{
			ProductId = StripQuotes(Values[ValueIndex + 1]);
		}
		else if (Values[ValueIndex] == TEXT("SandboxId"))
		{
			SandboxId = StripQuotes(Values[ValueIndex + 1]);
		}
		else if (Values[ValueIndex] == TEXT("DeploymentId"))
		{
			DeploymentId = StripQuotes(Values[ValueIndex + 1]);
		}
		else if (Values[ValueIndex] == TEXT("EncryptionKey"))
		{
			EncryptionKey = StripQuotes(Values[ValueIndex + 1]);
		}
		ValueIndex++;
	}
}

UEOSKitSettings::UEOSKitSettings()
{
	// Initialize with a default artifact
	FEOSArtifact DefaultArtifact;
	DefaultArtifact.ArtifactName = TEXT("DefaultArtifact");
	DefaultArtifact.ProductId = TEXT("");
	DefaultArtifact.SandboxId = TEXT("");
	DefaultArtifact.DeploymentId = TEXT("");
	DefaultArtifact.ClientId = TEXT("");
	DefaultArtifact.ClientSecret = TEXT("");
	DefaultArtifact.EncryptionKey = TEXT("");
	
	if (Artifacts.Num() == 0)
	{
		Artifacts.Add(DefaultArtifact);
	}

	// Initialize with default voice artifact
	FEOSArtifact VoiceArtifact;
	VoiceArtifact.ArtifactName = TEXT("DefaultArtifact");
	
	if (Artifacts.Num() < 2)
	{
		Artifacts.Add(VoiceArtifact);
	}

	// Initialize with dedicated server artifact
	FEOSArtifact ServerArtifact;
	ServerArtifact.ArtifactName = TEXT("DefaultArtifact");
	
	if (Artifacts.Num() < 3)
	{
		Artifacts.Add(ServerArtifact);
	}
}

#if WITH_EDITOR

// Helper function to generate encryption key - must be defined before use
FString UEOSKitSettings::GenerateRandomEncryptionKey()
{
	// Generate 64 random hexadecimal characters using FGuid
	// We'll generate two GUIDs (32 chars each) to get 64 chars
	FGuid Guid1 = FGuid::NewGuid();
	FGuid Guid2 = FGuid::NewGuid();
	
	// Convert GUIDs to hex strings (without dashes)
	FString HexString = Guid1.ToString(EGuidFormats::Digits) + Guid2.ToString(EGuidFormats::Digits);
	
	// Convert to lowercase for consistency with EOS encryption keys
	HexString = HexString.ToLower();
	
	// Ensure it's exactly 64 characters (should always be)
	check(HexString.Len() == 64);
	
	return HexString;
}

void UEOSKitSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property == nullptr)
	{
		return;
	}

	const FName PropertyName = PropertyChangedEvent.Property->GetFName();
	
	// Handle "Automatically Setup EOK" checkbox
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UEOSKitSettings, bAutomaticallySetupEIK))
	{
		if (bAutomaticallySetupEIK)
		{
			// IMPORTANT: Generate encryption keys for any artifacts that don't have one
			bool bAnyKeysGenerated = false;
			for (FEOSArtifact& Artifact : Artifacts)
			{
				if (Artifact.EncryptionKey.IsEmpty())
				{
					Artifact.EncryptionKey = GenerateRandomEncryptionKey();
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Auto-generated encryption key for artifact '%s': %s"), 
						*Artifact.ArtifactName, *Artifact.EncryptionKey);
					bAnyKeysGenerated = true;
				}
			}
			
			if (bAnyKeysGenerated)
			{
				// Mark as modified so the user can save
				MarkPackageDirty();
			}
			
			// Save config first - THIS IS CRITICAL
			SaveConfig();
			
			// IMPORTANT: Force write the full configuration using GConfig
			WriteFullConfigToEngineIni();
			
			// Update OnlineSubsystem configuration
			UpdateOnlineSubsystemConfig();
			
			// Force flush to ensure everything is written
			GConfig->Flush(false);
			
			UE_LOG(LogTemp, Warning, TEXT("===================================================="));
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: Automatically Setup EOK enabled"));
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: Configuration written to DefaultEngine.ini"));
			UE_LOG(LogTemp, Warning, TEXT("===================================================="));
			
			// Show restart prompt (EIK style)
			if (EAppReturnType::Ok == ShowRestartWarning(LOCTEXT("RestartEditorTitle", "EOSKit - Restart Required")))
			{
				UE_LOG(LogTemp, Log, TEXT("EOSKit: User chose to restart editor"));
				FUnrealEdMisc::Get().RestartEditor(true);
			}
		}
		else
		{
			// User unchecked the box
			SaveConfig();
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Automatically Setup EOK disabled"));
		}
	}
	// Handle artifact changes - automatically generate encryption key for new artifacts
	else if (PropertyChangedEvent.MemberProperty != nullptr &&
		PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UEOSKitSettings, Artifacts))
	{
		// Check if this is an "Add" operation
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
		{
			// Get the index of the newly added artifact
			int32 AddedIndex = PropertyChangedEvent.GetArrayIndex(TEXT("Artifacts"));
			
			if (Artifacts.IsValidIndex(AddedIndex))
			{
				FEOSArtifact& NewArtifact = Artifacts[AddedIndex];
				
				// Only generate if the encryption key is empty
				if (NewArtifact.EncryptionKey.IsEmpty())
				{
					// Generate a new encryption key
					NewArtifact.EncryptionKey = GenerateRandomEncryptionKey();
					
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Auto-generated encryption key for new artifact '%s': %s"), 
						*NewArtifact.ArtifactName, *NewArtifact.EncryptionKey);
					
					// Mark as modified
					MarkPackageDirty();
				}
			}
		}
		
		// Always validate artifacts when they change
		ValidateArtifacts();
		
		// If auto-setup is enabled, update config
		if (bAutomaticallySetupEIK)
		{
			SaveToDefaultEngineIni();
		}
	}
	// Handle other property changes
	else
	{
		// For other property changes, just save normally
		SaveConfig();
		
		// If auto-setup is enabled, also update the full config
		if (bAutomaticallySetupEIK)
		{
			SaveToDefaultEngineIni();
		}
		
		UE_LOG(LogTemp, Log, TEXT("EOSKit Settings: Property '%s' changed and saved"), *PropertyName.ToString());
	}
}

EAppReturnType::Type UEOSKitSettings::ShowRestartWarning(const FText& Title)
{
	FText Message = LOCTEXT("RestartWarningMsg", 
		"EOSKit configuration has been written to DefaultEngine.ini.\n\n"
		"The Unreal Editor must be restarted for these changes to take effect.\n\n"
		"Do you wish to restart now (you will be prompted to save any changes)?");
	
	return FMessageDialog::Open(EAppMsgType::OkCancel, Message, Title);
}

void UEOSKitSettings::ValidateArtifacts()
{
	// Validate artifact entries (similar to EOK)
	for (FEOSArtifact& Artifact : Artifacts)
	{
		// Validate Client ID
		if (!Artifact.ClientId.IsEmpty())
		{
			if (!Artifact.ClientId.StartsWith(TEXT("xyz")))
			{
				FMessageDialog::Open(EAppMsgType::Ok,
					LOCTEXT("ClientIdInvalidMsg", "Client IDs created after SDK version 1.5 start with 'xyz'. Double check that you did not use your BPT Client ID instead."));
			}
			
			// Check for ANSI and no whitespace
			bool bIsValid = true;
			for (const TCHAR& Char : Artifact.ClientId)
			{
				if (!FChar::IsPrint(Char) || FChar::IsWhitespace(Char))
				{
					bIsValid = false;
					break;
				}
			}
			
			if (!bIsValid)
			{
				FMessageDialog::Open(EAppMsgType::Ok,
					LOCTEXT("ClientIdNotAnsiMsg", "Client ID must contain ANSI printable characters only with no whitespace"));
				Artifact.ClientId.Empty();
			}
		}
		
		// Validate Client Secret
		if (!Artifact.ClientSecret.IsEmpty())
		{
			bool bIsValid = true;
			for (const TCHAR& Char : Artifact.ClientSecret)
			{
				if (!FChar::IsPrint(Char) || FChar::IsWhitespace(Char))
				{
					bIsValid = false;
					break;
				}
			}
			
			if (!bIsValid)
			{
				FMessageDialog::Open(EAppMsgType::Ok,
					LOCTEXT("ClientSecretNotAnsiMsg", "Client Secret must contain ANSI printable characters only with no whitespace"));
				Artifact.ClientSecret.Empty();
			}
		}
		
		// Validate Encryption Key
		if (!Artifact.EncryptionKey.IsEmpty())
		{
			bool bIsValidHex = true;
			if (Artifact.EncryptionKey.Len() != 64)
			{
				bIsValidHex = false;
			}
			else
			{
				for (const TCHAR& Char : Artifact.EncryptionKey)
				{
					if (!FChar::IsHexDigit(Char))
					{
					 bIsValidHex = false;
					 break;
					}
				}
			}
			
			if (!bIsValidHex)
			{
				FMessageDialog::Open(EAppMsgType::Ok,
					LOCTEXT("EncryptionKeyNotHexMsg", "Encryption Key must contain exactly 64 hexadecimal characters"));
				Artifact.EncryptionKey.Empty();
			}
		}
	}
}

void UEOSKitSettings::UpdateOnlineSubsystemConfig()
{
	FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString EngineIniText;
	
	if (!FFileHelper::LoadFileToString(EngineIniText, *EngineIniPath))
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to load DefaultEngine.ini"));
		return;
	}
	
	bool bConfigChanged = false;
	
	// DISABLE OnlineSubsystemEOS to prevent packaging conflicts
	if (!EngineIniText.Contains(TEXT("[OnlineSubsystemEOS]")))
	{
		EngineIniText += TEXT("\n[OnlineSubsystemEOS]\nbEnabled=false\n");
		bConfigChanged = true;
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Added [OnlineSubsystemEOS] section with bEnabled=false"));
	}
	else if (EngineIniText.Contains(TEXT("[OnlineSubsystemEOS]\nbEnabled=true")))
	{
		EngineIniText.ReplaceInline(TEXT("[OnlineSubsystemEOS]\nbEnabled=true"), TEXT("[OnlineSubsystemEOS]\nbEnabled=false"));
		bConfigChanged = true;
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Set [OnlineSubsystemEOS] bEnabled=false to prevent packaging conflicts"));
	}
	
	// Check if [OnlineSubsystemEOSKit] section exists and add it if not
	if (!EngineIniText.Contains(TEXT("[OnlineSubsystemEOSKit]")))
	{
		EngineIniText += TEXT("\n[OnlineSubsystemEOSKit]\nbEnabled=true\n");
		bConfigChanged = true;
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Added [OnlineSubsystemEOSKit] section"));
	}
	
	// Update [OnlineSubsystem] section - Set to EOSKit
	if (!EngineIniText.Contains(TEXT("DefaultPlatformService=EOSKit")))
	{
		// Remove any existing DefaultPlatformService setting
		if (EngineIniText.Contains(TEXT("DefaultPlatformService=EOS")))
		{
			EngineIniText.ReplaceInline(TEXT("DefaultPlatformService=EOS"), TEXT("DefaultPlatformService=EOSKit"));
			bConfigChanged = true;
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Changed DefaultPlatformService to EOSKit"));
		}
		else if (!EngineIniText.Contains(TEXT("[OnlineSubsystem]")))
		{
			EngineIniText += TEXT("\n[OnlineSubsystem]\nDefaultPlatformService=EOSKit\n");
			bConfigChanged = true;
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Set DefaultPlatformService=EOSKit"));
		}
		else
		{
			// Add to existing section
			int32 SectionPos = EngineIniText.Find(TEXT("[OnlineSubsystem]"));
			if (SectionPos != INDEX_NONE)
			{
				int32 NextSectionPos = EngineIniText.Find(TEXT("\n["), SectionPos + 1);
				if (NextSectionPos == INDEX_NONE)
				{
					NextSectionPos = EngineIniText.Len();
				}
				
				FString InsertText = TEXT("DefaultPlatformService=EOSKit\n");
				EngineIniText.InsertAt(NextSectionPos, InsertText);
				bConfigChanged = true;
				UE_LOG(LogTemp, Log, TEXT("EOSKit: Set DefaultPlatformService=EOSKit"));
			}
		}
	}
	
	// Update [/Script/OnlineSubsystemEOSKit.NetDriverEOSKit] section
	if (!EngineIniText.Contains(TEXT("[/Script/OnlineSubsystemEOSKit.NetDriverEOSKit]")))
	{
		FString Comment = TEXT("\n; EOSKit Comment: P2P sockets setting - dynamically set in Travel URL for Listen/Dedicated Server\n");
		EngineIniText += Comment;
		EngineIniText += TEXT("[/Script/OnlineSubsystemEOSKit.NetDriverEOSKit]\nbIsUsingP2PSockets=true\n");
		bConfigChanged = true;
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Added NetDriver configuration"));
	}
	
	// Update [/Script/Engine.GameEngine] section with NetDriverDefinitions
	// Use OnlineSubsystemEOSKit.NetDriverEOSKit
	if (!EngineIniText.Contains(TEXT("OnlineSubsystemEOSKit.NetDriverEOSKit")))
	{
		FString NetDriverConfig;
		
		if (!EngineIniText.Contains(TEXT("[/Script/Engine.GameEngine]")))
		{
			NetDriverConfig = TEXT("\n[/Script/Engine.GameEngine]\n");
		}
		else
		{
			int32 GameEnginePos = EngineIniText.Find(TEXT("[/Script/Engine.GameEngine]"));
			if (GameEnginePos != INDEX_NONE)
			{
				int32 NextSectionPos = EngineIniText.Find(TEXT("\n["), GameEnginePos + 1);
				if (NextSectionPos == INDEX_NONE)
				{
					NextSectionPos = EngineIniText.Len();
				}
				
				// Check if NetDriverDefinitions already exists
				if (!EngineIniText.Contains(TEXT("!NetDriverDefinitions=ClearArray")))
				{
					NetDriverConfig = TEXT("!NetDriverDefinitions=ClearArray\n");
				}
			}
		}
		
		// Use OnlineSubsystemEOSKit.NetDriverEOSKit
		NetDriverConfig += TEXT("+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"OnlineSubsystemEOSKit.NetDriverEOSKit\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n");
		
		if (!NetDriverConfig.IsEmpty())
		{
			if (!EngineIniText.Contains(TEXT("[/Script/Engine.GameEngine]")))
			{
				EngineIniText += NetDriverConfig;
			}
			else
			{
				int32 GameEnginePos = EngineIniText.Find(TEXT("[/Script/Engine.GameEngine]"));
				int32 NextSectionPos = EngineIniText.Find(TEXT("\n["), GameEnginePos + 1);
				if (NextSectionPos == INDEX_NONE)
				{
					NextSectionPos = EngineIniText.Len();
				}
				EngineIniText.InsertAt(NextSectionPos, NetDriverConfig);
			}
			bConfigChanged = true;
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Added OnlineSubsystemEOSKit NetDriver definitions"));
		}
	}
	
	// Save the modified text back to DefaultEngine.ini if any changes were made
	if (bConfigChanged)
	{
		if (FFileHelper::SaveStringToFile(EngineIniText, *EngineIniPath))
		{
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: OnlineSubsystem configuration updated in DefaultEngine.ini"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to save modified DefaultEngine.ini"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: OnlineSubsystem configuration already exists"));
	}
}

FText UEOSKitSettings::GetSectionText() const
{
	return NSLOCTEXT("EOSKit", "EOSKitSettingsName", "EOS Kit");
}

FText UEOSKitSettings::GetSectionDescription() const
{
	return NSLOCTEXT("EOSKit", "EOSKitSettingsDescription", "Configure Epic Online Services integration settings for your project");
}

void UEOSKitSettings::SaveToDefaultEngineIni()
{
	// Write to DefaultEngine.ini instead of DefaultGame.ini
	FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const FString ConfigSectionName = TEXT("/Script/EOSKit.EOSKitSettings");
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Saving settings to %s"), *ConfigPath);
	
	// Update the config file with current values using GConfig
	GConfig->SetBool(*ConfigSectionName, TEXT("bAutomaticallySetupEIK"), bAutomaticallySetupEIK, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableAutoLogin"), bEnableAutoLogin, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("AutoLoginCredentialType"), *AutoLoginCredentialType, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("ProductName"), *ProductName, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("ProductVersion"), *ProductVersion, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("DefaultArtifactName"), *DefaultArtifactName, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("ProductId"), *ProductId, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("SandboxId"), *SandboxId, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("DeploymentId"), *DeploymentId, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("EncryptionKey"), *EncryptionKey, ConfigPath);
	
	// Flush the config cache to ensure changes are written
	GConfig->Flush(false, ConfigPath);
	
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Settings saved successfully"));
}

void UEOSKitSettings::WriteFullConfigToEngineIni()
{
	// Write to DefaultEngine.ini instead of DefaultGame.ini
	FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const FString ConfigSectionName = TEXT("/Script/EOSKit.EOSKitSettings");
	
	UE_LOG(LogTemp, Log, TEXT("===================================================="));
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Writing Full Configuration to DefaultEngine.ini"));
	UE_LOG(LogTemp, Log, TEXT("Config Path: %s"), *ConfigPath);
	UE_LOG(LogTemp, Log, TEXT("===================================================="));
	
	// Core Settings
	GConfig->SetBool(*ConfigSectionName, TEXT("bAutomaticallySetupEIK"), bAutomaticallySetupEIK, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bAutoLaunchDevTool"), bAutoLaunchDevTool, ConfigPath);
	
	// Product Information
	GConfig->SetString(*ConfigSectionName, TEXT("OrganizationName"), *OrganizationName, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("ProductName"), *ProductName, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("ProductVersion"), *ProductVersion, ConfigPath);
	
	// Google Integration
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableGoogleOneTap"), bEnableGoogleOneTap, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableGooglePlayGames"), bEnableGooglePlayGames, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("GooglePlayGamesAppId"), *GooglePlayGamesAppId, ConfigPath);
	
	// Auto-Login Settings
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableAutoLogin"), bEnableAutoLogin, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("AutoLoginCredentialType"), *AutoLoginCredentialType, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("AutoLoginCredentialId"), *AutoLoginCredentialId, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("AutoLoginCredentialToken"), *AutoLoginCredentialToken, ConfigPath);
	
	// EOS Platform Settings
	GConfig->SetInt(*ConfigSectionName, TEXT("TickBudgetInMilliseconds"), TickBudgetInMilliseconds, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bRequireEpicGamesStoreLaunch"), bRequireEpicGamesStoreLaunch, ConfigPath);
	
	// Overlay Settings
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableOverlay"), bEnableOverlay, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableSocialOverlay"), bEnableSocialOverlay, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableEditorOverlay"), bEnableEditorOverlay, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("ReturnLevelName"), *ReturnLevelName, ConfigPath);
	
	// Feature Settings
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableTitleStorage"), bEnableTitleStorage, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnablePlayerTicketing"), bEnablePlayerTicketing, ConfigPath);
	
	// Artifact Names
	GConfig->SetString(*ConfigSectionName, TEXT("DefaultArtifactName"), *DefaultArtifactName, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("VoiceArtifactName"), *VoiceArtifactName, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("DedicatedServerArtifactName"), *DedicatedServerArtifactName, ConfigPath);
	
	// Platform Specific
	GConfig->SetString(*ConfigSectionName, TEXT("AndroidArtifactName"), *AndroidArtifactName, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("iOSArtifactName"), *iOSArtifactName, ConfigPath);
	
	// One Click Deploy
	GConfig->SetBool(*ConfigSectionName, TEXT("bEnableOneClickDeploy"), bEnableOneClickDeploy, ConfigPath);
	
	// Legacy Credentials (for backward compatibility)
	GConfig->SetString(*ConfigSectionName, TEXT("ProductId"), *ProductId, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("SandboxId"), *SandboxId, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("DeploymentId"), *DeploymentId, ConfigPath);
	GConfig->SetString(*ConfigSectionName, TEXT("EncryptionKey"), *EncryptionKey, ConfigPath);
	
	// Write Auth Scope Flags
	GConfig->SetBool(*ConfigSectionName, TEXT("bAuthScope_BasicProfile"), bAuthScope_BasicProfile, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bAuthScope_FriendsList"), bAuthScope_FriendsList, ConfigPath);
	GConfig->SetBool(*ConfigSectionName, TEXT("bAuthScope_Presence"), bAuthScope_Presence, ConfigPath);
	
	// Write all artifacts (similar to EOK format)
	// First, remove existing artifact entries
	TArray<FString> ExistingArtifacts;
	GConfig->GetArray(*ConfigSectionName, TEXT("+Artifacts"), ExistingArtifacts, ConfigPath);

	// Clear existing artifacts
	for (int32 i = 0; i < ExistingArtifacts.Num(); i++)
	{
		GConfig->RemoveKey(*ConfigSectionName, TEXT("+Artifacts"), ConfigPath);
	}
	
	// Write updated artifacts in EOK format
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Writing %d artifact(s) to config:"), Artifacts.Num());
	for (int32 i = 0; i < Artifacts.Num(); i++)
	{
		const FEOSArtifact& Artifact = Artifacts[i];
		
		// Format: +Artifacts=(ArtifactName="name",ClientId="...",ClientSecret="...",ProductId="...",SandboxId="...",DeploymentId="...",EncryptionKey="...")
		FString ArtifactString = FString::Printf(
			TEXT("(ArtifactName=\"%s\",ClientId=\"%s\",ClientSecret=\"%s\",ProductId=\"%s\",SandboxId=\"%s\",DeploymentId=\"%s\",EncryptionKey=\"%s\")"),
			*Artifact.ArtifactName,
			*Artifact.ClientId,
			*Artifact.ClientSecret,
			*Artifact.ProductId,
			*Artifact.SandboxId,
			*Artifact.DeploymentId,
			*Artifact.EncryptionKey
		);
		
		GConfig->SetString(*ConfigSectionName, TEXT("+Artifacts"), *ArtifactString, ConfigPath);

		UE_LOG(LogTemp, Log, TEXT("  [%d] Artifact: %s"), i, *Artifact.ArtifactName);
		if (!Artifact.ClientId.IsEmpty())
		{
			FString ClientIdPreview = Artifact.ClientId.Left(10) + TEXT("...");
			UE_LOG(LogTemp, Log, TEXT("      Client ID: %s"), *ClientIdPreview);
		}
		if (!Artifact.ProductId.IsEmpty())
		{
			FString ProductIdPreview = Artifact.ProductId.Left(10) + TEXT("...");
			UE_LOG(LogTemp, Log, TEXT("      Product ID: %s"), *ProductIdPreview);
		}
	}
	
	// Flush to disk
	GConfig->Flush(false, ConfigPath);
	
	UE_LOG(LogTemp, Log, TEXT("===================================================="));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Full configuration written successfully"));
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Total artifacts configured: %d"), Artifacts.Num());
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? Config file: %s"), *ConfigPath);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: ? RESTART REQUIRED - Please restart the editor!"));
	UE_LOG(LogTemp, Log, TEXT("===================================================="));
}
#endif // WITH_EDITOR

FEOSArtifact UEOSKitSettings::GetActiveArtifact() const
{
	FString PlatformArtifactName;

#if PLATFORM_ANDROID
	PlatformArtifactName = AndroidArtifactName;
#elif PLATFORM_IOS
	PlatformArtifactName = iOSArtifactName;
#else
	PlatformArtifactName = DefaultArtifactName;
#endif

	// Try to find the platform-specific artifact
	for (const FEOSArtifact& Artifact : Artifacts)
	{
		if (Artifact.ArtifactName == PlatformArtifactName)
		{
			return Artifact;
		}
	}

	// Fall back to legacy single-artifact configuration
	FEOSArtifact LegacyArtifact;
	LegacyArtifact.ArtifactName = DefaultArtifactName;
	LegacyArtifact.ProductId = ProductId;
	LegacyArtifact.SandboxId = SandboxId;
	LegacyArtifact.DeploymentId = DeploymentId;
	LegacyArtifact.EncryptionKey = EncryptionKey;

	return LegacyArtifact;
}

FEOSArtifact UEOSKitSettings::GetArtifactByName(const FString& ArtifactName) const
{
	for (const FEOSArtifact& Artifact : Artifacts)
	{
		if (Artifact.ArtifactName == ArtifactName)
		{
			return Artifact;
		}
	}

	// Return an empty artifact if not found
	return FEOSArtifact();
}

TArray<FString> UEOSKitSettings::GetEnabledAuthScopeFlags() const
{
	TArray<FString> EnabledFlags;
	
	if (bAuthScope_BasicProfile)
	{
		EnabledFlags.Add(TEXT("BasicProfile"));
	}
	
	if (bAuthScope_FriendsList)
	{
		EnabledFlags.Add(TEXT("FriendsList"));
	}
	
	if (bAuthScope_Presence)
	{
		EnabledFlags.Add(TEXT("Presence"));
	}
	
	return EnabledFlags;
}

bool UEOSKitSettings::GetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings)
{
	if (UObjectInitialized())
	{
		return UEOSKitSettings::AutoGetSettingsForArtifact(ArtifactName, OutSettings);
	}
	return UEOSKitSettings::ManualGetSettingsForArtifact(ArtifactName, OutSettings);
}

bool UEOSKitSettings::ManualGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings)
{
	static TOptional<FString> CachedDefaultArtifactName;
	static TOptional<TArray<FEOSArtifact>> CachedArtifactSettings;

	if (!CachedDefaultArtifactName.IsSet())
	{
		CachedDefaultArtifactName.Emplace();
		GConfig->GetString(INI_SECTION, TEXT("DefaultArtifactName"), *CachedDefaultArtifactName, GEngineIni);
	}

	if (!CachedArtifactSettings.IsSet())
	{
		CachedArtifactSettings.Emplace();

		TArray<FString> Artifacts;
		GConfig->GetArray(INI_SECTION, TEXT("Artifacts"), Artifacts, GEngineIni);
		for (const FString& Line : Artifacts)
		{
			FEOSArtifact Artifact;
			Artifact.ParseRawArrayEntry(Line);
			CachedArtifactSettings->Add(Artifact);
		}
	}

	FString ArtifactNameOverride;
	FString DeploymentIdOverride;
	FString SandboxIdOverride;

	// Figure out which config object we are loading
	FParse::Value(FCommandLine::Get(), TEXT("EOSArtifactNameOverride="), ArtifactNameOverride);
	if (ArtifactNameOverride.IsEmpty())
	{
		ArtifactNameOverride = ArtifactName;
	}

	// Override DeploymentId and SandboxId from command line
	if (FParse::Value(FCommandLine::Get(), TEXT("epicdeploymentid="), DeploymentIdOverride))
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Overriding DeploymentId with %s"), *DeploymentIdOverride);
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("epicsandboxid="), SandboxIdOverride))
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Overriding SandboxId with %s"), *SandboxIdOverride);
	}

	// Search by name and then default if not found
	for (const FEOSArtifact& Artifact : *CachedArtifactSettings)
	{
		if (Artifact.ArtifactName == ArtifactNameOverride)
		{
			OutSettings = Artifact;
			if (!DeploymentIdOverride.IsEmpty())
			{
				OutSettings.DeploymentId = DeploymentIdOverride;
			}
			if (!SandboxIdOverride.IsEmpty())
			{
				OutSettings.SandboxId = SandboxIdOverride;
			}
			return true;
		}
	}

	// Fall back to default artifact
	for (const FEOSArtifact& Artifact : *CachedArtifactSettings)
	{
		if (Artifact.ArtifactName == *CachedDefaultArtifactName)
		{
			OutSettings = Artifact;
			if (!DeploymentIdOverride.IsEmpty())
			{
				OutSettings.DeploymentId = DeploymentIdOverride;
			}
			if (!SandboxIdOverride.IsEmpty())
			{
				OutSettings.SandboxId = SandboxIdOverride;
			}
			return true;
		}
	}

	return false;
}

bool UEOSKitSettings::AutoGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings)
{
	const UEOSKitSettings* This = GetDefault<UEOSKitSettings>();
	FString DeploymentIdOverride;
	FString SandboxIdOverride;
	
	if (FParse::Value(FCommandLine::Get(), TEXT("epicdeploymentid="), DeploymentIdOverride))
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Overriding DeploymentId with %s"), *DeploymentIdOverride);
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("epicsandboxid="), SandboxIdOverride))
	{
		UE_LOG(LogTemp, Log, TEXT("EOSKit: Overriding SandboxId with %s"), *SandboxIdOverride);
	}
	
	FString ArtifactNameOverride;
	// Figure out which config object we are loading
	FParse::Value(FCommandLine::Get(), TEXT("EOSArtifactNameOverride="), ArtifactNameOverride);
	if (ArtifactNameOverride.IsEmpty())
	{
		ArtifactNameOverride = ArtifactName;
	}
	
	// Search by name
	for (const FEOSArtifact& Artifact : This->Artifacts)
	{
		if (Artifact.ArtifactName == ArtifactNameOverride)
		{
			OutSettings = Artifact;
			if (!DeploymentIdOverride.IsEmpty())
			{
				OutSettings.DeploymentId = DeploymentIdOverride;
			}
			if (!SandboxIdOverride.IsEmpty())
			{
				OutSettings.SandboxId = SandboxIdOverride;
			}
			return true;
		}
	}
	
	// Fall back to default artifact
	for (const FEOSArtifact& Artifact : This->Artifacts)
	{
		if (Artifact.ArtifactName == This->DefaultArtifactName)
		{
			OutSettings = Artifact;
			if (!DeploymentIdOverride.IsEmpty())
			{
				OutSettings.DeploymentId = DeploymentIdOverride;
			}
			if (!SandboxIdOverride.IsEmpty())
			{
				OutSettings.SandboxId = SandboxIdOverride;
			}
			return true;
		}
	}
	
	if (!ArtifactNameOverride.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find artifact settings for %s"), *ArtifactNameOverride);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find artifact settings. Please check Project Settings under EOSKit"));
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
