// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

//for dlib config.h
// #if PLATFORM_ANDROID
// #ifndef ANDROID
// #define ANDROID
// #endif
// #endif

class FDlibModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	
};
