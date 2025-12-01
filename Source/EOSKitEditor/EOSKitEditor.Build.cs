using UnrealBuildTool;
using System.IO;

public class EOSKitEditor : ModuleRules
{
    public EOSKitEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // UE 5.5 FIX — ModuleType.Editor REMOVED
        if (Target.bBuildEditor)
        {
            PublicDefinitions.Add("EOSKITEDITOR_BUILD=1");
        }
        else
        {
            PublicDefinitions.Add("EOSKITEDITOR_BUILD=0");
        }

        PublicDefinitions.Add("EOSKITEDITOR_API=__declspec(dllexport)");

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "Projects",
            "ToolMenus",
            "EditorFramework"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "PropertyEditor",
            "LevelEditor",
            "InputCore",
            "EditorStyle",
            "EOSKit"
        });
    }
}
