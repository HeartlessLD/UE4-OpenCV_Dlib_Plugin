// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "OpenCV.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FOpenCVModule"

void FOpenCVModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("OpenCV"))->GetBaseDir();
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*PluginDir, TEXT("Library/Win64/"));
	UE_LOG(LogTemp, Warning, TEXT("opencv world LibraryPath == %s"), *(LibraryPath + TEXT("opencv_world340.dll")));
	OpenCV_World_Handler = FPlatformProcess::GetDllHandle(*(LibraryPath + TEXT("opencv_world340.dll")));
	OpenCV_FFmpeg_Handler = FPlatformProcess::GetDllHandle(*(LibraryPath + TEXT("opencv_ffmpeg340_64.dll")));
	if (!OpenCV_World_Handler || !OpenCV_FFmpeg_Handler)
	{
		UE_LOG(LogTemp, Error, TEXT("Load OpenCV dll failed!"));
	}
#elif PLATFORM_ANDROID

#elif PLATFORM_IOS

#endif
}

void FOpenCVModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if PLATFORM_WINDOWS
	FPlatformProcess::FreeDllHandle(OpenCV_World_Handler);
	OpenCV_World_Handler = nullptr;
	FPlatformProcess::FreeDllHandle(OpenCV_FFmpeg_Handler);
	OpenCV_FFmpeg_Handler = nullptr;
#elif PLATFORM_ANDROID

#elif PLATFORM_IOS

#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOpenCVModule, OpenCV)