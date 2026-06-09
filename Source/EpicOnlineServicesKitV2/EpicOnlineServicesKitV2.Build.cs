// Copyright © 2025 Asrock Studios.
// All rights reserved.

using System.IO;
using UnrealBuildTool;

public class EpicOnlineServicesKitV2 : ModuleRules
{
	public EpicOnlineServicesKitV2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDefinitions.Add("EpicOnlineServicesKitV2_API=DLLEXPORT");
		PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
		PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"OnlineSubsystemUtils",
			}
		);
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects",
				"OnlineSubsystem",
				"Slate",
				"SlateCore",
			}
		);
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
					"ToolMenus",
				}
			);
		}

		if (!Target.bBuildEditor && Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicDefinitions.Add("ANDROIDX_ENABLED=1");
			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });

			string modulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(modulePath, "EpicOnlineServicesKitV2_Android.xml"));
		}
	}
}