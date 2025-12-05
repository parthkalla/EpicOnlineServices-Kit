using UnrealBuildTool;

public class EOSKitStorage : ModuleRules
{
	public EOSKitStorage(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// TitleStorage + PlayerDataStorage
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
			"Slate",
			"SlateCore",
			"Projects"
		});
	}
}


