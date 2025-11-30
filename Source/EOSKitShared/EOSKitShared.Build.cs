// Copyright (C) 2024, All Rights Reserved.

using UnrealBuildTool;

public class EOSKitShared : ModuleRules
{
	public EOSKitShared(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects"
			}
		);

		// Ensure WITH_EOS_SDK is defined for builds that don't set it externally
		PublicDefinitions.Add("WITH_EOS_SDK=0");
	}
}
