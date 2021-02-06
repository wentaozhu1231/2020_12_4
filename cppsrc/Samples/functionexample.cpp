#include "functionexample.h"

#include <string>
std::string functionexample::hello() { return "Hello World"; }

int functionexample::add(const int& a, const int& b) { return a + b; }

template <int x>
double functionexample::process_images(const std::string& path_name) {
  const std::string file_path = path_name;

  if (file_path.length() == 0) exit(EXIT_FAILURE);

  DicomImage dcm_image(file_path.c_str());

  const auto rows = dcm_image.getHeight();
  const auto cols = dcm_image.getWidth();

  DcmFileFormat file_format;
  const auto condition = file_format.loadFile(file_path.c_str());
  if (condition.bad()) {
    std::cerr << "cannot load the file. " << std::endl;
    return EXIT_FAILURE;
  }
  DcmDataset* data_set = file_format.getDataset();
  const E_TransferSyntax xfer = data_set->getOriginalXfer();
  // decompress data set if compressed
  data_set->chooseRepresentation(xfer, nullptr);
  // data_set->chooseRepresentation(EXS_LittleEndianExplicit, nullptr);
  DcmElement* element = nullptr;
  OFCondition result = data_set->findAndGetElement(DCM_PixelData, element);
  if (result.bad() || element == nullptr) return 1;
  unsigned short* pix_data;
  result = element->getUint16Array(pix_data);

  cv::Mat src_16_u(rows, cols, CV_16UC1, cv::Scalar::all(0));
  unsigned short* data = nullptr;
  for (auto i = 0; i < rows; i++) {
    // could use dst2.at<unsigned short>(i, j) = ?
    data = src_16_u.ptr<unsigned short>(i);
    for (auto j = 0; j < cols; j++) {
      auto temp = pix_data[i * cols + j];
      temp = temp == 65535 ? 0 : temp;
      *data++ = temp;
    }
  }
  std::vector<int> m_v;
  for (auto i = 0; i < cols; i++) {
    m_v.emplace_back(pix_data[(rows / 2) * cols + i]);
  }
  /*for (auto x : m_v) {
    std::cout << x << " ";
  }
  std::cout << std::endl;*/

  cv::Mat src_8_u(static_cast<int>(dcm_image.getHeight()),
                  static_cast<int>(dcm_image.getWidth()), CV_8UC1,
                  (Uint8*)dcm_image.getOutputData(8));

  cv::Mat binary;

  double t =
      threshold(src_8_u, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
  // std::cout << "threshold : %.2f\n" << t << std::endl;
  // cv::imshow("binary", binary);

  cv::Mat grad_x, grad_y;
  cv::Mat abs_grad_x, abs_grad_y, dst;
  Sobel(binary, grad_x, CV_16S, 1, 0, 3, 1, 1, cv::BORDER_DEFAULT);
  convertScaleAbs(grad_x, abs_grad_x);
  Sobel(binary, grad_y, CV_16S, 0, 1, 3, 1, 1, cv::BORDER_DEFAULT);
  convertScaleAbs(grad_y, abs_grad_y);
  addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);

  // cv::imshow("sobel", dst);

  std::vector<int> v_rows;
  abs_grad_x.row(cols / 2).copyTo(v_rows);
  std::vector<int> v_rows_index;

  // here, the logic could be wrong
  // I don't know whether this is a good way to filter those strange line
  for (auto it = v_rows.begin(); it != v_rows.end(); ++it) {
    if ((*it == 255) && (it > v_rows.begin() + 20) && (it < v_rows.end() - 21))
    // if (*it >= 10 && *it <= 100)
    {
      // subVRowsScale.push_back(*it);
      v_rows_index.push_back(it - v_rows.begin());
    }
  }

  /*for (auto x : v_rows_index) std::cout << x << " ";
  std::cout << std::endl;*/

  auto centerRowIndex = (*v_rows_index.begin() + *(v_rows_index.end() - 1)) / 2;

  std::vector<int> v_cols;
  // use previous calculated result as new index
  abs_grad_y.col(centerRowIndex).copyTo(v_cols);
  std::vector<int> v_cols_index;
  // in order to prevent image is a rectangular
  int centerColIndex = rows / 2;

  for (auto it = v_cols.begin(); it != v_cols.end(); ++it) {
    if (*it == 255) v_cols_index.push_back(it - v_cols.begin());
  }
  // if this is a square filed instead of a rectangular
  if (!v_cols_index.empty()) {
    /* for (auto x : v_cols_index) std::cout << x << " ";
     std::cout << std::endl;*/
    // in order to prevent noise,parenthesis never redundant
    if ((*(v_cols_index.end() - 1) - *v_cols_index.begin()) > cols / 7) {
      centerColIndex = (*v_cols_index.begin() + *(v_cols_index.end() - 1)) / 2;
    }
  }

  std::vector<double> centerlineGrayscale;

  // cout << src_16U.row(centerColIndex) << endl;
  src_16_u.row(centerColIndex).copyTo(centerlineGrayscale);

  auto max_element =
      *std::max_element(centerlineGrayscale.begin(), centerlineGrayscale.end());

  auto center_element = centerlineGrayscale[centerRowIndex];

  auto half_element = 0.5 * (max_element + center_element);

  // this sub-vector is what I used to calculate flatteness
  std::vector<int> sub_v_rows_scale;

  auto rangeMinElement = *std::min_element(
      centerlineGrayscale.begin() + *v_rows_index.begin(),
      centerlineGrayscale.begin() + *(v_rows_index.end() - 1));

  auto flatness = center_element / rangeMinElement;

  //std::cout << "the flatness of this graph is :" << flatness * 100 << "%"
  //          << std::endl;

  auto symmetry = 0.0;
  double gap = 9;
  for (auto i = 1; i <= 10; i++) {
    symmetry = max(centerlineGrayscale[centerRowIndex + i * gap] /
                       centerlineGrayscale[centerRowIndex - i * gap],
                   symmetry);
  }

  //std::cout << "the symmetry of this graph is: " << symmetry << std::endl;

  // cv::imshow("16U", src_16_u);

  // cv::waitKey();
  // cv::destroyAllWindows();
  if (x == 1) return symmetry;
  if (x == 2) return flatness;
}

Napi::Number functionexample::SymmetryWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();

  double returnValue = functionexample::process_images<1>(first.ToString());

  return Napi::Number::New(env, returnValue);
}
Napi::Number functionexample::FlatnessWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();

  double returnValue = functionexample::process_images<2>(first.ToString());

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
  // exports.Set("print", Napi::Function::New(env,
  // functionexample::SymmetryWrapped));
  exports.Set("get_symmetry",
              Napi::Function::New(env, functionexample::SymmetryWrapped));
  exports.Set("get_flatness",
              Napi::Function::New(env, functionexample::FlatnessWrapped));
  return exports;
}
