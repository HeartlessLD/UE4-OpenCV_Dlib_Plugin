// Fill out your copyright notice in the Description page of Project Settings.


#include "FaceRecoActor.h"


// Fill out your copyright notice in the Description page of Project Settings.
#include "FaceFrameCalcThread.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "WebcamReader.h"
#include "OpenCV.h"
#include "dlib/opencv/cv_image.h"
#include "Engine/Engine.h"


template<typename _Tp>
cv::Mat ConvertVector2Mat(std::vector<_Tp> v, int channels, int rows)
{
	cv::Mat mat = cv::Mat(v);//将vector变成单列的mat
	cv::Mat dest = mat.reshape(channels, rows).clone();//PS：必须clone()一份，否则返回出错
	return dest;
}


// Sets default values
AFaceRecoActor::AFaceRecoActor()
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
	// //Dlib
	OptimizeCount = 2;
	FaceCheckRate = 4;

}
 
// Called when the game starts or when spawned
void AFaceRecoActor::BeginPlay()
{
	Super::BeginPlay();
	
	 
	
}
 
// Called every frame
void AFaceRecoActor::Tick( float DeltaTime )
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
			//UpdateTexture();
			OnNextVideoFrame();
		}
	}
	
}
 
void AFaceRecoActor::UpdateFrame()
{
	if (isStreamOpen)
	{
		///>
		CameraFrame->getFrame(frame);
		
		//frame = FaceThread->FinalFrame.clone();
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Camera is not open"));
	}
}

void AFaceRecoActor::DoProcessing()
{
	//Face Reconition Use Dlib
 
	// if(frame.empty())
	// 	return;
 //
	//
	// cv::Mat smallMat;
	// cv::resize(frame, smallMat, cv::Size(), 1.0/OptimizeCount, 1.0/OptimizeCount);
	// dlib::cv_image<dlib::bgr_pixel> cimg(frame);
	// dlib::cv_image<dlib::bgr_pixel> cimg_small(smallMat);
	//
	// // Detect faces   
	// if (UpdateCount++ % FaceCheckRate == 0) {
	// 	faces = FrontDector(cimg_small);
	// 	if(faces.size() == 0)
	// 	{
	// 		UE_LOG(LogTemp, Error, TEXT("Can't dector face from cimg_small"));
	// 		return;
	// 	}
	// }
	// // Find the pose of each face.  
	// std::vector<dlib::full_object_detection> shapes;
	// for (unsigned long i = 0; i < faces.size(); ++i) {
	// 	dlib::rectangle r(
 //            (long)(faces[i].left()*OptimizeCount),
 //            (long)(faces[i].top()*OptimizeCount ),
 //            (long)(faces[i].right()*OptimizeCount),
 //            (long)(faces[i].bottom()*OptimizeCount)
 //        );
	// 	
	// 	dlib::full_object_detection shape = PoseModel(cimg, r);
	// 	
	// 	// Custom Face Render
	// 	RenderFace(frame, shape);
	// }
}
 
void AFaceRecoActor::UpdateTexture()
{
	//UE_LOG(LogTemp, Warning, TEXT("Begin UpdateTexture"));
	
	cv::Mat Temp = FaceThread->FinalFrame.clone();
	if (isStreamOpen && Temp.data)
	{
		// Copy Mat data to Data array
		for (int y = 0; y < VideoSize.Y; y++)
		{
			for (int x = 0; x < VideoSize.X; x++)
			{
				int i = x + (y * VideoSize.X);
				Data[i].B = Temp.data[i * 3 + 0];
				Data[i].G = Temp.data[i * 3 + 1];
				Data[i].R = Temp.data[i * 3 + 2];
			}
		}
 
		// Update texture 2D
		UpdateTextureRegions(VideoTexture, (int32)0, (uint32)1, VideoUpdateTextureRegion, (uint32)(4 * VideoSize.X), (uint32)4, (uint8*)Data.GetData(), false);
	}
}
 
void AFaceRecoActor::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	
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
	
}

void AFaceRecoActor::BeginCapture()
{
	
	
	// Open the stream
	CameraFrame = MakeShareable<FCameraFrame>(new FCameraFrame());
	isStreamOpen = CameraFrame->captureCamera(CameraID, ResizeDeminsions.X, ResizeDeminsions.Y);


	if (isStreamOpen)
	{
		InitDlib();
		// Initialize stream
		isStreamOpen = true;
		UpdateFrame();
		VideoSize = FVector2D(frame.cols, frame.rows);
		UE_LOG(LogTemp, Warning, TEXT("VideoSize == %s"), *VideoSize.ToString());
		if(VideoSize == FVector2D::ZeroVector)
		{
			UE_LOG(LogTemp, Error, TEXT("Camera is opened, but get video size is zero"));
		}
		else
		{
			size = cv::Size(ResizeDeminsions.X, ResizeDeminsions.Y);
            		VideoTexture = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y);

			VideoTexture->UpdateResource();
			VideoUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, VideoSize.X, VideoSize.Y);

			// Initialize data array
			Data.Init(FColor(0, 0, 0, 255), VideoSize.X * VideoSize.Y);

			// Do first frame
			// DoProcessing();
			// UpdateTexture();
			// OnNextVideoFrame();
			GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, TEXT("Camera is  opened"));
		}
		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Red, TEXT("Camera is not open"));
	}
	///>test
	GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Yellow, *FString::Printf(TEXT("windows frame is %d"), frame.type()));
	bBeginCapture = true;

}

void AFaceRecoActor::InitDlib()
{
	//Init Dlib
// 	FrontDector = dlib::get_frontal_face_detector();
// 	
// 	FString ShapePath = FPaths::Combine(FPaths::GameContentDir(), TEXT("TestRes/shape_predictor_68_face_landmarks.dat"));
// #if PLATFORM_ANDROID
// 	extern const FString &GetFileBasePath();
// 	FString ProjectPath = GetFileBasePath();
// 	ShapePath = FPaths::Combine(ProjectPath, FApp::GetProjectName(), TEXT("Content/TestRes/shape_predictor_68_face_landmarks.dat"));
// #endif
// 	std::ifstream StreamIn(TCHAR_TO_UTF8(*ShapePath), std::ios::binary);
// 	
// 	deserialize(PoseModel, StreamIn);
// 	StreamIn.clear();
// 	StreamIn.close();
// 	UE_LOG(LogTemp, Warning, TEXT("deserialize end, begin deserialize ShapePath = %s"), *ShapePath );
	FaceThread = NewObject<UFaceFrameCalcThread>(this);
	(FaceThread->UpdateFaceInfoCallback).AddDynamic(this, &AFaceRecoActor::UpdateTexture);
	FaceThread->Start(&frame, FaceCheckRate, OptimizeCount);
	
}

// void AFaceRecoActor::RenderFace(cv::Mat& img, const dlib::full_object_detection& shape)
// {
//
// 	if(shape.num_parts() != 68)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("Dlib shape number is not 68"));
// 	}
//  
// 	DrawPolyLine(img, shape, 0, 16);           // Jaw line
// 	DrawPolyLine(img, shape, 17, 21);          // Left eyebrow
// 	DrawPolyLine(img, shape, 22, 26);          // Right eyebrow
// 	DrawPolyLine(img, shape, 27, 30);          // Nose bridge
// 	DrawPolyLine(img, shape, 30, 35, true);    // Lower nose
// 	DrawPolyLine(img, shape, 36, 41, true);    // Left eye
// 	DrawPolyLine(img, shape, 42, 47, true);    // Right Eye
// 	DrawPolyLine(img, shape, 48, 59, true);    // Outer lip
// 	DrawPolyLine(img, shape, 60, 67, true);    // Inner lip
//
// }
//
// void AFaceRecoActor::DrawPolyLine(cv::Mat& img, const dlib::full_object_detection& d, const int start, const int end,
// 	bool isClosed)
// {
// 	std::vector <cv::Point> points;
// 	for (int i = start; i <= end; ++i)
// 	{
// 		points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
// 	}
// 	cv::polylines(img, points, isClosed, cv::Scalar(255, 0, 0), 2, 16);
//
// }


void AFaceRecoActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	
	Super::EndPlay(EndPlayReason);
	if (CameraFrame) {
		CameraFrame->releaseCamera();
	}
	if (FaceThread)
	{
		FaceThread->bStop = true;
	}
	
	isStreamOpen = false;
}


