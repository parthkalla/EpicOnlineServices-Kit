// Copyright Epic Games, Inc. All Rights Reserved.


using System;
using System.IO;
using UnrealBuildTool;

public class EOKSDK : ModuleRules
{
	public string SDKBaseDir
	{
		get { return ModuleDirectory; }
	}

	public virtual string SDKBinariesDir
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Android)
			{
				return Path.Combine(SDKBaseDir, "Bin", "Android");
			}
			else if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				return Path.Combine(SDKBaseDir, "Bin", "IOS");
			}

			return Path.Combine(SDKBaseDir, "Bin");
		}
	}

	public virtual string SDKLibsDir
	{
		get { return Path.Combine(SDKBaseDir, "Lib"); }
	}

	public virtual string SDKIncludesDir
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Android)
			{
				return Path.Combine(SDKBaseDir, "include");
			}
			else if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				return Path.Combine(SDKBinariesDir, "EOSSDK.framework", "Headers");
			}
			return Path.Combine(SDKBaseDir, "Include");
		}
	}

	public virtual string LibraryLinkNameBase
	{
		get
		{
			if(Target.Platform == UnrealTargetPlatform.Android)
			{
				return "EOSSDK";
			}
			
			return String.Format("EOSSDK-{0}-Shipping", Target.Platform.ToString());
		}
	}
	
	public virtual string RuntimeLibraryFileName
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				return "lib" + LibraryLinkNameBase + ".dylib";
			}
			else if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				return LibraryLinkNameBase + ".framework";
			}
			else if (Target.Platform == UnrealTargetPlatform.Android ||
			         Target.Platform == UnrealTargetPlatform.Linux)
			{
				return "lib" + LibraryLinkNameBase + ".so";
			}
			else if (Target.Platform.IsInGroup(UnrealPlatformGroup.Microsoft))
			{
				return LibraryLinkNameBase + ".dll";
			}
			// Other platforms will override this property.

			Console.WriteLine(string.Format("EOKSDK: Unsupported platform detected: {0}", Target.Platform));
			return string.Empty;
		}
	}
	public virtual bool bRequiresRuntimeLoad
	{
		get
		{
			return Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) || Target.Platform == UnrealTargetPlatform.Mac;
			// Other platforms may override this property.
		}
	}
	public EOKSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PublicDefinitions.Add("WITH_EOS_SDK=1");
		PublicDefinitions.Add(String.Format("EOSSDK_RUNTIME_LOAD_REQUIRED={0}", bRequiresRuntimeLoad ? 1 : 0));
		PublicDefinitions.Add(String.Format("EOSSDK_RUNTIME_LIBRARY_NAME=\"{0}\"", RuntimeLibraryFileName));
		Console.WriteLine(RuntimeLibraryFileName);
		PublicIncludePaths.Add(SDKIncludesDir);
		PublicSystemIncludePaths.Add(SDKIncludesDir);
		Console.WriteLine("Epic Online Services-Kit V2: Include path: " + SDKIncludesDir);
		PublicDefinitions.Add("EOSSDK_USE_PROJECT_BINARY=1");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string DLLFileName = "EOSSDK-Win64-Shipping.dll";
			string DLLSourcePath = Path.Combine(SDKBinariesDir, DLLFileName);
			string LIBFileName = "EOSSDK-Win64-Shipping.lib";
			string LIBSourcePath = Path.Combine(SDKLibsDir, LIBFileName);

			// Stage under the plugin folder, not $(BinaryOutputDir). The engine EOSSDK module also
			// stages EOSSDK-Win64-Shipping.dll into engine/game binaries; duplicate targets fail Fab CI.
			string DLLTargetPath = Path.Combine("$(PluginDir)", "Binaries", "Win64", DLLFileName).Replace('\\', '/');

			PublicAdditionalLibraries.Add(LIBSourcePath);
			PublicDelayLoadDLLs.Add(DLLFileName);
			RuntimeDependencies.Add(DLLTargetPath, DLLSourcePath, StagedFileType.NonUFS);
		}

		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PrivateRuntimeLibraryPaths.Add(SDKBinariesDir);
			PublicAdditionalLibraries.Add(Path.Combine(SDKBinariesDir, "libEOSSDK-Linux-Shipping.so"));
			RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "libEOSSDK-Linux-Shipping.so"), Path.Combine(SDKBinariesDir, "libEOSSDK-Linux-Shipping.so"));
			RuntimeDependencies.Add(Path.Combine(SDKBinariesDir, "libEOSSDK-Linux-Shipping.so"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicDelayLoadDLLs.Add(Path.Combine(SDKBinariesDir, "libEOSSDK-Mac-Shipping.dylib"));
			RuntimeDependencies.Add(Path.Combine(SDKBinariesDir, "libEOSSDK-Mac-Shipping.dylib"));
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PublicAdditionalFrameworks.Add(new Framework("EOSSDK", SDKBinariesDir, "", true));
			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(SDKBaseDir, "LibEOK_UPL.xml"));
		} 
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
#if !UE_5_3_OR_LATER
			IAndroidToolChain ToolChain = AndroidExports.CreateToolChain(Target.ProjectFile);
			var Architectures = ToolChain.GetAllArchitectures();

			foreach (var arch in Architectures)
			{
				string FolderName;

				// Map Unreal's architecture names to folder names
				if (arch == "-armv7")
				{
					FolderName = "armeabi-v7a";
				}
				else if (arch == "-arm64")
				{
					FolderName = "arm64-v8a";
				}
				else if (arch == "-x86")
				{
					FolderName = "x86";
				}
				else if (arch == "-x64" || arch == "-x86_64")
				{
					FolderName = "x86_64";
				}
				else
				{
					Console.WriteLine("Unknown architecture: " + arch);
					continue;
				}

				Console.WriteLine("Adding EOS SDK for architecture folder: " + FolderName);

				// Add the corresponding .so file for this architecture
				PublicAdditionalLibraries.Add(Path.Combine(SDKBinariesDir, FolderName, "libEOSSDK.so"));
				RuntimeDependencies.Add(Path.Combine(SDKBinariesDir, FolderName, "libEOSSDK.so"));
			}
#else
			if (Target.Architectures.Contains(UnrealArch.Arm64))
			{
				Console.WriteLine("Adding EOS SDK for arm64-v8a");
				PublicAdditionalLibraries.Add(Path.Combine(SDKBinariesDir, "arm64-v8a", "libEOSSDK.so"));
			}
			if (Target.Architectures.Contains(UnrealArch.Arm64ec))
			{
				Console.WriteLine("Adding EOS SDK for armeabi-v7a");
				PublicAdditionalLibraries.Add(Path.Combine(SDKBinariesDir, "armeabi-v7a", "libEOSSDK.so"));
			}
			if (Target.Architectures.Contains(UnrealArch.X64))
			{
				Console.WriteLine("Adding EOS SDK for x86_64");
				PublicAdditionalLibraries.Add(Path.Combine(SDKBinariesDir, "x86_64", "libEOSSDK.so"));
			}
#endif
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(SDKBaseDir, "LibEOK_APL.xml"));
		}
	}
}
