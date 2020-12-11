#include "functionexample.h"

std::string functionexample::hello() { return "Hello World"; }

int functionexample::add(int a, int b) { return a + b; }

double functionexample::processImages(std::string PathName) {
  DcmFileFormat file;

  OFCondition status1 = file.loadFile(PathName.c_str());

  DcmDataset* dataset = file.getDataset();
  E_TransferSyntax xfer = dataset->getOriginalXfer();

  DicomImage DCM_image(dataset, xfer);

  cv::Mat src(int(DCM_image.getWidth()), int(DCM_image.getHeight()), CV_8UC1,
              (Uint8*)DCM_image.getOutputData(8));

  cv::Mat src_8U;
  cv::Mat src_16U;

  int nWidth = DCM_image.getWidth();
  int nHeight = DCM_image.getHeight();
  auto data = DCM_image.getOutputData();
  unsigned short* pixelData = (unsigned short*)(data);

  if (pixelData != NULL) {
    src_16U = cv::Mat(nHeight, nWidth, CV_16UC1, cv::Scalar::all(0.0));
    Uint16* Data = nullptr;
    for (int i = 0; i < nHeight; i++) {
      Data = src_16U.ptr<Uint16>(i);
      for (int j = 0; j < nWidth; j++) {
        *Data++ = pixelData[i * nWidth + j];
      }
    }
  }

  src_8U = cv::Mat(int(DCM_image.getHeight()), int(DCM_image.getWidth()),
                   CV_8UC1, (Uint8*)DCM_image.getOutputData(8));

  cv::Mat grad_x, grad_y;
  cv::Mat abs_grad_x, abs_grad_y, dst;

  Sobel(src_8U, grad_x, CV_16S, 1, 0, 3, 1, 1, cv::BORDER_DEFAULT);
  convertScaleAbs(grad_x, abs_grad_x);
  // imshow("x axis direction soble", abs_grad_x);

  Sobel(src_8U, grad_y, CV_16S, 0, 1, 3, 1, 1, cv::BORDER_DEFAULT);
  convertScaleAbs(grad_y, abs_grad_y);
  // imshow("y axis direction soble", abs_grad_y);

  addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);

  int rows = dst.rows;
  int cols = dst.cols;

  std::vector<int> vRows;
  abs_grad_x.row(cols / 2).copyTo(vRows);
  std::vector<int> vRowsIndex;
  for (auto it = vRows.begin(); it != vRows.end(); it++) {
    if (*it >= 30 && *it <= 100) vRowsIndex.push_back(it - vRows.begin());
  }

  int centerRowIndex = (*vRowsIndex.begin() + *(vRowsIndex.end() - 1)) / 2;

  std::vector<double> centerlineGrayscale;

  src_16U.row((rows/2)-1).copyTo(centerlineGrayscale);

  double symmetry = 0.0;
  double gap = 9;
  for (int i = 1; i <= 10; i++) {
    symmetry = max(centerlineGrayscale[centerRowIndex + i * gap] /
                       centerlineGrayscale[centerRowIndex - i * gap],
                   symmetry);
  }



    // this sub-vector is what I used to calculate flatteness
  std::vector<int> subVRowsScale;
  double centerElement = centerlineGrayscale[centerRowIndex];
  double rangeMinElement =
      *std::min_element(centerlineGrayscale.begin() + *vRowsIndex.begin(),
                        centerlineGrayscale.begin() + *(vRowsIndex.end() - 1));

  double flatteness = centerElement / rangeMinElement;

 
  // cv::imshow("DicomSrc", src);
  cv::imshow("16U", src_16U);
  // cv::imshow("sobel", dst);

  cv::waitKey();
  cv::destroyAllWindows();

  return symmetry;
}

Napi::Number functionexample::ImageWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();

  double returnValue = functionexample::processImages(first.ToString());

  return Napi::Number::New(env, returnValue);
}

Napi::String functionexample::HelloWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::String returnValue = Napi::String::New(env, functionexample::hello());
  return returnValue;
}

Napi::Number functionexample::AddWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }

  Napi::Number first = info[0].As<Napi::Number>();
  Napi::Number second = info[1].As<Napi::Number>();

  int returnValue =
      functionexample::add(first.Int32Value(), second.Int32Value());

  return Napi::Number::New(env, returnValue);
}

Napi::Object functionexample::Init(Napi::Env env, Napi::Object exports) {
  exports.Set("hello", Napi::Function::New(env, functionexample::HelloWrapped));
  exports.Set("add", Napi::Function::New(env, functionexample::AddWrapped));
  exports.Set("print", Napi::Function::New(env, functionexample::ImageWrapped));
  return exports;
}
