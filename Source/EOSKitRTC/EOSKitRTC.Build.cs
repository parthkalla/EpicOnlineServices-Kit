using UnrealBuildTool;

public class EOSKitRTC : ModuleRules
{
	public EOSKitRTC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// RTC + Voice
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AudioMixer",
			"SignalProcessing",
			"VoiceChat",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"EOSSDK",
			"EOSKitShared",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"NetCore",
			"Projects",
			"Json",
			"JsonUtilities"
		});

		PublicDefinitions.Add("WITH_EOS_RTC=1");
	}
}
