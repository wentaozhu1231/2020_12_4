#include <napi.h>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"

namespace functionexample {

    std::string hello();
    Napi::String HelloWrapped(const Napi::CallbackInfo& info);

    int add(const int&a, const int&b);
    Napi::Number AddWrapped(const Napi::CallbackInfo& info);

    template <int x>
    double process_two_images(const std::string& path_name_1,
                          const std::string& path_name_2);

    template<int x>
    double process_images(const std::string& path_name_1);
    
    Napi::Number SymmetryWrapped(const Napi::CallbackInfo& info);

    Napi::Number FlatnessWrapped(const Napi::CallbackInfo& info);

    Napi::Number FWHMWrapped(const Napi::CallbackInfo& info);

    Napi::Number PenumbraWrapped(const Napi::CallbackInfo& info);

    Napi::Number AngleWrapped(const Napi::CallbackInfo& info);
    Napi::Number CenterDistanceWrapped(const Napi::CallbackInfo& info);

    Napi::Object Init(Napi::Env env, Napi::Object exports);
    
}
