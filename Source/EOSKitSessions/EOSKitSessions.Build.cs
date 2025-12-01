using UnrealBuildTool;

public class EOSKitSessions : ModuleRules
{
	public EOSKitSessions(ReadOnlyTargetRules Target) : base(Target)
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
			"EOSKitShared",
			"EOSKitAuth",
			"EOSKit"
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
