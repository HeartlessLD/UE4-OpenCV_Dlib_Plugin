// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FaceRecognition : ModuleRules
{
	public FaceRecognition(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		//for c4458 c4459
		ShadowVariableWarningLevel = WarningLevel.Off;
		//for c4668  err
		bEnableUndefinedIdentifierWarnings = false;
		bEnableExceptions = true;
		
		PublicIncludePathModuleNames.Add("OpenCV");
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OpenCV", "Dlib", "RHI", 
			"RenderCore", });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
