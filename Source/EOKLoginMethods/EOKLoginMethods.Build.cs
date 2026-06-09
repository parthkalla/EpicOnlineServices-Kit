// Copyright © 2025 Asrock Studios.
// All rights reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class EOKLoginMethods : ModuleRules
{
    public EOKLoginMethods(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDefinitions.Add("EOKLoginMethods_API=DLLEXPORT");
        PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
        PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "Launch" });

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            Console.WriteLine("PluginPath: " + PluginPath);
            //AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "GoogleOneTap_UPL.xml"));
        }
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "OnlineSubsystemEOK",
                "GoogleOneTapLibrary",
                "GooglePlayGamesLibrary",
                "Json",
                "JsonUtilities"
            }
        );
    }
}