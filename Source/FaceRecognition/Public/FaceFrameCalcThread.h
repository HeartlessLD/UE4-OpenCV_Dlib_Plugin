// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"	
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
//for kalman
#include "opencv2/video/tracking.hpp"

#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

//dlib
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing.h" 

#include "CoreMinimal.h"
#include "Frame/CameraFrame.h"
#include "HAL/Runnable.h"
#include "UObject/NoExportTypes.h"

#include "FaceFrameCalcThread.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateFaceInfoDelegate);

/**
 * 
 */
UCLASS()
class FACERECOGNITION_API UFaceFrameCalcThread : public UObject, public FRunnable
{
public:
	bool Init() override{return true;};
	uint32 Run() override;
	void Stop() override;
	void Exit() override{};

	GENERATED_BODY()
public:
	void Start(cv::Mat* Frame, int FrameRate, int OptimizeScale);

	cv::Mat FinalFrame;
	
	FUpdateFaceInfoDelegate UpdateFaceInfoCallback;
	bool bStop;
private:
	cv::Mat* Frame;
	
	
	FCriticalSection Mutex;
	//dlib
	int UpdateCount;
	dlib::frontal_face_detector FrontDector;
	dlib::shape_predictor PoseModel;
	std::vector<dlib::rectangle> faces;

	int FrameRate;
	int OptimizeScale;

	void RenderFace(cv::Mat& img, const dlib::full_object_detection& shape);
	void ProcessShapeWithKalman(const dlib::full_object_detection& shape);
	void DrawPolyLine(cv::Mat &img, const dlib::full_object_detection& d, const int start, const int end, bool isClosed = false);
	void DrawCircle(cv::Mat &img, const dlib::full_object_detection& d, const float radius, cv::Scalar color);

	void CalcFaceFeature(const dlib::full_object_detection& shape);
	void CalcHeadInfo(const dlib::full_object_detection& shape);
public:
	UPROPERTY(BlueprintReadOnly)
	float LeftEyeWeight;
	UPROPERTY(BlueprintReadOnly)
	float RightEyeWeight;
	UPROPERTY(BlueprintReadOnly)
	float MouseWeight;
	
	UPROPERTY(BlueprintReadOnly)
	FQuat HeadQuat;

	UPROPERTY(BlueprintReadOnly)
	FRotator HeadRotator;

	

private:
	//Intrisics can be calculated using opencv sample code under opencv/sources/samples/cpp/tutorial_code/calib3d
	//Normally, you can also apprximate fx and fy by image width, cx by half image width, cy by half image height instead
	double K[9] = { 6.5308391993466671e+002, 0.0, 3.1950000000000000e+002, 0.0, 6.5308391993466671e+002, 2.3950000000000000e+002, 0.0, 0.0, 1.0 };
	double D[5] = { 7.0834633684407095e-002, 6.9140193737175351e-002, 0.0, 0.0, -1.3073460323689292e+000 };
	//fill in cam intrinsics and distortion coefficients
	cv::Mat cam_matrix = cv::Mat(3, 3, CV_64FC1, K);
	cv::Mat dist_coeffs = cv::Mat(5, 1, CV_64FC1, D);
	std::vector<cv::Point3d> object_pts;
	std::vector<cv::Point2d> image_pts;
	//result
	cv::Mat rotation_vec;                           //3 x 1
	cv::Mat rotation_mat;                           //3 x 3 R
	cv::Mat translation_vec;                        //3 x 1 T
	cv::Mat pose_mat = cv::Mat(3, 4, CV_64FC1);     //3 x 4 R | T
	cv::Mat euler_angle = cv::Mat(3, 1, CV_64FC1);

	//reproject 3D points world coordinate axis to verify result pose
	std::vector<cv::Point3d> reprojectsrc;
	//reprojected 2D points
	std::vector<cv::Point2d> reprojectdst;
	//temp buf for decomposeProjectionMatrix()
	cv::Mat out_intrinsics = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_rotation = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_translation = cv::Mat(3, 1, CV_64FC1);

	//Filter
	cv::KalmanFilter KFs[68];
	cv::Mat measurements[68];
	cv::Point facePoints[68];
};
