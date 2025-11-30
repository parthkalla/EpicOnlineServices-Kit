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

		// Add EOSSDK dependency if available
		PublicDependencyModuleNames.Add("EOSSDK");
		
		// WITH_EOS_SDK should be set by the EOSSDK module or build system
		// Don't force it to 0 - let the build system determine if EOS SDK is available
	}
}
