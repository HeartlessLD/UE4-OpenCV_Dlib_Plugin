// FaceRecognition.cpp : Defines the exported functions for the DLL application.
//

#include "../../Public/Frame/CameraFrame.h"

#include "Kismet/BlueprintPlatformLibrary.h"
#include <array>
#include <iostream>
#include <mutex>
#include "opencv2/imgproc/imgproc.hpp"


using namespace cv;
using namespace std;

std::mutex frame_mutex;
cv::Mat FCameraFrame::androidFrame;


#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJni.h"
#endif

#if PLATFORM_ANDROID

JavaVM *gJavaVm = NULL;

jclass AndroidCameraClass;
jmethodID midSetPreviewSize;
jobject AndroidCameraObject;
jobject  CameraObject;

//Start Java Camera
bool setPreviewSize(int width, int height) {

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr == AndroidCameraObject) {
		ALOGW("AndroidCameraObject is null. Check InitJNI call");
		return false;
	}

	if (nullptr == midSetPreviewSize) {
		ALOGW("MethodId for SetPreviewSize is null");
		return false;
	}

	if (nullptr == Env) {
		ALOGW("Env is null");
		return false;
	}
	ALOGW("Call setPreviewSize in java");
	FCameraFrame::androidFrame = Mat::zeros(height, width, CV_8UC3);
	Env->CallVoidMethod(AndroidCameraObject, midSetPreviewSize, width, height);
	return true;
}
#endif



FCameraFrame::FCameraFrame()
{
	m_isOpen = false;
#if PLATFORM_ANDROID
	//Init Java Env
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	//Init Camera Class
	AndroidCameraClass = FAndroidApplication::FindJavaClass("org/getid/facerecognition/AndroidCamera");
	if(AndroidCameraClass == 0)
	{
		ALOGW("AndroidCameraClass is null");
	}
	else
	{
		ALOGW("AndroidCameraClass is not null");
	}
	// Create Camera Instance
	jmethodID ConstructFunc = Env->GetMethodID(AndroidCameraClass, "<init>", "()V");
	if(ConstructFunc == 0)
	{
		ALOGW("ConstructFunc is null");
	}
	else
	{
		ALOGW("ConstructFunc is not null");
	}
	AndroidCameraObject = Env->NewObject(AndroidCameraClass, ConstructFunc);
	if(AndroidCameraObject == 0)
	{
		ALOGW("AndroidCameraObject is null");
	}
	else
	{
		ALOGW("AndroidCameraObject is %p", AndroidCameraObject);
	}
	//Get SetPreviewSize Func
	midSetPreviewSize = Env->GetMethodID(AndroidCameraClass, "SetPreviewSize", "(II)V");
	if(midSetPreviewSize == 0)
	{
		ALOGW("midSetPreviewSize is null");
	}
	else
	{
		ALOGW("midSetPreviewSize is not null");
	}
#endif
}

bool FCameraFrame::captureCamera(int cameraId, int width, int height)
{
#if PLATFORM_ANDROID
	m_isOpen = setPreviewSize(width, height);
	return m_isOpen;
#endif

	stream = new cv::VideoCapture();
	stream->open(cameraId);
	stream->set(CV_CAP_PROP_FRAME_WIDTH, width);
	stream->set(CV_CAP_PROP_FRAME_HEIGHT, height);

	m_isOpen = stream->isOpened();
	return m_isOpen;
}

bool FCameraFrame::getActualVideoSize(int& width, int& height)
{
	if (m_isOpen && stream != nullptr) {
		Mat frame;
		getFrame(frame);

		if (!frame.data) {
			return false;
		}

		width = frame.size().width;
		height = frame.size().height;
		return true;
	}
	return false;
}

void FCameraFrame::releaseCamera()
{
	if (m_isOpen && stream != nullptr && stream->isOpened()) {
		stream->release();
		delete stream;
	}

	m_isOpen = false;
}

bool FCameraFrame::loadClassifierFile(const char *classifierFile)
{
	return faceCascade.load(classifierFile);
}

void FCameraFrame::getFrame(Mat& frame)
{
	if (!m_isOpen) {
		return;
	}
	
#if PLATFORM_ANDROID
	frame_mutex.lock();
	frame = FCameraFrame::androidFrame.clone();
	
	if(UBlueprintPlatformLibrary::GetDeviceOrientation() == EScreenOrientation::Portrait)
	{
		//rotate 90 degree
		transpose(frame, frame);
		flip(frame, frame, -1);
	}
	else
	{
		flip(frame, frame, 1);
	}

	frame_mutex.unlock();
#else
	stream->read(frame);
	flip(frame, frame, 1);
#endif

}

void FCameraFrame::update(std::vector<uchar>& buffer)
{
	Mat frame;
	getFrame(frame);

	if (!frame.data) {
		return;
	}

	std::vector<cv::Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	faceCascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(60, 60));

	for (size_t i = 0; i < faces.size(); i++)
	{
		Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
		ellipse(frame, center, Size(faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);
	}

	(buffer).assign(frame.datastart, frame.dataend);
}



#if PLATFORM_ANDROID

extern "C"
void
Java_org_getid_facerecognition_AndroidCamera_InitJNI(JNIEnv* env, jobject thiz)
{
	// AndroidCameraObject = thiz;
	// ALOGW("c++ InitJNI AndroidCameraObject = %p", thiz);
}

extern "C"
jboolean
Java_org_getid_facerecognition_AndroidCamera_FrameProcessing(
	JNIEnv* env, jobject thiz,
	jint width, jint height,
	jbyteArray NV21FrameData)
{

	jbyte* pNV21FrameData = env->GetByteArrayElements(NV21FrameData, NULL);
	
	ALOGW("Java_org_getid_facerecognition_AndroidCamera_FrameProcessing");
	cv::Mat yuv(height + height / 2, width, CV_8UC1, (uchar*)pNV21FrameData);
	cv::Mat bgr(height, width, CV_8UC3);
	cv::cvtColor(yuv, bgr, CV_YUV2BGR_NV21);
	
	frame_mutex.lock();
	FCameraFrame::androidFrame = bgr;
	frame_mutex.unlock();
	
	ALOGW("androidFrame col = %d row = %d", height, width);
	
	env->ReleaseByteArrayElements(NV21FrameData, pNV21FrameData, JNI_ABORT);

	return true;
}


#endif
