// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class EOKVoiceChat : ModuleRules
	{
		public EOKVoiceChat(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			
			PublicDefinitions.Add("EOKVoiceChat_API=DLLEXPORT");
			PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
			PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");

			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Json",
					"AudioExtensions"
				}
			);

			PublicIncludePathModuleNames.AddRange(
				new string[]
				{
					"VoiceChat",
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "CoreUObject",
                    "Projects",
                    "EOKShared",
                    "EOKSDK",
					"Engine",
                    "HTTP", 
                    "EpicOnlineServicesKitV2",
					"Synthesis",
					"SignalProcessing",
					"AudioMixer",
				}
			);

			if(Target.Platform == UnrealTargetPlatform.IOS)
			{
				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"ApplicationCore"
					}
				);
			}
		}
	}
}
