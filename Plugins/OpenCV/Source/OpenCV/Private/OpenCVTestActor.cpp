// Fill out your copyright notice in the Description page of Project Settings.


#include "OpenCVTestActor.h"

#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include  <stdio.h>
#if PLATFORM_WINDOWS
#include  <direct.h>
#endif

#include <string>
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"	
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"


using namespace cv;

// Sets default values
AOpenCVTestActor::AOpenCVTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOpenCVTestActor::BeginPlay()
{
	Super::BeginPlay();
	//char   buffer[1024];
	//_getcwd(buffer, 1024);
	//FString Path(buffer);
	//UE_LOG(LogTemp, Warning, TEXT("exe path %s"), *Path);
	//GEngine->AddOnScreenDebugMessage(0, 5, FColor::Red, Path);
	
	
#if PLATFORM_WINDOWS
	FString ContentPath = FPaths::GameContentDir();
	FString TestImgPath = FPaths::Combine(ContentPath, TEXT("TestRes/test.png"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if(!PlatformFile.FileExists(*TestImgPath))
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString::Printf(TEXT("TestImgPath == %s not exist "), *TestImgPath));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("TestImgPath == %s exist "), *TestImgPath));
	}


	std::string temp(TCHAR_TO_UTF8(*TestImgPath));

	Mat img = imread(temp, IMREAD_UNCHANGED);
	if(img.empty())
	{	
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Opencv open img failed!"));
		return;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Opencv open img Success!"));
	}
	
	namedWindow("Example", WINDOW_AUTOSIZE);
	imshow("Example", img);
	
	
#elif PLATFORM_ANDROID

	
	extern const FString &GetFileBasePath();
	
	FString ProjectPath = GetFileBasePath();
	FString TestImgPath = FPaths::Combine(ProjectPath, FApp::GetProjectName(), TEXT("Content/TestRes/test.jpg"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if(!PlatformFile.FileExists(*TestImgPath))
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString::Printf(TEXT("TestImgPath == %s not exist "), *TestImgPath));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("TestImgPath == %s exist "), *TestImgPath));
	}

	std::string temp(TCHAR_TO_UTF8(*TestImgPath));

	Mat img = imread(temp, IMREAD_UNCHANGED);
	if(img.empty())
	{	
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Opencv open img failed!"));
		return;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Opencv open img Success!"));
	}
	
	// namedWindow("Example", WINDOW_AUTOSIZE);
	// imshow("Example", img);
#endif
	
	
}

// Called every frame
void AOpenCVTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

