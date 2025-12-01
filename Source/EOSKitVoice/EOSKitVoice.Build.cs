using UnrealBuildTool;
using System.IO;

public class EOSKitVoice : ModuleRules
{
	public EOSKitVoice(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "../EOSKitShared/Public")
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AudioMixer",
				"SignalProcessing",
				"EOSSDK",
				"EOSKitShared",
				"VoiceChat",
				"EOSKit"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"Sockets",
				"Json",
				"JsonUtilities"
			}
		);
		
		PublicDefinitions.Add("WITH_EOS_RTC=1");
	}
}
