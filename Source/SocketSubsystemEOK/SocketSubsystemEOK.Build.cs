// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class SocketSubsystemEOK : ModuleRules
{
	public SocketSubsystemEOK(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.CPlusPlus;
		PrivatePCHHeaderFile = "Public/InternetAddrEOK.h";
		
		PublicDefinitions.Add("SocketSubsystemEOK_API=DLLEXPORT");
		PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
		PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"EOKShared",
				"NetCore",
				"Sockets",
				"OnlineSubsystemUtils"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"EOKSDK"
			}
		);
	}
}
