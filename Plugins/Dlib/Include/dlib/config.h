

// If you are compiling dlib as a shared library and installing it somewhere on your system
// then it is important that any programs that use dlib agree on the state of the
// DLIB_ASSERT statements (i.e. they are either always on or always off).  Therefore,
// uncomment one of the following lines to force all DLIB_ASSERTs to either always on or
// always off.  If you don't define one of these two macros then DLIB_ASSERT will toggle
// automatically depending on the state of certain other macros, which is not what you want
// when creating a shared library.
//#define ENABLE_ASSERTS       // asserts always enabled 
//#define DLIB_DISABLE_ASSERTS // asserts always disabled 

//#define DLIB_ISO_CPP_ONLY
//#define DLIB_NO_GUI_SUPPORT
//#define DLIB_ENABLE_STACK_TRACE

// You should also consider telling dlib to link against libjpeg, libpng, libgif, fftw, CUDA, 
// and a BLAS and LAPACK library.  To do this you need to uncomment the following #defines.
// #define DLIB_JPEG_SUPPORT
// #define DLIB_PNG_SUPPORT
// #define DLIB_GIF_SUPPORT
// #define DLIB_USE_FFTW
// #define DLIB_USE_BLAS
// #define DLIB_USE_LAPACK
// #define DLIB_USE_CUDA
#ifndef STDTOSTRING_H
#define STDTOSTRING_H
    #if defined(ANDROID)
    #ifdef ATOMIC_INT_LOCK_FREE
    #undef ATOMIC_INT_LOCK_FREE
    #endif
    #define ATOMIC_INT_LOCK_FREE 2
    #include <bits/exception_ptr.h>
    #include <bits/nested_exception.h>
    
    #include <string>
    #include <sstream>

    using namespace std;
    namespace std {
        template <typename T> std::string to_string(const T& n) {
            std::ostringstream stm;
            stm << n;
            return stm.str();
        }

        template <typename T> T round(T v) {
            return (v > 0) ? (v + 0.5) : (v - 0.5);
        }
    }
    #endif
#endif
