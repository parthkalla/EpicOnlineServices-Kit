// Copyright (c) 2024 xist.gg

using UnrealBuildTool;

public class DiscordGame : ModuleRules
{
	public DiscordGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;
		
		PublicDefinitions.Add("DISCORDGAME_API=DLLEXPORT");
		PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
		PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"DiscordGameSDK",
			"Engine",
			"Projects",
		});
	}
}
