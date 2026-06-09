// Copyright © 2025 Asrock Studios.
// All rights reserved.

using UnrealBuildTool;

public class EOKEditor : ModuleRules
{
    public EOKEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDefinitions.Add("EOKEditor_API=DLLEXPORT");
        PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
        PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "MainFrame", "EditorStyle",
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "UnrealEd",
                "LevelEditor",
                "ToolMenus",
                "Projects",
                "UnrealEd", 
                "LauncherServices",
                "EpicOnlineServicesKitV2",
            }
        );
    }
}