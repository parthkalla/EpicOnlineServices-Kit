// Copyright (C) 2024, All Rights Reserved.

using UnrealBuildTool;

public class EOSKitWeb : ModuleRules
{
	public EOSKitWeb(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"HTTP",
				"Json",
				"JsonUtilities",
				"EOSSDK"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects",
				"EOSShared",
				"EOSKit" // Dependency for Subsystem/Settings
			}
		);
	}
}
