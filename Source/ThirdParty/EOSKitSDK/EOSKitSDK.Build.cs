// Copyright (C) 2024, All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class EOSKitSDK : ModuleRules
{
	private string SDKBaseDir => ModuleDirectory;
	private string SDKBinariesDir
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Android)
			{
				return Path.Combine(SDKBaseDir, "Bin", "Android");
			}
			return Path.Combine(SDKBaseDir, "Bin");
		}
	}
	private string SDKLibsDir => Path.Combine(SDKBaseDir, "Lib");
	private string SDKIncludesDir => Path.Combine(SDKBaseDir, "Include");

	private string LibraryLinkNameBase
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Android)
			{
				return "EOSSDK";
			}

			return $"EOSSDK-{Target.Platform}-Shipping";
		}
	}

	private string RuntimeLibraryFileName
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				return $"lib{LibraryLinkNameBase}.dylib";
			}
			if (Target.Platform == UnrealTargetPlatform.Linux ||
			    Target.Platform == UnrealTargetPlatform.LinuxArm64 ||
			    Target.Platform == UnrealTargetPlatform.Android)
			{
				return $"lib{LibraryLinkNameBase}.so";
			}
			if (Target.Platform.IsInGroup(UnrealPlatformGroup.Microsoft))
			{
				return $"{LibraryLinkNameBase}.dll";
			}

			return string.Empty;
		}
	}

	public EOSKitSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicDefinitions.Add("WITH_EOS_SDK=1");
		PublicDefinitions.Add($"EOSSDK_RUNTIME_LOAD_REQUIRED={(Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) || Target.Platform == UnrealTargetPlatform.Mac ? 1 : 0)}");
		PublicDefinitions.Add($"EOSSDK_RUNTIME_LIBRARY_NAME=\"{RuntimeLibraryFileName}\"");
		PublicDefinitions.Add("EOSSDK_USE_PROJECT_BINARY=1");

		PublicIncludePaths.Add(SDKIncludesDir);
		PublicSystemIncludePaths.Add(SDKIncludesDir);
		Console.WriteLine("EOS Kit: Include path: " + SDKIncludesDir);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string dllName = "EOSSDK-Win64-Shipping.dll";
			string libName = "EOSSDK-Win64-Shipping.lib";
			string dllSource = Path.Combine(SDKBinariesDir, dllName);
			string libSource = Path.Combine(SDKLibsDir, libName);

			PublicAdditionalLibraries.Add(libSource);
			PublicDelayLoadDLLs.Add(dllName);

			// DLL is needed at runtime - BinaryOutputDir covers both editor and packaged builds
			RuntimeDependencies.Add($"$(BinaryOutputDir)/{dllName}", dllSource, StagedFileType.NonUFS);
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64)
		{
			string soName = Target.Platform == UnrealTargetPlatform.LinuxArm64 ? "libEOSSDK-LinuxArm64-Shipping.so" : "libEOSSDK-Linux-Shipping.so";
			string soSource = Path.Combine(SDKBinariesDir, soName);
			PrivateRuntimeLibraryPaths.Add(SDKBinariesDir);
			PublicAdditionalLibraries.Add(soSource);
			RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", soName), soSource);
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string dylibName = "libEOSSDK-Mac-Shipping.dylib";
			string dylibSource = Path.Combine(SDKBinariesDir, dylibName);
			PublicDelayLoadDLLs.Add(dylibName);
			RuntimeDependencies.Add(Path.Combine(SDKBinariesDir, dylibName));
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Ship both ABI variants that exist in the ThirdParty folder
			string[] abiFolders = { "arm64-v8a", "armeabi-v7a", "x86", "x86_64" };
			foreach (string abi in abiFolders)
			{
				string soPath = Path.Combine(SDKBinariesDir, abi, "libEOSSDK.so");
				if (File.Exists(soPath))
				{
					PublicAdditionalLibraries.Add(soPath);
					RuntimeDependencies.Add(soPath);
				}
			}
			
			// Add Android Plugin Library (APL) file for Android-specific configuration
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(SDKBaseDir, "LibEOSKit_APL.xml"));
		}
		else
		{
			Console.WriteLine($"EOS Kit: platform {Target.Platform} is not explicitly handled in EOSSDK.Build.cs");
		}
	}
}

