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
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"EOSSDK",
				"EOSShared",
				"EOSKit",
				"EOSKitAuth",
				"EOSKitShared"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"Projects"
			}
		);
	}
}
