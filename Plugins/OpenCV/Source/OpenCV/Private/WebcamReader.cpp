// Fill out your copyright notice in the Description page of Project Settings.


#include "WebcamReader.h"
#include "OpenCV.h"
#include "Engine/Engine.h"


template<typename _Tp>
cv::Mat ConvertVector2Mat(std::vector<_Tp> v, int channels, int rows)
{
	cv::Mat mat = cv::Mat(v);//将vector变成单列的mat
	cv::Mat dest = mat.reshape(channels, rows).clone();//PS：必须clone()一份，否则返回出错
	return dest;
}


// Sets default values
AWebcamReader::AWebcamReader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
 
	//Initialize OpenCV and webcam properties
	CameraID = 0;
	RefreshRate = 15;
	isStreamOpen = false;
	VideoSize = FVector2D(0, 0);
	ShouldResize = false;
	ResizeDeminsions = FVector2D(320, 240);
	RefreshTimer = 0.0f;
	frame = cv::Mat();
	bBeginCapture = false; //default not capture
}
 
// Called when the game starts or when spawned
void AWebcamReader::BeginPlay()
{
	Super::BeginPlay();
	
	
	
}
 
// Called every frame
void AWebcamReader::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if(bBeginCapture)
	{
		RefreshTimer += DeltaTime;
		if (isStreamOpen && RefreshTimer >= 1.0f / RefreshRate)
		{
			RefreshTimer -= 1.0f / RefreshRate;
			UpdateFrame();
			DoProcessing();
			UpdateTexture();
			OnNextVideoFrame();
		}
	}
	
}
 
void AWebcamReader::UpdateFrame()
{
	if (isStreamOpen)
	{
		CameraFrame->getFrame(frame);

		if (ShouldResize)
		{
			cv::resize(frame, frame, size);
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Camera is not open"));
	}
}

void AWebcamReader::DoProcessing()
{

}
 
void AWebcamReader::UpdateTexture()
{
	UE_LOG(LogTemp, Warning, TEXT("Begin UpdateTexture"));
	if (isStreamOpen && frame.data)
	{
		// Copy Mat data to Data array
		for (int y = 0; y < VideoSize.Y; y++)
		{
			for (int x = 0; x < VideoSize.X; x++)
			{
				int i = x + (y * VideoSize.X);
				Data[i].B = frame.data[i * 3 + 0];
				Data[i].G = frame.data[i * 3 + 1];
				Data[i].R = frame.data[i * 3 + 2];
			}
		}
 
		// Update texture 2D
		UpdateTextureRegions(VideoTexture, (int32)0, (uint32)1, VideoUpdateTextureRegion, (uint32)(4 * VideoSize.X), (uint32)4, (uint8*)Data.GetData(), false);
	}
}
 
void AWebcamReader::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	UE_LOG(LogTemp, Warning, TEXT("Begin UpdateTextureRegions"));
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};
 
		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
 
		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;
 
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
			for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
			{
				int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
				if (RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(
						RegionData->Texture2DResource->GetTexture2DRHI(),
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Regions[RegionIndex],
						RegionData->SrcPitch,
						RegionData->SrcData
						+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
						+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
				}
			}
			if (bFreeData)
			{
				FMemory::Free(RegionData->Regions);
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
		});
	}
	UE_LOG(LogTemp, Warning, TEXT("End UpdateTextureRegions"));
}

void AWebcamReader::BeginCapture()
{
	// Open the stream

	CameraFrame = MakeShareable<FCameraFrame>(new FCameraFrame());
	isStreamOpen = CameraFrame->captureCamera(CameraID, ResizeDeminsions.X, ResizeDeminsions.Y);


	if (isStreamOpen)
	{
		// Initialize stream
		isStreamOpen = true;
		UpdateFrame();
		VideoSize = FVector2D(frame.cols, frame.rows);
		UE_LOG(LogTemp, Warning, TEXT("VideoSize == %s"), *VideoSize.ToString());
		size = cv::Size(ResizeDeminsions.X, ResizeDeminsions.Y);
		VideoTexture = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y);
		VideoTexture->UpdateResource();
		VideoUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, VideoSize.X, VideoSize.Y);
 
		// Initialize data array
		Data.Init(FColor(0, 0, 0, 255), VideoSize.X * VideoSize.Y);

		// Do first frame
		DoProcessing();
		UpdateTexture();
		OnNextVideoFrame();
		GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, TEXT("Camera is  opened"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Red, TEXT("Camera is not open"));
	}
	///>test
	GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Yellow, *FString::Printf(TEXT("windows frame is %d"), frame.type()));
	bBeginCapture = true;

}

void AWebcamReader::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
#if PLATFORM_ANDROID
	if (CameraFrame) {
		CameraFrame->releaseCamera();
	}
#endif
	
	isStreamOpen = false;
}
