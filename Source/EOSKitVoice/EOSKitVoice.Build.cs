// Copyright (C) 2024, All Rights Reserved.

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
				"VoiceChat" // Add VoiceChat module for IVoiceChat interface
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"EOSKit",
				"Sockets",
				"Json",
				"JsonUtilities"
			}
		);
		
		// Add conditional compilation for voice chat
		PublicDefinitions.Add("WITH_EOS_RTC=1");
	}
}
