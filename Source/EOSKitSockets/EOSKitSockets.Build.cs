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
				"EOSKitShared",
				"OnlineSubsystemUtils",
				"EOSKit"
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
