// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#include "EpicOnlineServicesKitV2.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Launch/Resources/Version.h"
#define LOCTEXT_NAMESPACE "FEpicOnlineServicesKitV2Module"


DEFINE_LOG_CATEGORY(LogEpicOnlineServicesKitV2);
void FEpicOnlineServicesKitV2Module::StartupModule()
{
    ConfigureOnlineSubsystemEOK();
#if WITH_EDITOR && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
    //Special love and respect to Redpoint Dev but this had to be added as a warning and you know why 🙏
    if(IPluginManager::Get().FindPlugin("EOSCore") != nullptr && IPluginManager::Get().FindPlugin("EOSCore")->IsEnabled())
    {
        const FText Message = LOCTEXT("EpicOnlineServicesKitV2Error","Epic Online Services-Kit V2 Error");
        FMessageDialog::Open(EAppMsgCategory::Warning,EAppMsgType::Ok,LOCTEXT("Epic Online Services-Kit V2 Error","EOSCore Plugin is enabled which will not allow Epic Online Services-Kit V2 to work properly. Please disable EOSCore Plugin and restart the editor. The basic reasoning behind this is that the EOS SDK is already included in our plugin and you cannot include the SDK twice."));
    }
    if(IPluginManager::Get().FindPlugin("OnlineSubsystemRedpoint") != nullptr && IPluginManager::Get().FindPlugin("OnlineSubsystemRedpoint")->IsEnabled())
    {
        const FText Message = LOCTEXT("EpicOnlineServicesKitV2Error","Epic Online Services-Kit V2 Error");
        FMessageDialog::Open(EAppMsgCategory::Warning,EAppMsgType::Ok,LOCTEXT("Epic Online Services-Kit V2 Error","OnlineSubsystemRedpoint Plugin is enabled which will not allow Epic Online Services-Kit V2 to work properly. Please disable OnlineSubsystemRedpoint Plugin and restart the editor. The basic reasoning behind this is that the EOS SDK is already included in our plugin and you cannot include the SDK twice."));
    }
#endif
}

void FEpicOnlineServicesKitV2Module::ConfigureOnlineSubsystemEOK() const
{
    FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
    FString EngineIniText;

    if (FFileHelper::LoadFileToString(EngineIniText, *EngineIniPath))
    {
        FString SectionName = TEXT("/Script/EpicOnlineServicesKitV2.EOKSettings");
        FString KeyName = TEXT("bAutomaticallySetupEOK");
        bool bAutomaticallySetupEOK = false;
        // Read the value from the ini file
        if (GConfig->GetBool(*SectionName, *KeyName, bAutomaticallySetupEOK, GEngineIni) && bAutomaticallySetupEOK)
        {
            UE_LOG(LogEpicOnlineServicesKitV2, Log, TEXT("Automatically setting up OnlineSubsystemEOK"));
            bool bConfigChanged = false;

            // Check if [OnlineSubsystemEOK] section exists and add it if not
            if (!EngineIniText.Contains(TEXT("[OnlineSubsystemEOK]")))
            {
                EngineIniText += TEXT("\n[OnlineSubsystemEOK]\nbEnabled=true\n");
                bConfigChanged = true;
            }

            // Update [OnlineSubsystem] section
            FString OnlineSubsystemSection = TEXT("[OnlineSubsystem]");
            if (EngineIniText.Contains(OnlineSubsystemSection))
            {
                // Find the existing DefaultPlatformService entry and update it if needed
                int32 SectionStart = EngineIniText.Find(OnlineSubsystemSection);
                int32 SectionEnd = EngineIniText.Find(TEXT("\n["), ESearchCase::IgnoreCase, ESearchDir::FromStart, SectionStart + 1);

                if (SectionEnd == INDEX_NONE)
                {
                    SectionEnd = EngineIniText.Len();
                }

                FString OnlineSubsystemContent = EngineIniText.Mid(SectionStart, SectionEnd - SectionStart);
                if (!OnlineSubsystemContent.Contains(TEXT("DefaultPlatformService=EOK")))
                {
                    int32 DefaultPlatformServiceIndex = OnlineSubsystemContent.Find(TEXT("DefaultPlatformService="));
                    if (DefaultPlatformServiceIndex != INDEX_NONE)
                    {
                        // Replace the existing DefaultPlatformService value
                        int32 LineEndIndex = OnlineSubsystemContent.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, DefaultPlatformServiceIndex);
                        if (LineEndIndex == INDEX_NONE)
                        {
                            LineEndIndex = OnlineSubsystemContent.Len();
                        }

                        FString ExistingLine = OnlineSubsystemContent.Mid(DefaultPlatformServiceIndex, LineEndIndex - DefaultPlatformServiceIndex);
                        OnlineSubsystemContent.ReplaceInline(*ExistingLine, TEXT("DefaultPlatformService=EOK"));
                    }
                    else
                    {
                        // Add the DefaultPlatformService setting
                        OnlineSubsystemContent += TEXT("\nDefaultPlatformService=EOK");
                    }
                    EngineIniText.ReplaceInline(*EngineIniText.Mid(SectionStart, SectionEnd - SectionStart), *OnlineSubsystemContent);
                    bConfigChanged = true;
                }
            }
            else
            {
                // Add the [OnlineSubsystem] section
                EngineIniText += TEXT("\n[OnlineSubsystem]\nDefaultPlatformService=EOK\n");
                bConfigChanged = true;
            }

            // Update [/Script/OnlineSubsystemEOK.NetDriverEOK] section
            if (!EngineIniText.Contains(TEXT("[/Script/OnlineSubsystemEOK.NetDriverEOK]")))
            {
                FString Comment = TEXT("\n;EOK Comment: You do not need to worry about this setting as we dynamically set it in Travel URL depending upon if we are using Listen Server or Dedicated Server\n");
                EngineIniText += Comment;
                EngineIniText += TEXT("[/Script/OnlineSubsystemEOK.NetDriverEOK]\nbIsUsingP2PSockets=true\n");
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
                    TEXT("!NetDriverDefinitions=ClearArray\n+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOK.NetDriverEOK\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n+NetDriverDefinitions=(DefName=\"BeaconNetDriver\",DriverClassName=\"/Script/OnlineSubsystemEOK.NetDriverEOK\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n")
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
                    TEXT("!NetDriverDefinitions=ClearArray\n+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"OnlineSubsystemEOK.NetDriverEOK\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")\n")
                );
                EngineIniText += NetDriverDefinitions;

                bConfigChanged = true;
            }
#endif

            // Add OnlineEngineInterfaceImpl section for EOS compatible net ID types
            FString OnlineEngineInterfaceImplSection = TEXT("[/Script/OnlineSubsystemUtils.OnlineEngineInterfaceImpl]");
            if (!EngineIniText.Contains(OnlineEngineInterfaceImplSection))
            {
                EngineIniText += TEXT("\n") + OnlineEngineInterfaceImplSection + TEXT("\n");
                EngineIniText += TEXT("+CompatibleUniqueNetIdTypes=EOS\n");
                EngineIniText += TEXT("+CompatibleUniqueNetIdTypes=EOSPlus\n");
                bConfigChanged = true;
            }
            else
            {
                // Section exists, check if EOS types are present
                int32 SectionStart = EngineIniText.Find(OnlineEngineInterfaceImplSection);
                int32 SectionEnd = EngineIniText.Find(TEXT("\n["), ESearchCase::IgnoreCase, ESearchDir::FromStart, SectionStart + 1);
                if (SectionEnd == INDEX_NONE)
                {
                    SectionEnd = EngineIniText.Len();
                }
                FString SectionContent = EngineIniText.Mid(SectionStart, SectionEnd - SectionStart);
                if (!SectionContent.Contains(TEXT("CompatibleUniqueNetIdTypes=EOS")))
                {
                    // Insert EOS types into existing section
                    int32 InsertPos = SectionEnd;
                    EngineIniText.InsertAt(InsertPos, TEXT("+CompatibleUniqueNetIdTypes=EOS\n+CompatibleUniqueNetIdTypes=EOSPlus\n"));
                    bConfigChanged = true;
                }
            }

            // Add NetworkSettings section for client-server travel
            FString NetworkSettingsSection = TEXT("[/Script/Engine.NetworkSettings]");
            if (!EngineIniText.Contains(NetworkSettingsSection))
            {
                EngineIniText += TEXT("\n") + NetworkSettingsSection + TEXT("\n");
                EngineIniText += TEXT("bAllowClientServerTravel=true\n");
                bConfigChanged = true;
            }
            else
            {
                // Section exists, check if bAllowClientServerTravel is present
                int32 SectionStart = EngineIniText.Find(NetworkSettingsSection);
                int32 SectionEnd = EngineIniText.Find(TEXT("\n["), ESearchCase::IgnoreCase, ESearchDir::FromStart, SectionStart + 1);
                if (SectionEnd == INDEX_NONE)
                {
                    SectionEnd = EngineIniText.Len();
                }
                FString SectionContent = EngineIniText.Mid(SectionStart, SectionEnd - SectionStart);
                if (!SectionContent.Contains(TEXT("bAllowClientServerTravel")))
                {
                    int32 InsertPos = SectionEnd;
                    EngineIniText.InsertAt(InsertPos, TEXT("bAllowClientServerTravel=true\n"));
                    bConfigChanged = true;
                }
            }

            // Add ConsoleVariables section for network handshake
            FString ConsoleVariablesSection = TEXT("[ConsoleVariables]");
            if (!EngineIniText.Contains(ConsoleVariablesSection))
            {
                EngineIniText += TEXT("\n") + ConsoleVariablesSection + TEXT("\n");
                EngineIniText += TEXT("net.CurrentHandshakeVersion=2\n");
                EngineIniText += TEXT("net.MinHandshakeVersion=2\n");
                bConfigChanged = true;
            }
            else
            {
                // Section exists, check if handshake versions are present
                int32 SectionStart = EngineIniText.Find(ConsoleVariablesSection);
                int32 SectionEnd = EngineIniText.Find(TEXT("\n["), ESearchCase::IgnoreCase, ESearchDir::FromStart, SectionStart + 1);
                if (SectionEnd == INDEX_NONE)
                {
                    SectionEnd = EngineIniText.Len();
                }
                FString SectionContent = EngineIniText.Mid(SectionStart, SectionEnd - SectionStart);
                if (!SectionContent.Contains(TEXT("net.CurrentHandshakeVersion")))
                {
                    int32 InsertPos = SectionEnd;
                    EngineIniText.InsertAt(InsertPos, TEXT("net.CurrentHandshakeVersion=2\nnet.MinHandshakeVersion=2\n"));
                    bConfigChanged = true;
                }
            }

            // Add IpNetDriver fallback settings
            FString IpNetDriverSection = TEXT("[/Script/OnlineSubsystemUtils.IpNetDriver]");
            if (!EngineIniText.Contains(IpNetDriverSection))
            {
                EngineIniText += TEXT("\n") + IpNetDriverSection + TEXT("\n");
                EngineIniText += TEXT("; Fallback IP NetDriver settings (used when EOS is unavailable)\n");
                EngineIniText += TEXT("MaxClientRate=1000000000\n");
                EngineIniText += TEXT("MaxInternetClientRate=1000000000\n");
                EngineIniText += TEXT("InitialConnectTimeout=120.0\n");
                bConfigChanged = true;
            }

            // Save the modified text back to the DefaultEngine.ini file if any changes were made
            if (bConfigChanged)
            {
                if (FFileHelper::SaveStringToFile(EngineIniText, *EngineIniPath))
                {
                    UE_LOG(LogEpicOnlineServicesKitV2, Log, TEXT("OnlineSubsystemEOK configuration added/updated in DefaultEngine.ini"));
                }
                else
                {
                    UE_LOG(LogEpicOnlineServicesKitV2, Error, TEXT("Failed to save modified DefaultEngine.ini"));
                }
            }
            else
            {
                UE_LOG(LogEpicOnlineServicesKitV2, Log, TEXT("OnlineSubsystemEOK configuration already exists in DefaultEngine.ini"));
            }
        }
        else
        {
            UE_LOG(LogEpicOnlineServicesKitV2, Log, TEXT("Automatically setting up OnlineSubsystemEOK is disabled"));
        }
    }
    else
    {
        UE_LOG(LogEpicOnlineServicesKitV2, Log, TEXT("Failed to load DefaultEngine.ini"));
    }
}



void FEpicOnlineServicesKitV2Module::ConfigureDedicatedServerConfigEOK()
{
    FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("WindowsServer") / TEXT("Engine.ini");
}

void FEpicOnlineServicesKitV2Module::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FEpicOnlineServicesKitV2Module, EpicOnlineServicesKitV2)