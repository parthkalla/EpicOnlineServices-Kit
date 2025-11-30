// Copyright (C) 2024, All Rights Reserved.

using UnrealBuildTool;

public class EOSKitLeaderboards : ModuleRules
{
	public EOSKitLeaderboards(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
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
		});
	}
}

