#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EOSKitTypes.h"
#include "EOSKitSettings.generated.h"

/**
 * Login flags for automatic login behavior
 */
USTRUCT(BlueprintType)
struct FEOSLoginFlag
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Config, Category = "Login")
	FString FlagName = TEXT("NoFlags");

	UPROPERTY(EditAnywhere, Config, Category = "Login")
	bool bEnabled = false;
};

/**
 * Artifact configuration for EOS artifacts
 * Each artifact represents a complete set of credentials for an environment (Dev/Staging/Prod)
 */
USTRUCT(BlueprintType)
struct FEOSArtifact
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Config, Category = "Artifact", meta = (DisplayName = "Artifact Name"))
	FString ArtifactName = TEXT("DefaultArtifact");

	UPROPERTY(EditAnywhere, Config, Category = "Artifact", meta = (DisplayName = "Client ID"))
	FString ClientId;

	UPROPERTY(EditAnywhere, Config, Category = "Artifact", meta = (DisplayName = "Client Secret"))
	FString ClientSecret;

	UPROPERTY(EditAnywhere, Config, Category = "Artifact", meta = (DisplayName = "Product ID"))
	FString ProductId;

	UPROPERTY(EditAnywhere, Config, Category = "Artifact", meta = (DisplayName = "Sandbox ID"))
	FString SandboxId;

	UPROPERTY(EditAnywhere, Config, Category = "Artifact", meta = (DisplayName = "Deployment ID"))
	FString DeploymentId;

	UPROPERTY(EditAnywhere, Config, Category = "Artifact", meta = (DisplayName = "Encryption Key"))
	FString EncryptionKey;

	/** Parse artifact from raw config file entry */
	void ParseRawArrayEntry(const FString& RawLine);
};

/**
 * Settings for the EOSKit plugin that can be configured in Project Settings
 */
UCLASS(Config=Engine, DefaultConfig, meta=(DisplayName="EOS Kit"))
class EOSKIT_API UEOSKitSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEOSKitSettings();

	// ========== EOS Integration Kit Settings ==========
	
	/** Automatically setup EIK on project startup */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Integration Kit Settings", meta = (DisplayName = "Automatically Setup EOK"))
	bool bAutomaticallySetupEIK = true;

	/** Auto launch the Dev Auth tool for testing */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Integration Kit Settings", meta = (DisplayName = "Auto Launch Dev Tool"))
	bool bAutoLaunchDevTool = false;

	/** Organization name for your EOS application */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Integration Kit Settings", meta = (DisplayName = "Organization Name"))
	FString OrganizationName;

	/** Product name for your EOS application */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Integration Kit Settings", meta = (DisplayName = "Product Name"))
	FString ProductName = TEXT("MyEOSGame");

	/** Enable Google One Tap sign-in */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Integration Kit Settings", meta = (DisplayName = "Enable Google One Tap"))
	bool bEnableGoogleOneTap = false;

	/** Enable Google Play Games sign-in */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Integration Kit Settings", meta = (DisplayName = "Enable Google Play Games"))
	bool bEnableGooglePlayGames = false;

	/** Google Play Games App ID */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Integration Kit Settings", meta = (DisplayName = "Google Play Games App ID", EditCondition = "bEnableGooglePlayGames"))
	FString GooglePlayGamesAppId;

	// ========== EOS Settings ==========

	/** Tick budget in milliseconds for EOS platform updates */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Settings", meta = (DisplayName = "Tick Budget in Milliseconds", ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	int32 TickBudgetInMilliseconds = 0;

	/** Require being launched by the Epic Games Store */
	UPROPERTY(Config, EditAnywhere, Category = "EOS Settings", meta = (DisplayName = "Require Being Launched by the Epic Games Store"))
	bool bRequireEpicGamesStoreLaunch = false;

	// ========== Login Settings ==========

	/** Enable Basic Profile auth scope - allows access to basic user profile information */
	UPROPERTY(Config, EditAnywhere, Category = "Login Settings|Auth Scope Flags", meta = (DisplayName = "Basic Profile"))
	bool bAuthScope_BasicProfile = true;

	/** Enable Friends List auth scope - allows access to user's friends list */
	UPROPERTY(Config, EditAnywhere, Category = "Login Settings|Auth Scope Flags", meta = (DisplayName = "Friends List"))
	bool bAuthScope_FriendsList = false;

	/** Enable Presence auth scope - allows access to user's online presence/status */
	UPROPERTY(Config, EditAnywhere, Category = "Login Settings|Auth Scope Flags", meta = (DisplayName = "Presence"))
	bool bAuthScope_Presence = false;

	/** Enable automatic login on startup */
	UPROPERTY(Config, EditAnywhere, Category = "Login Settings|Auto Login", meta = (DisplayName = "Enable Auto Login"))
	bool bEnableAutoLogin = false;

	/** Credential type for auto login (AccountPortal, Developer, PersistentAuth, etc) */
	UPROPERTY(Config, EditAnywhere, Category = "Login Settings|Auto Login", meta = (DisplayName = "Auto Login Credential Type", EditCondition = "bEnableAutoLogin"))
	FString AutoLoginCredentialType = TEXT("AccountPortal");

	/** Credential ID for auto login (if needed) */
	UPROPERTY(Config, EditAnywhere, Category = "Login Settings|Auto Login", meta = (DisplayName = "Auto Login Credential ID", EditCondition = "bEnableAutoLogin"))
	FString AutoLoginCredentialId;

	/** Credential token for auto login (if needed) */
	UPROPERTY(Config, EditAnywhere, Category = "Login Settings|Auto Login", meta = (DisplayName = "Auto Login Credential Token", EditCondition = "bEnableAutoLogin"))
	FString AutoLoginCredentialToken;

	// ========== Player Ticketing Settings ==========

	/** Enable player ticketing features */
	UPROPERTY(Config, EditAnywhere, Category = "Player Ticketing Settings", meta = (DisplayName = "Enable Player Ticketing"))
	bool bEnablePlayerTicketing = false;

	// ========== Title Storage Settings ==========

	/** Enable title storage features */
	UPROPERTY(Config, EditAnywhere, Category = "Title Storage Settings", meta = (DisplayName = "Enable Title Storage"))
	bool bEnableTitleStorage = true;

	// ========== Overlay Settings ==========

	/** Enable the EOS overlay */
	UPROPERTY(Config, EditAnywhere, Category = "Overlay Settings", meta = (DisplayName = "Enable Overlay"))
	bool bEnableOverlay = true;

	/** Enable social overlay */
	UPROPERTY(Config, EditAnywhere, Category = "Overlay Settings", meta = (DisplayName = "Enable Social Overlay", EditCondition = "bEnableOverlay"))
	bool bEnableSocialOverlay = true;

	/** Enable editor overlay */
	UPROPERTY(Config, EditAnywhere, Category = "Overlay Settings", meta = (DisplayName = "Enable Editor Overlay"))
	bool bEnableEditorOverlay = true;

	/** Return level name for overlay */
	UPROPERTY(Config, EditAnywhere, Category = "Overlay Settings", meta = (DisplayName = "Return Level Name"))
	FString ReturnLevelName;

	// ========== Artifact Settings ==========

	/** Default artifact name to use */
	UPROPERTY(Config, EditAnywhere, Category = "Artifact Settings", meta = (DisplayName = "Default Artifact Name"))
	FString DefaultArtifactName = TEXT("SummaGrandeClient");

	/** Voice artifact name */
	UPROPERTY(Config, EditAnywhere, Category = "Artifact Settings", meta = (DisplayName = "Voice Artifact Name"))
	FString VoiceArtifactName = TEXT("DefaultArtifact");

	/** Dedicated server artifact name */
	UPROPERTY(Config, EditAnywhere, Category = "Artifact Settings", meta = (DisplayName = "Dedicated Server Artifact Name"))
	FString DedicatedServerArtifactName = TEXT("DefaultArtifact");

	/** List of configured artifacts */
	UPROPERTY(Config, EditAnywhere, Category = "Artifact Settings|Artifacts", meta = (DisplayName = "Artifacts"))
	TArray<FEOSArtifact> Artifacts;

	// ========== Platform Specific ==========

	/** Platform-specific artifact name for Android */
	UPROPERTY(Config, EditAnywhere, Category = "Platform Specific|Android", meta = (DisplayName = "Platform Specific Artifact Name"))
	FString AndroidArtifactName = TEXT("DefaultArtifact");

	/** Platform-specific artifact name for iOS */
	UPROPERTY(Config, EditAnywhere, Category = "Platform Specific|iOS", meta = (DisplayName = "Platform Specific Artifact Name"))
	FString iOSArtifactName = TEXT("DefaultArtifact");

	// ========== One Click Deploy ==========

	/** Enable one-click deploy features */
	UPROPERTY(Config, EditAnywhere, Category = "One Click Deploy", meta = (DisplayName = "Enable One Click Deploy"))
	bool bEnableOneClickDeploy = false;

	// ========== DEPRECATED Legacy Fields (Kept for backward compatibility - Use Artifacts instead) ==========
	
	/** @deprecated Use Artifacts array instead */
	UPROPERTY(Config, meta = (DeprecatedProperty, DeprecationMessage = "Please use the Artifacts array in Artifact Settings instead"))
	FString ProductId;

	/** @deprecated Use Artifacts array instead */
	UPROPERTY(Config, meta = (DeprecatedProperty, DeprecationMessage = "Please use the Artifacts array in Artifact Settings instead"))
	FString SandboxId;

	/** @deprecated Use Artifacts array instead */
	UPROPERTY(Config, meta = (DeprecatedProperty, DeprecationMessage = "Please use the Artifacts array in Artifact Settings instead"))
	FString DeploymentId;
	
	/** @deprecated Use Artifacts array instead */
	UPROPERTY(Config, meta = (DeprecatedProperty, DeprecationMessage = "Please use the Artifacts array in Artifact Settings instead"))
	FString EncryptionKey;

	/** Product version */
	UPROPERTY(Config, EditAnywhere, Category = "General", meta = (DisplayName = "Product Version"))
	FString ProductVersion = TEXT("1.0");

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** Helper function to get enabled auth scope flags as an array */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Settings")
	TArray<FString> GetEnabledAuthScopeFlags() const;

	/** Find the Settings for an artifact by name */
	static bool GetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings);

	/** Get the active artifact based on current platform and configuration */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Settings")
	FEOSArtifact GetActiveArtifact() const;

	/** Get an artifact by name */
	UFUNCTION(BlueprintCallable, Category = "EOSKit|Settings")
	FEOSArtifact GetArtifactByName(const FString& ArtifactName) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
	
	/** Show restart warning dialog (EIK style) */
	static EAppReturnType::Type ShowRestartWarning(const FText& Title);
	
	/** Validate artifact credentials */
	void ValidateArtifacts();
	
	/** Update OnlineSubsystem configuration in DefaultEngine.ini */
	void UpdateOnlineSubsystemConfig();
	
	/** Helper function to generate a 64-character hex encryption key */
	static FString GenerateRandomEncryptionKey();
#endif

private:
#if WITH_EDITOR
	void SaveToDefaultEngineIni();
	void WriteFullConfigToEngineIni();
#endif

	static bool AutoGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings);
	static bool ManualGetSettingsForArtifact(const FString& ArtifactName, FEOSArtifact& OutSettings);
};
