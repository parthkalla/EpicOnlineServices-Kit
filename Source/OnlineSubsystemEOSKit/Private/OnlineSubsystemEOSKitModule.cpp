// Copyright (C) 2024, All Rights Reserved.

#include "OnlineSubsystemEOSKitModule.h"
#include "OnlineSubsystemModule.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitSettings.h"
#include "NetDriverEOS.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Features/IModularFeature.h"
#include "Features/IModularFeatures.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/LazySingleton.h"
#include "Misc/MessageDialog.h"

#if WITH_EDITOR
	#include "ISettingsModule.h"
	#include "ISettingsSection.h"
	#include "ToolMenus.h"
#endif

#define LOCTEXT_NAMESPACE "EOSKit"

IMPLEMENT_MODULE(FOnlineSubsystemEOSKitModule, OnlineSubsystemEOSKit);

/**
 * Class responsible for creating instance(s) of the subsystem
 */
class FOnlineFactoryEOSKit :
	public IOnlineFactory
{
public:
	FOnlineFactoryEOSKit() {}
	virtual ~FOnlineFactoryEOSKit() {}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName)
	{
		FOnlineSubsystemEOSKitPtr OnlineSub = MakeShared<FOnlineSubsystemEOSKit, ESPMode::ThreadSafe>(InstanceName);
		if (!OnlineSub->Init())
		{
			UE_LOG_ONLINE(Warning, TEXT("EOSKit API failed to initialize!"));
			OnlineSub->Shutdown();
			OnlineSub = nullptr;
		}

		return OnlineSub;
	}
};

void FOnlineSubsystemEOSKitModule::StartupModule()
{
	if (IsRunningCommandlet())
	{
		return;
	}

	// Force load the NetDriver class to ensure it's available when Unreal Engine tries to select it
	// This ensures the class is registered before NetDriver selection happens
	const UClass* NetDriverClass = UNetDriverEOS::StaticClass();
	if (NetDriverClass)
	{
		// Log the exact script path so we can match it in DefaultEngine.ini
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("EOSKit: NetDriverEOS class loaded and registered. PathName='%s', Name='%s'"),
			*NetDriverClass->GetPathName(),
			*NetDriverClass->GetName()
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EOSKit: Failed to load NetDriverEOS class!"));
	}

	EOSKitFactory = new FOnlineFactoryEOSKit();

	// Create and register our singleton factory with the main online subsystem for easy access
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.RegisterPlatformService("EOSKit", EOSKitFactory);

#if WITH_EDITOR
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOnlineSubsystemEOSKitModule::RegisterMenus));
#endif
#if WITH_EDITOR
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FOnlineSubsystemEOSKitModule::OnPostEngineInit);
	FCoreDelegates::OnPreExit.AddRaw(this, &FOnlineSubsystemEOSKitModule::OnPreExit);
#endif
}

#if WITH_EDITOR
void FOnlineSubsystemEOSKitModule::OnPostEngineInit()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "EOSKit",
			LOCTEXT("EOSKitSettingsName", "EOS Kit"),
			LOCTEXT("EOSKitSettingsDescription", "Configure Epic Online Services integration settings for your project"),
			GetMutableDefault<UEOSKitSettings>());
	}
}
#endif

#if WITH_EDITOR
void FOnlineSubsystemEOSKitModule::OnPreExit()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "EOSKit");
	}
}
#endif

void FOnlineSubsystemEOSKitModule::ShutdownModule()
{
	if (IsRunningCommandlet())
	{
		return;
	}

#if WITH_EDITOR
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
#endif
#if WITH_EDITOR
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	FCoreDelegates::OnPreExit.RemoveAll(this);
#endif

	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService("EOSKit");

	delete EOSKitFactory;
	EOSKitFactory = nullptr;
}

void FOnlineSubsystemEOSKitModule::RegisterMenus()
{
#if WITH_EDITOR
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);
#endif
}

void FOnlineSubsystemEOSKitModule::ConfigureOnlineSubsystemEOSKit()
{
	FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString EngineIniText;
	if (FFileHelper::LoadFileToString(EngineIniText, *EngineIniPath))
	{
		bool bConfigChanged = false;

		// Check if [OnlineSubsystemEOSKit] section exists and add it if not
		if (!EngineIniText.Contains(TEXT("[OnlineSubsystemEOSKit]")))
		{
			EngineIniText += TEXT("\n[OnlineSubsystemEOSKit]\nbEnabled=true\n");
			bConfigChanged = true;
		}

		// Update [OnlineSubsystem] section
		if (!EngineIniText.Contains(TEXT("[OnlineSubsystem]")))
		{
			EngineIniText += TEXT("\n[OnlineSubsystem]\nDefaultPlatformService=EOSKit\n");
			bConfigChanged = true;
		}

		// Update [/Script/OnlineSubsystemEOSKit.NetDriverEOS] section
		if (!EngineIniText.Contains(TEXT("\n[/Script/OnlineSubsystemEOSKit.NetDriverEOS]")))
		{
			FString Comment = TEXT(";EOSKit Comment: You do not need to worry about this setting as we dynamically set it in Travel URL depending upon if we are using Listen Server or Dedicated Server\n");
			EngineIniText += Comment;
			EngineIniText += TEXT("[/Script/OnlineSubsystemEOSKit.NetDriverEOS]\nbIsUsingP2PSockets=true\n");
			bConfigChanged = true;
		}

		// Update [/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit] section
		if (!EngineIniText.Contains(TEXT("[/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit]")))
		{
			EngineIniText += TEXT("\n[/Script/OnlineSubsystemEOSKit.OnlineSessionEOSKit]\nbUseLobbies=false\nbUseP2PNetworking=true\n");
			bConfigChanged = true;
		}

		// Update NetDriverDefinitions section (version-specific)
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
		// UE 5.6+ uses [/Script/Engine.Engine] and requires /Script/ prefix for class paths
		if (!EngineIniText.Contains(TEXT("[/Script/Engine.Engine]")))
		{
			EngineIniText += TEXT("\n[/Script/Engine.Engine]\n");

			// Update NetDriverDefinitions in [/Script/Engine.Engine] section
			FString NetDriverDefinitions = FString::Printf(
				TEXT("!NetDriverDefinitions=ClearArray\n+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOSKit.NetDriverEOS\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n+NetDriverDefinitions=(DefName=\"BeaconNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOSKit.NetDriverEOS\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n")
			);
			EngineIniText += NetDriverDefinitions;
			
			bConfigChanged = true;
		}
#else
		// UE 5.5 and below use [/Script/Engine.GameEngine] and old class path format
		if (!EngineIniText.Contains(TEXT("[/Script/Engine.GameEngine]")))
		{
			EngineIniText += TEXT("\n[/Script/Engine.GameEngine]\n");

			// Update NetDriverDefinitions in [/Script/Engine.GameEngine] section
			FString NetDriverDefinitions = FString::Printf(
				TEXT("!NetDriverDefinitions=ClearArray\n+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"OnlineSubsystemEOSKit.NetDriverEOS\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n")
			);
			EngineIniText += NetDriverDefinitions;
			
			bConfigChanged = true;
		}
#endif

		// Save the modified text back to the DefaultEngine.ini file if any changes were made
		if (bConfigChanged)
		{
			if (FFileHelper::SaveStringToFile(EngineIniText, *EngineIniPath))
			{
				UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystemEOSKit configuration added/updated in DefaultEngine.ini"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to save modified DefaultEngine.ini"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystemEOSKit configuration already exists in DefaultEngine.ini"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load DefaultEngine.ini"));
	}
}

#undef LOCTEXT_NAMESPACE

