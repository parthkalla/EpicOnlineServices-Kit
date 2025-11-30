// Copyright (C) 2024, All Rights Reserved.

using UnrealBuildTool;

public class EOSKitSockets : ModuleRules
{
	public EOSKitSockets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Sockets",
				"Networking",
				"NetCore",
				"EOSSDK",
				"EOSShared",
				"EOSKit",
				"EOSKitShared",
				"OnlineSubsystemUtils" // Needed for UIpNetDriver
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
