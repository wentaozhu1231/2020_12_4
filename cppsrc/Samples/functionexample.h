#include <napi.h>
#include <iostream>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"

namespace functionexample {

    std::string hello();
    Napi::String HelloWrapped(const Napi::CallbackInfo& info);

    int add(int a, int b);
    Napi::Number AddWrapped(const Napi::CallbackInfo& info);


    double processImages(std::string PathName);
    
    Napi::Number ImageWrapped(const Napi::CallbackInfo& info);


    Napi::Object Init(Napi::Env env, Napi::Object exports);
    
}
