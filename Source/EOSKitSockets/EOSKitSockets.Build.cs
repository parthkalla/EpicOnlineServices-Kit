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
				"Sockets",
				"Networking",
				"NetCore",
				"EOSSDK",
				"OnlineSubsystemUtils" // Needed for UIpNetDriver
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
				"EOSKit" // Dependency for Subsystem/Settings
			}
		);
	}
}
