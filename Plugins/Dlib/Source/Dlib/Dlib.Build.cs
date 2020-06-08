// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Dlib : ModuleRules
{
	public Dlib(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		//solve cannot use ‘throw’ with exceptions disabled
		bEnableExceptions = true;
		
		ShadowVariableWarningLevel = WarningLevel.Off;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
				"Engine",
				"CoreUObject",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);



		string IncludePath = Path.Combine(ModuleDirectory, "../../Include");
		string LibraryPath = Path.Combine(ModuleDirectory, "../../Library");
		
		//Add Include Path
		PublicIncludePaths.Add(IncludePath);
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			//Add static library
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Win64", "dlib.lib"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			//Add static library
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Android", "arm64-v8a", "lib", "libdlib.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Android", "armeabi", "lib", "libdlib.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Android", "armeabi-v7a", "lib", "libdlib.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Android", "mips", "lib", "libdlib.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Android", "mips64", "lib", "libdlib.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Android", "x86", "lib", "libdlib.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "Android", "x86_64", "lib", "libdlib.a"));
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			
		}

	}
}
