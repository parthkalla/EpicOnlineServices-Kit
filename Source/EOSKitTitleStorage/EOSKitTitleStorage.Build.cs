// Copyright (C) 2024, All Rights Reserved.

using UnrealBuildTool;

public class EOSKitTitleStorage : ModuleRules
{
	public EOSKitTitleStorage(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
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
				"EOSKit"
			}
		);
	}
}
