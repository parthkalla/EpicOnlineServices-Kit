using UnrealBuildTool;

public class EOSKitAuth : ModuleRules
{
	public EOSKitAuth(ReadOnlyTargetRules Target) : base(Target)
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
			"EOSKit"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"NetCore"
		});
	}
}
