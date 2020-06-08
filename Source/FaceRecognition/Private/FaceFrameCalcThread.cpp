// Fill out your copyright notice in the Description page of Project Settings.


#include "FaceFrameCalcThread.h"


#include <dlib/opencv/cv_image.h>
#include <opencv2/video/tracking.hpp>


#include "Engine/Engine.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/core/types_c.h"
#include "HAL/RunnableThread.h"


//1.kalman filter setup  
const int stateNum=4;  
const int measureNum=2;


uint32 UFaceFrameCalcThread::Run()
{
    while(!bStop)
    {
       
        if((*Frame).empty())
            continue;
        
        FinalFrame = (*Frame).clone();
      
        cv::Mat smallMat;
        cv::resize(FinalFrame, smallMat, cv::Size(), 1.0/OptimizeScale, 1.0/OptimizeScale);
        dlib::cv_image<dlib::bgr_pixel> cimg(FinalFrame);
        dlib::cv_image<dlib::bgr_pixel> cimg_small(smallMat);
	
        //Detect faces   
        if (UpdateCount++ % FrameRate == 0) {
            faces = FrontDector(cimg_small);
            if(faces.size() == 0)
            {
                UE_LOG(LogTemp, Error, TEXT("Can't dector face from cimg_small"));
                continue;
            }
        }
       else
       {
           continue;
       }

        // Find the pose of each face.  
        std::vector<dlib::full_object_detection> shapes;
        
        for (unsigned long i = 0; i < faces.size(); ++i) {
            dlib::rectangle r(
                (long)(faces[i].left()*OptimizeScale),
                (long)(faces[i].top()*OptimizeScale ),
                (long)(faces[i].right()*OptimizeScale),
                (long)(faces[i].bottom()*OptimizeScale)
            );
		
            dlib::full_object_detection shape = PoseModel(cimg, r);

            //convert shape to points with kalman
            ProcessShapeWithKalman(shape);
            //calc face feature
            CalcFaceFeature(shape);
            //calc Head Info
            CalcHeadInfo(shape);

            // Custom Face Render
            RenderFace(FinalFrame, shape);
        }
#if PLATFORM_WINDOWS
        cv::imshow("Example", FinalFrame);
#endif

       
        if(UpdateFaceInfoCallback.IsBound())
        {
            UpdateFaceInfoCallback.Broadcast();   
        }
    }
    return 0;
}

void UFaceFrameCalcThread::Stop()
{
    bStop = true;
#if PLATFORM_WINDOWS
    cv::destroyWindow("Example");
#endif
}

void UFaceFrameCalcThread::Start(cv::Mat* Frame, int FrameRate, int OptimizeScale)
{
#if PLATFORM_WINDOWS
    cv::namedWindow("Example", cv::WINDOW_AUTOSIZE);
 #endif
    this->Frame = Frame;
    this->FrameRate = FrameRate;
    this->OptimizeScale = OptimizeScale;
    this->UpdateCount = 0;

    FrontDector = dlib::get_frontal_face_detector();
	
    FString ShapePath = FPaths::Combine(FPaths::GameContentDir(), TEXT("TestRes/shape_predictor_68_face_landmarks.dat"));
#if PLATFORM_ANDROID
    extern const FString &GetFileBasePath();
    FString ProjectPath = GetFileBasePath();
    ShapePath = FPaths::Combine(ProjectPath, FApp::GetProjectName(), TEXT("Content/TestRes/shape_predictor_68_face_landmarks.dat"));
#endif
    std::ifstream StreamIn(TCHAR_TO_UTF8(*ShapePath), std::ios::binary);
	
    deserialize(PoseModel, StreamIn);
    StreamIn.clear();
    StreamIn.close();
    UE_LOG(LogTemp, Warning, TEXT("deserialize end, begin deserialize ShapePath = %s"), *ShapePath );

    //Init For Head Info
    object_pts.push_back(cv::Point3d(6.825897, 6.760612, 4.402142));     //#33 left brow left corner
    object_pts.push_back(cv::Point3d(1.330353, 7.122144, 6.903745));     //#29 left brow right corner
    object_pts.push_back(cv::Point3d(-1.330353, 7.122144, 6.903745));    //#34 right brow left corner
    object_pts.push_back(cv::Point3d(-6.825897, 6.760612, 4.402142));    //#38 right brow right corner
    object_pts.push_back(cv::Point3d(5.311432, 5.485328, 3.987654));     //#13 left eye left corner
    object_pts.push_back(cv::Point3d(1.789930, 5.393625, 4.413414));     //#17 left eye right corner
    object_pts.push_back(cv::Point3d(-1.789930, 5.393625, 4.413414));    //#25 right eye left corner
    object_pts.push_back(cv::Point3d(-5.311432, 5.485328, 3.987654));    //#21 right eye right corner
    object_pts.push_back(cv::Point3d(2.005628, 1.409845, 6.165652));     //#55 nose left corner
    object_pts.push_back(cv::Point3d(-2.005628, 1.409845, 6.165652));    //#49 nose right corner
    object_pts.push_back(cv::Point3d(2.774015, -2.080775, 5.048531));    //#43 mouth left corner
    object_pts.push_back(cv::Point3d(-2.774015, -2.080775, 5.048531));   //#39 mouth right corner
    object_pts.push_back(cv::Point3d(0.000000, -3.116408, 6.097667));    //#45 mouth central bottom corner
    object_pts.push_back(cv::Point3d(0.000000, -7.415691, 4.070434));    //#6 chin corner

    reprojectsrc.push_back(cv::Point3d(10.0, 10.0, 10.0));
    reprojectsrc.push_back(cv::Point3d(10.0, 10.0, -10.0));
    reprojectsrc.push_back(cv::Point3d(10.0, -10.0, -10.0));
    reprojectsrc.push_back(cv::Point3d(10.0, -10.0, 10.0));
    reprojectsrc.push_back(cv::Point3d(-10.0, 10.0, 10.0));
    reprojectsrc.push_back(cv::Point3d(-10.0, 10.0, -10.0));
    reprojectsrc.push_back(cv::Point3d(-10.0, -10.0, -10.0));
    reprojectsrc.push_back(cv::Point3d(-10.0, -10.0, 10.0));

    reprojectdst.resize(8);

    //Init kalman
    for(int i = 0; i < 68; i++)
    {
        cv::KalmanFilter KF = cv::KalmanFilter(stateNum, measureNum, 0);
        cv::Mat measurement = cv::Mat::zeros(measureNum, 1, CV_32F);
        //A 状态转移矩阵
        KF.transitionMatrix = (cv::Mat_<float>(stateNum, stateNum) << 1, 0, 1, 0,
          0, 1, 0, 1,
          0, 0, 1, 0,
          0, 0, 0, 1);
        //这里没有设置控制矩阵B，默认为零
        setIdentity(KF.measurementMatrix);//H=[1,0,0,0;0,1,0,0] 测量矩阵
        setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-5));//Q高斯白噪声，单位阵
        setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));//R高斯白噪声，单位阵
        setIdentity(KF.errorCovPost, cv::Scalar::all(1));//P后验误差估计协方差矩阵，初始化为单位阵
        randn(KF.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));//初始化状态为随机值
        KFs[i] = KF;
        measurements[i] = measurement;
        
    }
   
  
   
    
    // Kalman = cvCreateKalman(stateNum, measureNum, 0);
    // process_noise = cvCreateMat( stateNum, 1, CV_32FC1 );  
    // measurement = cvCreateMat( measureNum, 1, CV_32FC1 );//measurement(x,y)  
    // rng = cvRNG(-1);
    // memcpy( Kalman->transition_matrix->data.fl,A,sizeof(A));
    // cvSetIdentity(Kalman->measurement_matrix,cvRealScalar(1) );  
    // cvSetIdentity(Kalman->process_noise_cov,cvRealScalar(1e-5));  
    // cvSetIdentity(Kalman->measurement_noise_cov,cvRealScalar(1e-1));  
    // cvSetIdentity(Kalman->error_cov_post,cvRealScalar(1)); 
    // //initialize post state of kalman filter at random  
    // cvRandArr(&rng,Kalman->state_post,CV_RAND_UNI,cvRealScalar(0),cvRealScalar(Frame->cols));
    
    //start thread
    FRunnableThread::Create(this, TEXT("Dlib Frame THread"));
}

void UFaceFrameCalcThread::RenderFace(cv::Mat& img, const dlib::full_object_detection& shape)
{
    if(shape.num_parts() != 68)
    {
    	UE_LOG(LogTemp, Error, TEXT("Dlib shape number is not 68"));
    }

    //画线
    // DrawPolyLine(img, shape, 0, 16);           // Jaw line
    // DrawPolyLine(img, shape, 17, 21);          // Left eyebrow
    // DrawPolyLine(img, shape, 22, 26);          // Right eyebrow
    // DrawPolyLine(img, shape, 27, 30);          // Nose bridge
    // DrawPolyLine(img, shape, 30, 35, true);    // Lower nose
    // DrawPolyLine(img, shape, 36, 41, true);    // Left eye
    // DrawPolyLine(img, shape, 42, 47, true);    // Right Eye
    // DrawPolyLine(img, shape, 48, 59, true);    // Outer lip
    // DrawPolyLine(img, shape, 60, 67, true);    // Inner lip

    //画点
    DrawCircle(img, shape, 7.0, cv::Scalar(255, 0, 0));
}

void UFaceFrameCalcThread::ProcessShapeWithKalman(const dlib::full_object_detection& shape)
{
    for(int i = 0; i < (int)(shape.num_parts()); i++)
    {
        cv::Mat prediction = KFs[i].predict();
        cv::Point predict_pt = cv::Point((int)prediction.at<float>(0), (int)prediction.at<float>(1));
        measurements[i].at<float>(0) = (float)shape.part(i).x();
        measurements[i].at<float>(1) = (float)shape.part(i).y();
        KFs[i].correct(measurements[i]);
        facePoints[i] = predict_pt;
    }
}

void UFaceFrameCalcThread::DrawPolyLine(cv::Mat& img, const dlib::full_object_detection& d, const int start,
                                        const int end, bool isClosed)
{
    std::vector <cv::Point> points;
    for (int i = start; i <= end; ++i)
    {
        points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
    }
    cv::polylines(img, points, isClosed, cv::Scalar(255, 0, 0), 2, 16);
}

void UFaceFrameCalcThread::DrawCircle(cv::Mat& img, const dlib::full_object_detection& d, const float radius,
    cv::Scalar color)
{
    for(int i = 0; i < (int)(d.num_parts()); i++)
    {
        // cv::circle(img, cv::Point(d.part(i).x(), d.part(i).y()), radius, color);
        cv::circle(img, facePoints[i], radius, cv::Scalar(0, 0, 255));
        // char p[32];
        // sprintf(p,"%d",i);
        // string str=p;
        // cv::putText(img, str, cv::Point(d.part(i).x(), d.part(i).y()), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
    }
}

void UFaceFrameCalcThread::CalcFaceFeature(const dlib::full_object_detection& shape)
{
    
    float FaceHeight = FVector2D::Distance(FVector2D(facePoints[27].x, facePoints[27].y),
        FVector2D(facePoints[8].x, facePoints[8].y));
    
    float FaceWidth = FVector2D::Distance(FVector2D(facePoints[0].x, facePoints[0].y),
       FVector2D(facePoints[16].x, facePoints[16].y));
    
    float LengthBase = (FaceHeight + FaceWidth)/2;

   
    // //left eye
    // float LeftEye1 = FVector2D::Distance(FVector2D(facePoints[37].x, facePoints[37].y),
    //     FVector2D(facePoints[41].x, facePoints[41].y));
    // float LeftEye2 = FVector2D::Distance(FVector2D(facePoints[38].x, facePoints[38].y),
    //       FVector2D(facePoints[40].x, facePoints[40].y));
    //
    // LeftEyeWeight = ((LeftEye1 + LeftEye2)/2)/FaceHeight;
    //
    //
    // //right eye
    // float RightEye1 = FVector2D::Distance(FVector2D(facePoints[43].x, facePoints[43].y),
    //     FVector2D(facePoints[47].x, facePoints[47].y));
    // float RightEye2 = FVector2D::Distance(FVector2D(facePoints[44].x, facePoints[44].y),
    //     FVector2D(facePoints[46].x, facePoints[46].y));
    //
    // RightEyeWeight = ((RightEye1 + RightEye2)/2)/FaceHeight;
    
    if((shape.part(39).x() - shape.part(36).x()) == 0)
    {
        LeftEyeWeight = 0;
    }
    else
    {
        LeftEyeWeight = FMath::Abs((float)(shape.part(41).y() + shape.part(40).y() - shape.part(37).y() - shape.part(38).y())/(shape.part(39).x() - shape.part(36).x()));
    }
    if((shape.part(45).x() - shape.part(42).x()) == 0)
    {
        RightEyeWeight = 0;
    }
    else
    {
        RightEyeWeight = FMath::Abs((float)(shape.part(47).y() + shape.part(46).y() - shape.part(43).y() - shape.part(44).y())/(shape.part(45).x() - shape.part(42).x()));
    }
   
    //Mouse
    float MouseWidh = FVector2D::Distance(FVector2D(facePoints[51].x, facePoints[51].y),
        FVector2D(facePoints[57].x, facePoints[57].y));
    MouseWeight = (MouseWidh/LengthBase - 0.13)*1.27 + 0.02;
    
}

void UFaceFrameCalcThread::CalcHeadInfo(const dlib::full_object_detection& shape)
{
   
    //fill in 2D ref points, annotations follow https://ibug.doc.ic.ac.uk/resources/300-W/
    image_pts.push_back(facePoints[17]); //#17 left brow left corner
    image_pts.push_back(facePoints[21]); //#21 left brow right corner
    image_pts.push_back(facePoints[22]); //#22 right brow left corner
    image_pts.push_back(facePoints[26]); //#26 right brow right corner
    image_pts.push_back(facePoints[36]); //#36 left eye left corner
    image_pts.push_back(facePoints[39]); //#39 left eye right corner
    image_pts.push_back(facePoints[42]); //#42 right eye left corner
    image_pts.push_back(facePoints[45]); //#45 right eye right corner
    image_pts.push_back(facePoints[31]); //#31 nose left corner
    image_pts.push_back(facePoints[35]); //#35 nose right corner
    image_pts.push_back(facePoints[48]); //#48 mouth left corner
    image_pts.push_back(facePoints[54]); //#54 mouth right corner
    image_pts.push_back(facePoints[57]); //#57 mouth central bottom corner
    image_pts.push_back(facePoints[8]);   //#8 chin corner

    //calc pose
    cv::solvePnP(object_pts, image_pts, cam_matrix, dist_coeffs, rotation_vec, translation_vec);

    //reproject
    cv::projectPoints(reprojectsrc, rotation_vec, translation_vec, cam_matrix, dist_coeffs, reprojectdst);

    //draw axis
    cv::line(FinalFrame, reprojectdst[0], reprojectdst[1], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[1], reprojectdst[2], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[2], reprojectdst[3], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[3], reprojectdst[0], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[4], reprojectdst[5], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[5], reprojectdst[6], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[6], reprojectdst[7], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[7], reprojectdst[4], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[0], reprojectdst[4], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[1], reprojectdst[5], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[2], reprojectdst[6], cv::Scalar(0, 0, 255));
    cv::line(FinalFrame, reprojectdst[3], reprojectdst[7], cv::Scalar(0, 0, 255));

    //calc euler angle
    cv::Rodrigues(rotation_vec, rotation_mat);
    cv::hconcat(rotation_mat, translation_vec, pose_mat);
    cv::decomposeProjectionMatrix(pose_mat, out_intrinsics, out_rotation, out_translation, cv::noArray(), cv::noArray(), cv::noArray(), euler_angle);
    image_pts.clear();

    HeadRotator.Pitch = -euler_angle.at<double>(0);
    HeadRotator.Yaw = euler_angle.at<double>(1);
    HeadRotator.Roll = -euler_angle.at<double>(2);
}