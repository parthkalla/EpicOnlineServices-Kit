using UnrealBuildTool;

public class EOSKitUserInfo : ModuleRules
{
	public EOSKitUserInfo(ReadOnlyTargetRules Target) : base(Target)
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
			"EOSKitSessions",
			"EOSKit"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"NetCore"
		});
	}
}
