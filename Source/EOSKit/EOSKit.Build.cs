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
				"EOSShared",
				"EOSKitShared",
				"Sockets",
				"EOSKitAuth",
				"EOSKitLobbies",
				"EOSKitSessions"
			}
		);

		PublicDependencyModuleNames.Add("EOSSDK");
		
		// Add UnrealEd module for editor builds (needed for FUnrealEdMisc)
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
