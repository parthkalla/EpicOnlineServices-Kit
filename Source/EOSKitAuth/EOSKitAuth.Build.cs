using UnrealBuildTool;

public class EOSKitAuth : ModuleRules
{
	public EOSKitAuth(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Auth + UserInfo + UI helpers
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"EOSKitSDK",
			"EOSKitShared",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"NetCore",
			"Json",
			"JsonUtilities",
			"Slate",
			"SlateCore"
		});
	}
}
