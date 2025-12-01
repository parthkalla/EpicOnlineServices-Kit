using UnrealBuildTool;
using System.IO;

public class EOSKitEditor : ModuleRules
{
    public EOSKitEditor(ReadOnlyTargetRules Target) : base(Target)
    {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
			"EditorFramework",
			"EOSKitShared"
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
