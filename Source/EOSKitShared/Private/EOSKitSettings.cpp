#include "EOSKitSettings.h"
#include "CoreMinimal.h" // Restore minimal engine includes for runtime safety
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

#define INI_SECTION TEXT("/Script/EOSKitShared.EOSKitSettings")

static bool IsArtifactComplete(const FEOSArtifact& Artifact)
{
	return !Artifact.ProductId.IsEmpty()
		&& !Artifact.SandboxId.IsEmpty()
		&& !Artifact.DeploymentId.IsEmpty()
		&& !Artifact.ClientId.IsEmpty()
		&& !Artifact.ClientSecret.IsEmpty()
		&& !Artifact.EncryptionKey.IsEmpty();
}

static FString GetFirstValidArtifactName(const TArray<FEOSArtifact>& ArtifactList)
{
	for (const FEOSArtifact& Artifact : ArtifactList)
	{
		if (!Artifact.ArtifactName.IsEmpty())
		{
			return Artifact.ArtifactName;
		}
	}
	return FString();
}

static FString ResolveArtifactName(const FString& PreferredName, const TArray<FEOSArtifact>& ArtifactList)
{
	if (!PreferredName.IsEmpty())
	{
		return PreferredName;
	}
	return GetFirstValidArtifactName(ArtifactList);
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
	// Don't pre-populate artifacts - let users add them manually
	// Artifacts array starts empty
}

#if WITH_EDITOR
FString UEOSKitSettings::GenerateRandomEncryptionKey()
{
	FGuid Guid1 = FGuid::NewGuid();
	FGuid Guid2 = FGuid::NewGuid();
	FString HexString = Guid1.ToString(EGuidFormats::Digits) + Guid2.ToString(EGuidFormats::Digits);
	HexString = HexString.ToLower();
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

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UEOSKitSettings, bAutomaticallySetupEIK))
	{
		if (bAutomaticallySetupEIK)
		{
			bool bAnyKeysGenerated = false;
			for (FEOSArtifact& Artifact : Artifacts)
			{
				if (Artifact.EncryptionKey.IsEmpty())
				{
					Artifact.EncryptionKey = GenerateRandomEncryptionKey();
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Auto-generated encryption key for artifact '%s': %s"), *Artifact.ArtifactName, *Artifact.EncryptionKey);
					bAnyKeysGenerated = true;
				}
			}
			if (bAnyKeysGenerated)
			{
				MarkPackageDirty();
			}
			SaveConfig();
			WriteFullConfigToEngineIni();
			UpdateOnlineSubsystemConfig();
			GConfig->Flush(false);
			UE_LOG(LogTemp, Warning, TEXT("===================================================="));
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: Automatically Setup EOK enabled"));
			UE_LOG(LogTemp, Warning, TEXT("EOSKit: Configuration written to DefaultEngine.ini"));
			UE_LOG(LogTemp, Warning, TEXT("===================================================="));
			if (EAppReturnType::Ok == ShowRestartWarning(LOCTEXT("RestartEditorTitle", "EOSKit - Restart Required")))
			{
				FUnrealEdMisc::Get().RestartEditor(true);
			}
		}
		else
		{
			SaveConfig();
			UE_LOG(LogTemp, Log, TEXT("EOSKit: Automatically Setup EOK disabled"));
		}
	}
	else if (PropertyChangedEvent.MemberProperty != nullptr && PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UEOSKitSettings, Artifacts))
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
		{
			int32 AddedIndex = PropertyChangedEvent.GetArrayIndex(TEXT("Artifacts"));
			if (Artifacts.IsValidIndex(AddedIndex))
			{
				FEOSArtifact& NewArtifact = Artifacts[AddedIndex];
				if (NewArtifact.EncryptionKey.IsEmpty())
				{
					NewArtifact.EncryptionKey = GenerateRandomEncryptionKey();
					UE_LOG(LogTemp, Warning, TEXT("EOSKit: Auto-generated encryption key for new artifact '%s': %s"), *NewArtifact.ArtifactName, *NewArtifact.EncryptionKey);
					MarkPackageDirty();
				}
			}
		}
		ValidateArtifacts();
		if (bAutomaticallySetupEIK)
		{
			SaveToDefaultEngineIni();
		}
	}
	else
	{
		SaveConfig();
		if (bAutomaticallySetupEIK)
		{
			SaveToDefaultEngineIni();
		}
		UE_LOG(LogTemp, Log, TEXT("EOSKit Settings: Property '%s' changed and saved"), *PropertyName.ToString());
	}
}

EAppReturnType::Type UEOSKitSettings::ShowRestartWarning(const FText& Title)
{
	FText Message = LOCTEXT("RestartWarningMsg", "EOSKit configuration has been written to DefaultEngine.ini.\n\nThe Unreal Editor must be restarted for these changes to take effect.\n\nDo you wish to restart now (you will be prompted to save any changes)?");
	return FMessageDialog::Open(EAppMsgType::OkCancel, Message, Title);
}

void UEOSKitSettings::ValidateArtifacts()
{
	for (FEOSArtifact& Artifact : Artifacts)
	{
		if (!Artifact.ClientId.IsEmpty())
		{
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
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ClientIdNotAnsiMsg", "Client ID must contain ANSI printable characters only with no whitespace"));
				Artifact.ClientId.Empty();
			}
		}
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
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ClientSecretNotAnsiMsg", "Client Secret must contain ANSI printable characters only with no whitespace"));
				Artifact.ClientSecret.Empty();
			}
		}
		if (!Artifact.EncryptionKey.IsEmpty())
		{
			bool bIsValidHex = (Artifact.EncryptionKey.Len() == 64);
			if (bIsValidHex)
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
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("EncryptionKeyNotHexMsg", "Encryption Key must contain exactly 64 hexadecimal characters"));
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
	if (!EngineIniText.Contains(TEXT("DefaultPlatformService=EOSKit")))
	{
		if (EngineIniText.Contains(TEXT("DefaultPlatformService=EOS")))
		{
			EngineIniText.ReplaceInline(TEXT("DefaultPlatformService=EOS"), TEXT("DefaultPlatformService=EOSKit"));
		}
		else if (EngineIniText.Contains(TEXT("DefaultPlatformService=NULL")))
		{
			EngineIniText.ReplaceInline(TEXT("DefaultPlatformService=NULL"), TEXT("DefaultPlatformService=EOSKit"));
		}
		else if (!EngineIniText.Contains(TEXT("[OnlineSubsystem]")))
		{
			EngineIniText += TEXT("\n[OnlineSubsystem]\nDefaultPlatformService=EOSKit\n");
		}
		else
		{
			int32 SectionPos = EngineIniText.Find(TEXT("[OnlineSubsystem]"));
			int32 NextSectionPos = EngineIniText.Find(TEXT("\n["), SectionPos + 1);
			if (NextSectionPos == INDEX_NONE)
			{
				NextSectionPos = EngineIniText.Len();
			}
			EngineIniText.InsertAt(NextSectionPos, TEXT("DefaultPlatformService=EOSKit\n"));
		}
		bConfigChanged = true;
	}
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
	
	if (!EngineIniText.Contains(TEXT("OnlineSubsystemEOSKit.NetDriverEOS")))
	{
		FString NetDriverConfig = TEXT("\n[/Script/Engine.GameEngine]\n!NetDriverDefinitions=ClearArray\n+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOSKit.NetDriverEOS\",DriverClassNameFallback=\"/Script/OnlineSubsystemUtils.IpNetDriver\")\n");
		EngineIniText += NetDriverConfig;
		bConfigChanged = true;
	}
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
	if (!GConfig)
	{
		return;
	}
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const TCHAR* SettingsSection = TEXT("/Script/EOSKitShared.EOSKitSettings");
	GConfig->SetBool(SettingsSection, TEXT("bAutomaticallySetupEIK"), bAutomaticallySetupEIK, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bEnableAutoLogin"), bEnableAutoLogin, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("AutoLoginCredentialType"), *AutoLoginCredentialType, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("ProductName"), *ProductName, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("ProductVersion"), *ProductVersion, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("DefaultArtifactName"), *DefaultArtifactName, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("ProductId"), *ProductId, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("SandboxId"), *SandboxId, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("DeploymentId"), *DeploymentId, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("EncryptionKey"), *EncryptionKey, ConfigPath);
	GConfig->Flush(false, ConfigPath);
}

void UEOSKitSettings::WriteFullConfigToEngineIni()
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: GConfig not available - cannot write full configuration"));
		return;
	}
	const FEOSArtifact ActiveArtifact = GetActiveArtifact();
	if (!IsArtifactComplete(ActiveArtifact))
	{
#if WITH_EDITOR
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("EOSKit", "EOSKitInvalidArtifactMsg", "EOSKit auto configuration requires a fully populated artifact.\n\nPlease fill in: Product Id, Sandbox Id, Deployment Id, Client Id, Client Secret, and Encryption Key before enabling 'Automatically Setup EOK'."));
#endif
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Aborting auto configuration - active artifact incomplete."));
		return;
	}
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const TCHAR* SettingsSection = TEXT("/Script/EOSKitShared.EOSKitSettings");
	// Clean legacy section
	GConfig->EmptySection(TEXT("/Script/EOSKit.EOSKitSettings"), ConfigPath);
	// Core & product
	GConfig->SetBool(SettingsSection, TEXT("bAutomaticallySetupEIK"), bAutomaticallySetupEIK, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bAutoLaunchDevTool"), bAutoLaunchDevTool, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("OrganizationName"), *OrganizationName, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("ProductName"), *ProductName, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("ProductVersion"), *ProductVersion, ConfigPath);
	// Google
	GConfig->SetBool(SettingsSection, TEXT("bEnableGoogleOneTap"), bEnableGoogleOneTap, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bEnableGooglePlayGames"), bEnableGooglePlayGames, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("GooglePlayGamesAppId"), *GooglePlayGamesAppId, ConfigPath);
	// Auto-login
	GConfig->SetBool(SettingsSection, TEXT("bEnableAutoLogin"), bEnableAutoLogin, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("AutoLoginCredentialType"), *AutoLoginCredentialType, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("AutoLoginCredentialId"), *AutoLoginCredentialId, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("AutoLoginCredentialToken"), *AutoLoginCredentialToken, ConfigPath);
	// EOS platform
	GConfig->SetInt(SettingsSection, TEXT("TickBudgetInMilliseconds"), TickBudgetInMilliseconds, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bRequireEpicGamesStoreLaunch"), bRequireEpicGamesStoreLaunch, ConfigPath);
	// Overlay
	GConfig->SetBool(SettingsSection, TEXT("bEnableOverlay"), bEnableOverlay, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bEnableSocialOverlay"), bEnableSocialOverlay, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bEnableEditorOverlay"), bEnableEditorOverlay, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("ReturnLevelName"), *ReturnLevelName, ConfigPath);
	// Features
	GConfig->SetBool(SettingsSection, TEXT("bEnableTitleStorage"), bEnableTitleStorage, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bEnablePlayerTicketing"), bEnablePlayerTicketing, ConfigPath);
	// Artifact names
	GConfig->SetString(SettingsSection, TEXT("DefaultArtifactName"), *DefaultArtifactName, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("VoiceArtifactName"), *VoiceArtifactName, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("DedicatedServerArtifactName"), *DedicatedServerArtifactName, ConfigPath);
	// Platform specific
	GConfig->SetString(SettingsSection, TEXT("AndroidArtifactName"), *AndroidArtifactName, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("iOSArtifactName"), *iOSArtifactName, ConfigPath);
	// One click deploy
	GConfig->SetBool(SettingsSection, TEXT("bEnableOneClickDeploy"), bEnableOneClickDeploy, ConfigPath);
	// Legacy
	GConfig->SetString(SettingsSection, TEXT("ProductId"), *ProductId, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("SandboxId"), *SandboxId, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("DeploymentId"), *DeploymentId, ConfigPath);
	GConfig->SetString(SettingsSection, TEXT("EncryptionKey"), *EncryptionKey, ConfigPath);
	// Auth scopes
	GConfig->SetBool(SettingsSection, TEXT("bAuthScope_BasicProfile"), bAuthScope_BasicProfile, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bAuthScope_FriendsList"), bAuthScope_FriendsList, ConfigPath);
	GConfig->SetBool(SettingsSection, TEXT("bAuthScope_Presence"), bAuthScope_Presence, ConfigPath);
	// Clear artifacts then write with +Artifacts
	TArray<FString> ExistingArtifacts;
	GConfig->GetArray(SettingsSection, TEXT("Artifacts"), ExistingArtifacts, ConfigPath);
	for (int32 i = 0; i < ExistingArtifacts.Num(); ++i)
	{
		GConfig->RemoveKey(SettingsSection, TEXT("Artifacts"), ConfigPath);
	}
	UE_LOG(LogTemp, Log, TEXT("EOSKit: Writing %d artifact(s) to config:"), Artifacts.Num());
	for (int32 Index = 0; Index < Artifacts.Num(); ++Index)
	{
		const FEOSArtifact& Artifact = Artifacts[Index];
		FString ArtifactString = FString::Printf(TEXT("(ArtifactName=\"%s\",ClientId=\"%s\",ClientSecret=\"%s\",ProductId=\"%s\",SandboxId=\"%s\",DeploymentId=\"%s\",EncryptionKey=\"%s\")"), *Artifact.ArtifactName, *Artifact.ClientId, *Artifact.ClientSecret, *Artifact.ProductId, *Artifact.SandboxId, *Artifact.DeploymentId, *Artifact.EncryptionKey);
		GConfig->SetString(SettingsSection, TEXT("+Artifacts"), *ArtifactString, ConfigPath);
		UE_LOG(LogTemp, Log, TEXT("  [%d] Artifact: %s"), Index, *Artifact.ArtifactName);
	}
	// OnlineSubsystem toggles
	GConfig->SetBool(TEXT("OnlineSubsystemEOS"), TEXT("bEnabled"), false, ConfigPath);
	GConfig->SetBool(TEXT("OnlineSubsystemEOSKit"), TEXT("bEnabled"), true, ConfigPath);
	GConfig->RemoveKey(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), ConfigPath);
	GConfig->SetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), TEXT("EOSKit"), ConfigPath);
	// Net driver
	const TCHAR* GameEngineSection = TEXT("/Script/Engine.GameEngine");
	TArray<FString> NetDriverDefinitions;
	NetDriverDefinitions.Add(TEXT("(DefName=\"GameNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOSKit.NetDriverEOS\",DriverClassNameFallback=\"/Script/OnlineSubsystemUtils.IpNetDriver\")"));
	GConfig->SetArray(GameEngineSection, TEXT("NetDriverDefinitions"), NetDriverDefinitions, ConfigPath);
	GConfig->SetBool(TEXT("/Script/OnlineSubsystemEOSKit.NetDriverEOS"), TEXT("bIsUsingP2PSockets"), true, ConfigPath);
	
	// OnlineSessionEOSKit settings
	GConfig->SetBool(TEXT("/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit"), TEXT("bUseLobbies"), false, ConfigPath);
	GConfig->SetBool(TEXT("/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit"), TEXT("bUseP2PNetworking"), true, ConfigPath);
	
	GConfig->Flush(false, ConfigPath);
	UE_LOG(LogTemp, Warning, TEXT("EOSKit: Full configuration written - restart editor."));
}
#endif // WITH_EDITOR

FEOSArtifact UEOSKitSettings::GetActiveArtifact() const
{
	FString EffectiveDefaultName = ResolveArtifactName(DefaultArtifactName, Artifacts);
	FString PlatformArtifactName;
#if PLATFORM_ANDROID
	PlatformArtifactName = AndroidArtifactName;
#elif PLATFORM_IOS
	PlatformArtifactName = iOSArtifactName;
#else
	PlatformArtifactName = EffectiveDefaultName;
#endif

	if (IsRunningDedicatedServer() && !DedicatedServerArtifactName.IsEmpty())
	{
		PlatformArtifactName = DedicatedServerArtifactName;
	}

	if (PlatformArtifactName.IsEmpty())
	{
		PlatformArtifactName = EffectiveDefaultName;
	}

	const FEOSArtifact* FirstCompleteArtifact = nullptr;
	const FEOSArtifact* FirstNamedArtifact = nullptr;

	for (const FEOSArtifact& Artifact : Artifacts)
	{
		if (!FirstNamedArtifact && !Artifact.ArtifactName.IsEmpty())
		{
			FirstNamedArtifact = &Artifact;
		}
		if (!FirstCompleteArtifact && IsArtifactComplete(Artifact))
		{
			FirstCompleteArtifact = &Artifact;
		}

		if (!PlatformArtifactName.IsEmpty() && Artifact.ArtifactName == PlatformArtifactName)
		{
			return Artifact;
		}
	}

	if (FirstCompleteArtifact)
	{
		return *FirstCompleteArtifact;
	}
	if (FirstNamedArtifact)
	{
		return *FirstNamedArtifact;
	}

	FEOSArtifact LegacyArtifact;
	LegacyArtifact.ArtifactName = PlatformArtifactName.IsEmpty() ? EffectiveDefaultName : PlatformArtifactName;
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
	return FEOSArtifact();
}

TArray<FString> UEOSKitSettings::GetEnabledAuthScopeFlags() const
{
	TArray<FString> EnabledFlags;
	if (bAuthScope_BasicProfile) { EnabledFlags.Add(TEXT("BasicProfile")); }
	if (bAuthScope_FriendsList) { EnabledFlags.Add(TEXT("FriendsList")); }
	if (bAuthScope_Presence) { EnabledFlags.Add(TEXT("Presence")); }
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

bool UEOSKitSettings::GetSettingsForArtifactFromIni(const FString& ArtifactName, FEOSArtifact& OutSettings)
{
	return UEOSKitSettings::ManualGetSettingsForArtifact(ArtifactName, OutSettings);
}

bool UEOSKitSettings::ManualGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings)
{
	static TOptional<FString> CachedDefaultArtifactName;
	static TOptional<TArray<FEOSArtifact>> CachedArtifactSettings;
	if (!CachedArtifactSettings.IsSet())
	{
		CachedArtifactSettings.Emplace();
		TArray<FString> ArtifactsLines;
		GConfig->GetArray(INI_SECTION, TEXT("Artifacts"), ArtifactsLines, GEngineIni);
		for (const FString& Line : ArtifactsLines)
		{
			FEOSArtifact Artifact; Artifact.ParseRawArrayEntry(Line); CachedArtifactSettings->Add(Artifact);
		}
	}
	if (!CachedDefaultArtifactName.IsSet())
	{
		CachedDefaultArtifactName.Emplace();
		GConfig->GetString(INI_SECTION, TEXT("DefaultArtifactName"), *CachedDefaultArtifactName, GEngineIni);
		if (CachedDefaultArtifactName->IsEmpty())
		{
			*CachedDefaultArtifactName = GetFirstValidArtifactName(*CachedArtifactSettings);
		}
	}
	FString ArtifactNameOverride;
	FString DeploymentIdOverride;
	FString SandboxIdOverride;
	FParse::Value(FCommandLine::Get(), TEXT("EOSArtifactNameOverride="), ArtifactNameOverride);
	if (ArtifactNameOverride.IsEmpty())
	{
		ArtifactNameOverride = ArtifactName;
	}
	if (ArtifactNameOverride.IsEmpty() && IsRunningDedicatedServer())
	{
		FString DedicatedServerName;
		GConfig->GetString(INI_SECTION, TEXT("DedicatedServerArtifactName"), DedicatedServerName, GEngineIni);
		ArtifactNameOverride = DedicatedServerName;
	}
	if (ArtifactNameOverride.IsEmpty())
	{
		ArtifactNameOverride = *CachedDefaultArtifactName;
	}
	FParse::Value(FCommandLine::Get(), TEXT("epicdeploymentid="), DeploymentIdOverride);
	FParse::Value(FCommandLine::Get(), TEXT("epicsandboxid="), SandboxIdOverride);

	const FEOSArtifact* FirstCompleteArtifact = nullptr;
	const FEOSArtifact* FirstNamedArtifact = nullptr;

	for (const FEOSArtifact& Artifact : *CachedArtifactSettings)
	{
		if (!FirstNamedArtifact && !Artifact.ArtifactName.IsEmpty())
		{
			FirstNamedArtifact = &Artifact;
		}
		if (!FirstCompleteArtifact && IsArtifactComplete(Artifact))
		{
			FirstCompleteArtifact = &Artifact;
		}
		if (Artifact.ArtifactName == ArtifactNameOverride)
		{
			OutSettings = Artifact;
			if (!DeploymentIdOverride.IsEmpty()) { OutSettings.DeploymentId = DeploymentIdOverride; }
			if (!SandboxIdOverride.IsEmpty()) { OutSettings.SandboxId = SandboxIdOverride; }
			return true;
		}
	}

	const FEOSArtifact* FallbackArtifact = FirstCompleteArtifact ? FirstCompleteArtifact : FirstNamedArtifact;
	if (FallbackArtifact)
	{
		OutSettings = *FallbackArtifact;
		if (!DeploymentIdOverride.IsEmpty()) { OutSettings.DeploymentId = DeploymentIdOverride; }
		if (!SandboxIdOverride.IsEmpty()) { OutSettings.SandboxId = SandboxIdOverride; }
		return true;
	}
	return false;
}

bool UEOSKitSettings::AutoGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings)
{
	const UEOSKitSettings* This = GetDefault<UEOSKitSettings>();
	FString DeploymentIdOverride; FParse::Value(FCommandLine::Get(), TEXT("epicdeploymentid="), DeploymentIdOverride);
	FString SandboxIdOverride; FParse::Value(FCommandLine::Get(), TEXT("epicsandboxid="), SandboxIdOverride);
	FString ArtifactNameOverride; FParse::Value(FCommandLine::Get(), TEXT("EOSArtifactNameOverride="), ArtifactNameOverride);
	if (ArtifactNameOverride.IsEmpty())
	{
		if (IsRunningDedicatedServer() && !This->DedicatedServerArtifactName.IsEmpty())
		{
			ArtifactNameOverride = This->DedicatedServerArtifactName;
		}
		else if (!ArtifactName.IsEmpty())
		{
			ArtifactNameOverride = ArtifactName;
		}
		else
		{
			ArtifactNameOverride = ResolveArtifactName(This->DefaultArtifactName, This->Artifacts);
		}
	}
	const FEOSArtifact* FirstCompleteArtifact = nullptr;
	const FEOSArtifact* FirstNamedArtifact = nullptr;
	for (const FEOSArtifact& Artifact : This->Artifacts)
	{
		if (!FirstNamedArtifact && !Artifact.ArtifactName.IsEmpty())
		{
			FirstNamedArtifact = &Artifact;
		}
		if (!FirstCompleteArtifact && IsArtifactComplete(Artifact))
		{
			FirstCompleteArtifact = &Artifact;
		}
		if (Artifact.ArtifactName == ArtifactNameOverride)
		{
			OutSettings = Artifact;
			if (!DeploymentIdOverride.IsEmpty()) { OutSettings.DeploymentId = DeploymentIdOverride; }
			if (!SandboxIdOverride.IsEmpty()) { OutSettings.SandboxId = SandboxIdOverride; }
			return true;
		}
	}
	const FEOSArtifact* FallbackArtifact = FirstCompleteArtifact ? FirstCompleteArtifact : FirstNamedArtifact;
	if (FallbackArtifact)
	{
		OutSettings = *FallbackArtifact;
		if (!DeploymentIdOverride.IsEmpty()) { OutSettings.DeploymentId = DeploymentIdOverride; }
		if (!SandboxIdOverride.IsEmpty()) { OutSettings.SandboxId = SandboxIdOverride; }
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to find artifact settings. Please check Project Settings under EOS Kit"));
	return false;
}

#undef LOCTEXT_NAMESPACE
