using UnrealBuildTool;
using System.IO;

public class EOSKit : ModuleRules
{
    public EOSKit(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // -------- Include Paths --------
        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "Public")
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "Private")
        });

        // Allow EOSKit to include feature headers without a hard DLL dependency. This keeps
        // EOSKitAuth / EOSKitSessions free to depend back on EOSKit without circular DLL imports.
        PrivateIncludePathModuleNames.AddRange(new string[]
        {
            "EOSKitAuth",
            "EOSKitSessions"
        });

        // -------- Public Dependencies --------
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "Projects",
            "DeveloperSettings",
            "HTTP",
            "Json",
            "JsonUtilities",
            "EOSSDK",
            "EOSKitShared"     // VERY IMPORTANT
        });

        // -------- Private Dependencies --------
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "Sockets",
            "NetCore"
        });

        // -------- Editor Only Dependencies --------
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "ToolMenus"
            });
        }

    }
}
