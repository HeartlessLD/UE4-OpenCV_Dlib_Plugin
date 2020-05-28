// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class OpenCV : ModuleRules
{
   
    public OpenCV(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Projects",
              
				// ... add other public dependencies that you statically link with here ...
                // For WebCamera Draw Texture To World
                "RHI",
                "RenderCore",
               
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
        	//for jni call
    		PrivateDependencyModuleNames.Add("Launch");
            //for android permission
            PrivateDependencyModuleNames.Add("AndroidPermission");
        }

            DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        string OpenCVIncludePath = Path.Combine(ModuleDirectory, "../../Include");
        string OpenCVLibraryPath = Path.Combine(ModuleDirectory, "../../Library");
        
        System.Console.WriteLine("----Architecture -----" + Target.Architecture);
        
        //Load OpenCV Include

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            OpenCVIncludePath = Path.Combine(OpenCVIncludePath, "Win64");
            
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            OpenCVIncludePath = Path.Combine(OpenCVIncludePath, "Android");
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            OpenCVIncludePath = Path.Combine(OpenCVIncludePath, "IOS");
        }
        System.Console.WriteLine("----Include -----" + OpenCVIncludePath);
        PublicIncludePaths.AddRange(new string[] { OpenCVIncludePath });

        //Load OpenCV Library
        string OpenCVFinalLibraryPath = "";
        if(Target.Platform == UnrealTargetPlatform.Win64)
        {
	        OpenCVFinalLibraryPath = Path.Combine(OpenCVLibraryPath, "Win64");
	        // Add Library Path 
	        PublicLibraryPaths.Add(OpenCVFinalLibraryPath);
	        
	        PublicAdditionalLibraries.Add(Path.Combine(OpenCVFinalLibraryPath, "opencv_world340.lib"));
	        
            //add dynamic library
            PublicDelayLoadDLLs.Add("opencv_ffmpeg340_64.dll");
            PublicDelayLoadDLLs.Add("opencv_world340.dll");
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(OpenCVFinalLibraryPath, "opencv_ffmpeg340_64.dll"))); //for package
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(OpenCVFinalLibraryPath, "opencv_world340.dll")));  //for package
            
            System.Console.WriteLine("----Library -----" + OpenCVFinalLibraryPath);
        }
        else if(Target.Platform == UnrealTargetPlatform.Android)
        {
	        
            string[] AndroidTarget = { "arm64-v8a", "armeabi", "armeabi-v7a", "mips", "mips64", "x86", "x86_64"  };
            
            string[] ThirdLibName = { "libcpufeatures.a", "libIlmImf.a", "liblibjasper.a", "liblibjpeg.a", 
                "liblibprotobuf.a", "liblibtiff.a", "liblibwebp.a", "libtbb.a", "libtegra_hal.a"
            };
            string[] ThirdLibName_x86 = { "libcpufeatures.a", "libIlmImf.a", "libippicv.a", "libippiw.a", "libittnotify.a",
	            "liblibjasper.a", "liblibjpeg.a",  "liblibprotobuf.a", "liblibtiff.a", "liblibwebp.a", "libtbb.a"
            };
            string[] OpenCVLibName = { "libopencv_calib3d.a", "libopencv_core.a", "libopencv_flann.a", "libopencv_highgui.a",
                "libopencv_imgcodecs.a", "libopencv_imgproc.a", "libopencv_ml.a", "libopencv_objdetect.a",
                "libopencv_photo.a", "libopencv_shape.a", "libopencv_stitching.a", "libopencv_superres.a", "libopencv_video.a",
                "libopencv_videoio.a", "libopencv_videostab.a", "libopencv_dnn.a", "libopencv_features2d.a"
            };
            OpenCVFinalLibraryPath = Path.Combine(OpenCVLibraryPath, "Android");
            
            //add Third lib
            for (int i = 0; i < 3; i++)
            {
	            for (int j = 0; j < ThirdLibName.Length; j++)
	            {
		            PublicAdditionalLibraries.Add(Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", AndroidTarget[i], ThirdLibName[j]));
		            System.Console.WriteLine("----Android -----" + Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", AndroidTarget[i], ThirdLibName[j]));
	            }
            }
			
            for (int j = 0; j < ThirdLibName.Length - 1; j++)
            {
	            PublicAdditionalLibraries.Add(Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "mips", ThirdLibName[j]));
	            System.Console.WriteLine("----Android -----" + Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "mips", ThirdLibName[j]));
            }
            for (int j = 0; j < ThirdLibName.Length - 1; j++)
            {
	            PublicAdditionalLibraries.Add(Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "mips64", ThirdLibName[j]));
	            System.Console.WriteLine("----Android -----" + Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "mips64", ThirdLibName[j]));
            }
          
            for (int j = 0; j < ThirdLibName_x86.Length ; j++)
            {
	            PublicAdditionalLibraries.Add(Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "x86", ThirdLibName_x86[j]));
	            System.Console.WriteLine("----Android -----" + Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "x86", ThirdLibName_x86[j]));
            }
            for (int j = 0; j < ThirdLibName_x86.Length; j++)
            {
	            PublicAdditionalLibraries.Add(Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "x86_64", ThirdLibName_x86[j]));
	            System.Console.WriteLine("----Android -----" + Path.Combine(OpenCVFinalLibraryPath, "3rdparty\\libs", "x86_64", ThirdLibName_x86[j]));
            }
			
            //add core lib
            for (int i = 0; i < AndroidTarget.Length; i++)
            {
               
                for (int k = 0; k < OpenCVLibName.Length; k++)
                {
                 PublicAdditionalLibraries.Add(Path.Combine(OpenCVFinalLibraryPath, "libs", AndroidTarget[i], OpenCVLibName[k]));
                 System.Console.WriteLine("----Android -----" + Path.Combine(OpenCVFinalLibraryPath, "libs", AndroidTarget[i], OpenCVLibName[k]));
                }
            }
            
            //load APL.xml
            string APLXmlPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(APLXmlPath, "OpenCV_APL.xml"));
            System.Console.WriteLine("----APLXmlPath -----" + Path.Combine(APLXmlPath, "OpenCV_APL.xml"));
        }
        else if(Target.Platform == UnrealTargetPlatform.IOS)
        {

        }
        
	}
}
