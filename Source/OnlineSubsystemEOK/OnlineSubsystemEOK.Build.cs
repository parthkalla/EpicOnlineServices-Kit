// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
#if !UE_5_0_OR_LATER
using Tools.DotNETCommon;
#else
using EpicGames.Core;
#endif
public class OnlineSubsystemEOK : ModuleRules
{
	public OnlineSubsystemEOK(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDefinitions.Add("ONLINESUBSYSTEMEOS_PACKAGE=1");
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/OnlineSubsystemEOKModule.h";
		
		// Fab/CI: ensure ENGINE_* macros exist before any #if ENGINE_MAJOR_VERSION checks (C4668).
		PublicDefinitions.Add($"ENGINE_MAJOR_VERSION={Target.Version.MajorVersion}");
		PublicDefinitions.Add($"ENGINE_MINOR_VERSION={Target.Version.MinorVersion}");
		
		PublicDefinitions.Add("OnlineSubsystemEOK_API=DLLEXPORT");

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"OnlineSubsystemUtils"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"EOKSDK",
				"EOKShared",
				"EOKVoiceChat",
				"Json",
				"OnlineSubsystem",
				"Sockets",
				"VoiceChat",
				"NetCore", 
				"SocketSubsystemEOK",
				"InputCore",
				"Projects",
				"Slate",
				"SlateCore",
				"HTTP", 
				"EpicOnlineServicesKitV2",
			}
		);
		#if UE_5_0_OR_LATER
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreOnline", "Core", "Sockets" , "OnlineBase"});
		#endif

		
		PrivateDefinitions.Add("USE_XBL_XSTS_TOKEN=" + (bUseXblXstsToken ? "1" : "0"));
		PrivateDefinitions.Add("USE_PSN_ID_TOKEN=" + (bUsePsnIdToken ? "1" : "0"));
		PrivateDefinitions.Add("ADD_USER_LOGIN_INFO=" + (bAddUserLoginInfo ? "1" : "0"));
		bool bSupportOculusPlatform = false;
		ConfigHierarchy PlatformGameConfig = ConfigCache.ReadHierarchy(ConfigHierarchyType.Engine, DirectoryReference.FromFile(Target.ProjectFile), UnrealTargetPlatform.Android);
		if (!PlatformGameConfig.GetBool("OnlineSubsystemOculus", "bEnabled", out bSupportOculusPlatform))
		{
			bSupportOculusPlatform = false;
		}
		if (bSupportOculusPlatform)
		{
			PublicDefinitions.Add("SUPPORTOCULUSPLATFORM=1");
		}
		else
		{
			PublicDefinitions.Add("SUPPORTOCULUSPLATFORM=0");
		}

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
		}
		
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

	protected virtual bool bUseXblXstsToken { get { return false; } }
	protected virtual bool bUsePsnIdToken { get { return false; } }
	protected virtual bool bAddUserLoginInfo { get { return false; } }
}
