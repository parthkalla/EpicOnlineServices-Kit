using UnrealBuildTool;
using System.IO;

public class OnlineSubsystemEOSKit : ModuleRules
{
	public OnlineSubsystemEOSKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDefinitions.Add("ONLINESUBSYSTEMEOSKIT_PACKAGE=1");
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"EOSKitShared"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"EOSSDK",
				"EOSKit",
				"EOSKitAuth",
				"EOSKitSessions",
				"EOSKitFriends",
				"EOSKitPresence",
				"EOSKitStats",
				"EOSKitLeaderboards",
				"EOSKitAchievements",
				"EOSKitUserInfo",
				"EOSKitUi",
				"EOSKitTitleStorage",
				"EOSKitPlayerDataStorage",
				"EOSKitEcom",
				"Json",
				"OnlineSubsystem",
				"Sockets",
				"NetCore",
				"InputCore",
				"Projects",
				"Slate",
				"SlateCore",
				"HTTP",
				"DeveloperSettings"
			}
		);
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
					"ToolMenus",
				}
			);
		}
	}
}
