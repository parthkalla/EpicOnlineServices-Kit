using UnrealBuildTool;

public class EOSKitSessions : ModuleRules
{
	public EOSKitSessions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Sessions + Lobbies + P2P
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"EOSKitSDK",
			"EOSKitShared",
			"EOSKitAuth",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"NetCore",
			"HTTP",
			"Json",
			"JsonUtilities"
		});
	}
}
