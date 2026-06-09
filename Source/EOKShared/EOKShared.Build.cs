// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class EOKShared : ModuleRules
{
	public EOKShared(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDefinitions.Add("EOKShared_API=DLLEXPORT");
		PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
		PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");
		
		PublicDependencyModuleNames.AddRange(new string[] { "EOKSDK" });
		Type = ModuleType.CPlusPlus;
		PrivatePCHHeaderFile = "Private/EOKSharedModule.h";

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"EOKSDK",
				"Projects",
			}
		);
	}
}
