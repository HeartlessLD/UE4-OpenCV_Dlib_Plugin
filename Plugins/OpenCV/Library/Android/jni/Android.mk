LOCAL_PATH := $(call my-dir)

PATH_TO_LIBS = libs
 


include $(CLEAR_VARS)
LOCAL_MODULE := opencv_java3
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libopencv_java3.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := UE4
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libUE4.so
include $(PREBUILT_SHARED_LIBRARY)
