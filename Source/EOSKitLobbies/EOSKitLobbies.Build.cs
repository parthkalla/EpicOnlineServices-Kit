using UnrealBuildTool;

public class EOSKitLobbies : ModuleRules
{
	public EOSKitLobbies(ReadOnlyTargetRules Target) : base(Target)
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
			"EOSKitAuth"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"NetCore"
		});
	}
}
