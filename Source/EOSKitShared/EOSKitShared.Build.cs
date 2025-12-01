using UnrealBuildTool;
using System.IO;

public class EOSKitShared : ModuleRules
{
    public EOSKitShared(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "Public")
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "Private")
        });

        // Headers we include but don't want to link against (avoid circular module deps)
        PrivateIncludePathModuleNames.AddRange(new string[]
        {
            "EOSKit",
            "EOSKitAuth",
            "EOSKitSessions"
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Projects",
            "EOSSDK",
            "Json",
            "JsonUtilities",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "DeveloperSettings"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

    }
}
