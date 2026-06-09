// Copyright © 2025 Asrock Studios.
// All rights reserved.

using UnrealBuildTool;

public class EOKWeb : ModuleRules
{
    public EOKWeb(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDefinitions.Add("EOKWeb_API=DLLEXPORT");
        PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
        PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "HTTP",
                "JsonUtilities",
                "Json",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
    }
}