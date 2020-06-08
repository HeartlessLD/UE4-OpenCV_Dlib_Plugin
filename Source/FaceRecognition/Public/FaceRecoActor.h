// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Frame/CameraFrame.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"	
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"

#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

//dlib
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing.h" 
#define UpdateResource UpdateResource


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FaceRecoActor.generated.h"

UCLASS()
class FACERECOGNITION_API AFaceRecoActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFaceRecoActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
 
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
 
	// The device ID opened by the Video Stream
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
    int32 CameraID;
 
	// If the webcam images should be resized every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
    bool ShouldResize;
 
	// The targeted resize width and height (width, height)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
    FVector2D ResizeDeminsions;
 
	// The rate at which the color data array and video texture is updated (in frames per second)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
    float RefreshRate;
 
	// The refresh timer
	UPROPERTY(BlueprintReadWrite, Category = Webcam)
    float RefreshTimer;
 
	// Blueprint Event called every time the video frame is updated
	UFUNCTION(BlueprintImplementableEvent, Category = Webcam)
    void OnNextVideoFrame();
 
	// OpenCV fields
	cv::Mat frame;
	// cv::VideoCapture stream;
	cv::Size size;
 
	// OpenCV prototypes
	void UpdateFrame();
	void DoProcessing();
	UFUNCTION()
	void UpdateTexture();
 
	// If the stream has succesfully opened yet
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
    bool isStreamOpen;
 
	// The videos width and height (width, height)
	UPROPERTY(BlueprintReadWrite, Category = Webcam)
    FVector2D VideoSize;
 
	// The current video frame's corresponding texture
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
    UTexture2D* VideoTexture;
 
	// The current data array
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
    TArray<FColor> Data;

	protected:
 
     // Use this function to update the texture rects you want to change:
     // NOTE: There is a method called UpdateTextureRegions in UTexture2D but it is compiled WITH_EDITOR and is not marked as ENGINE_API so it cannot be linked
     // from plugins.
     // FROM: https://wiki.unrealengine.com/Dynamic_Textures
     void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);
 
	//Pointer to update texture region 2D struct
	FUpdateTextureRegion2D* VideoUpdateTextureRegion;

	TSharedPtr<FCameraFrame> CameraFrame;
	std::vector<uchar> buffer;


	bool bBeginCapture;

	public:
        UFUNCTION(BlueprintCallable, Category = Webcam)
            void BeginCapture();
            
  

	public:
		UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = Dlib)
			int FaceCheckRate;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Dlib)
	int OptimizeCount;

	public:
	UPROPERTY(BlueprintReadOnly)
        class UFaceFrameCalcThread* FaceThread;
	private:
		void InitDlib();
		void RenderFace(cv::Mat& img, const dlib::full_object_detection& shape);
		void DrawPolyLine(cv::Mat &img, const dlib::full_object_detection& d, const int start, const int end, bool isClosed = false);
	
		
};
