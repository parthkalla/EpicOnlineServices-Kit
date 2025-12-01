using UnrealBuildTool;
using System.IO;

public class EOSKit : ModuleRules
{
	public EOSKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public")
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private"),
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"Projects",
				"DeveloperSettings",
				"EOSKitShared",
				"Sockets",
				"Json",
				"EOSSDK"
			}
		);

		// Feature modules as private dependencies - EOSKit can use them but they don't depend on EOSKit
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EOSKitAuth",
				"EOSKitLobbies",
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
				"EOSKitP2P",
				"EOSKitRTC",
				"EOSKitSanctions",
				"EOSKitWeb",
				"EOSKitSockets",
				"EOSKitVoice"
			}
		);
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
