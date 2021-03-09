#include "functionexample.h"

#include <string>
std::string functionexample::hello() { return "Hello World"; }

int functionexample::add(const int& a, const int& b) { return a + b; }

enum project {
  e_symmetry,
  e_flatness,
  e_penumbra,
  e_angle,
  e_FWHM,
  e_center_distance
};

void my_matrix(const std::string file_path, cv::Mat& src_8_u, cv::Mat& src_16_u,
               unsigned long& rows, unsigned long& cols) {
  if (file_path.length() == 0) exit(EXIT_FAILURE);

  DicomImage dcm_image(file_path.c_str());

  rows = dcm_image.getHeight();
  cols = dcm_image.getWidth();

  DcmFileFormat file_format;
  const auto condition = file_format.loadFile(file_path.c_str());
  if (condition.bad()) {
    std::cerr << "cannot load the file. " << std::endl;
    exit(EXIT_FAILURE);
  }
  DcmDataset* data_set = file_format.getDataset();
  const E_TransferSyntax xfer = data_set->getOriginalXfer();
  // decompress data set if compressed
  data_set->chooseRepresentation(xfer, nullptr);
  // data_set->chooseRepresentation(EXS_LittleEndianExplicit, nullptr);
  DcmElement* element = nullptr;
  OFCondition result = data_set->findAndGetElement(DCM_PixelData, element);
  if (result.bad() || element == nullptr) exit(EXIT_FAILURE);

  unsigned short* pix_data;
  result = element->getUint16Array(pix_data);
  // cv::Mat src_16_u(rows, cols, CV_16UC1, cv::Scalar::all(0));
  src_16_u = cv::Mat(rows, cols, CV_16UC1, cv::Scalar::all(0));

  src_8_u = cv::Mat::zeros(rows, cols, CV_8UC1);  //先生成空的目标图片
  double minv = 0.0, maxv = 0.0;
  double* minp = &minv;
  double* maxp = &maxv;

  unsigned short* data = nullptr;
  unsigned char* data_8 = nullptr;
  for (auto i = 0; i < rows; i++) {
    // could use dst2.at<unsigned short>(i, j) = ?
    data = src_16_u.ptr<unsigned short>(i);
    // data_8 = src_8_u.ptr<unsigned char>(i);
    for (auto j = 0; j < cols; j++) {
      auto temp = pix_data[i * cols + j];
      temp = temp == 65535 ? 0 : temp;
      *data++ = temp;

      //  data_8[j] = (temp - minv) / (maxv - minv) * 255;
    }
  }
  // get the max value and the min value
  cv::minMaxIdx(src_16_u, minp, maxp);
  for (int i = 0; i < rows; i++) {
    // get the address of each line
    data = src_16_u.ptr<ushort>(i);
    data_8 = src_8_u.ptr<uchar>(i);
    for (int j = 0; j < cols; ++j) {
      data_8[j] = (data[j] - minv) / (maxv - minv) * 255;

      // this way is shit
      // int temp = img.at<ushort>(i, j);
      // dst.at<uchar>(i, j) = temp;
    }
  }
}
void find_center(cv::Mat& dst, unsigned long& rows, unsigned long& cols,
                 int& centerRowIndex, int& centerColIndex,
                 std::vector<int>& v_cols_index,
                 std::vector<int>& v_rows_index) {
  std::vector<int> v_rows;

  dst.row(cols / 2).copyTo(v_rows);
  // abs_grad_x.row(cols / 2).copyTo(v_rows);

  // here, the logic could be wrong
  // I don't know whether this is a good way to filter those strange line

  // I assume
  for (auto it = v_rows.begin(); it != v_rows.end(); ++it) {
    if ((*it == 255) && (it > v_rows.begin() + 20) && (it < v_rows.end() - 21))
    // if (*it >= 10 && *it <= 100)
    {
      // subVRowsScale.push_back(*it);
      v_rows_index.push_back(it - v_rows.begin());
    }
  }

  /*for (auto x : v_rows_index) cout << x << " ";
  cout << endl;*/

  centerRowIndex = (*v_rows_index.begin() + *(v_rows_index.end() - 1)) / 2;

  std::vector<int> v_cols;
  // use previous calculated result as new index

  dst.col(centerRowIndex).copyTo(v_cols);
  // abs_grad_y.col(centerRowIndex).copyTo(v_cols);

  // in order to prevent image is a rectangular
  centerColIndex = rows / 2;

  for (auto it = v_cols.begin(); it != v_cols.end(); ++it) {
    if ((*it == 255) && (it > v_cols.begin() + 20) && (it < v_cols.end() - 21))
      v_cols_index.push_back(it - v_cols.begin());
  }
  // if this is a square filed instead of a rectangular
  if (!v_cols_index.empty()) {
    /*for (auto x : v_cols_index) cout << x << " ";
    cout << endl;*/
    // in order to prevent noise,parenthesis never redundant
    if ((*(v_cols_index.end() - 1) - *v_cols_index.begin()) > cols / 7) {
      centerColIndex = (*v_cols_index.begin() + *(v_cols_index.end() - 1)) / 2;
    }
  }
}

template <int x>
double functionexample::process_images(const std::string& path_name_1) {
  const std::string file_path_1 = path_name_1;
  //    // "F:\\QC_data\\RI.1.3.46.423632.131000.1606838764.9.dcm";
  //    // "F:\\QC_data\\RI.1.3.46.423632.131000.1606838965.12.dcm";
  //    // "F:\\QC_data\\RI.1.3.46.423632.131000.1606839035.13.dcm";
  //    //"F:\\QC_data\\RI.1.3.46.423632.131000.1606839180.16.dcm";
  //    //"F:\\QC_data\\RI.1.3.46.423632.131000.1606839311.17.dcm";
  //    // "F:\\QC_data\\RI.1.3.46.423632.131000.1606839391.18.dcm";
  //    "F:\\QC_data\\RI.1.3.46.423632.131000.1606838764.9.dcm";
  ////"C:\\RI.1.2.246.352.81.3.3417524247.25685.18122.148.193--500mu.dcm";

  // const std::string file_path_2 = path_name_2;

  //   /* "F:\\QC_data\\RI.1.3.46.423632.131000.1606839391.18.dcm";*/

  cv::Mat src_8_u_1, src_16_u_1;
  unsigned long rows1 = 0;
  unsigned long cols1 = 0;

  my_matrix(file_path_1, src_8_u_1, src_16_u_1, rows1, cols1);

  cv::Mat binary1, binary2;

  double t1 = threshold(src_8_u_1, binary1, 0, 255,
                        cv::THRESH_BINARY | cv::THRESH_OTSU);
 /* std::cout << "threshold of first picture: %.2f\n" << t1 << std::endl;*/

  /*cv::imshow("binary1", binary1);
  cv::imshow("binary2", binary2);*/

  int centerRowIndex1 = 512, centerColIndex1 = 512;

  std::vector<int> v_cols_index_1;

  std::vector<int> v_rows_index_1;
  // for this module I reckon that there is no need to capsule.
  cv::Mat dst1;
  cv::Canny(binary1, dst1, 0, 0, 3);

  find_center(dst1, rows1, cols1, centerRowIndex1, centerColIndex1,
              v_rows_index_1, v_cols_index_1);

  std::vector<double> centerlineGrayscale;
  src_16_u_1.row(centerColIndex1).copyTo(centerlineGrayscale);

  std::vector<double> centerColGrayscale;
  src_16_u_1.col(centerRowIndex1).copyTo(centerColGrayscale);

  auto max_element =
      *std::max_element(centerlineGrayscale.begin(), centerlineGrayscale.end());

  auto center_element = centerlineGrayscale[centerRowIndex1];

  // this sub-vector is what I used to calculate flatteness
  std::vector<int> sub_v_rows_scale;

  auto rangeMinElement = *std::min_element(
      centerlineGrayscale.begin() + *v_rows_index_1.begin(),
      centerlineGrayscale.begin() + *(v_rows_index_1.end() - 1));

  auto flatness = center_element / rangeMinElement;

  /*std::cout << "the flatness of this graph is :" << flatness * 100 << "%"
            << std::endl;*/

  auto symmetry = 0.0;
  double gap = 9;
  for (auto i = 1; i <= 10; i++) {
    symmetry = max(centerlineGrayscale[centerRowIndex1 + i * gap] /
                       centerlineGrayscale[centerRowIndex1 - i * gap],
                   symmetry);
  }

 /* std::cout << "the symmetry of this graph is: " << symmetry << std::endl;*/

  // this is the gray scale of FWHM.And I need to find the index of FWHM.
  auto half_element = 0.5 * (max_element + center_element);
  std::vector<int> fwhm_row;
  for (auto it = centerlineGrayscale.begin() + 20;
       it != centerlineGrayscale.end() - 21; ++it) {
    if (*it < half_element) {
      fwhm_row.emplace_back(it - centerlineGrayscale.begin());
    }
  }
  auto fwhm_rowcenterline_begin = *fwhm_row.begin();
  auto fwhm_rowcenterline_end = *(fwhm_row.end() - 1);

  // funny ,the 50% and the edge detect are the same
  //std::cout << "the index of  50% percent in row index are: "
  //          << fwhm_rowcenterline_begin << " " << fwhm_rowcenterline_end
  //          << std::endl;

  //std::cout << "the length of the edge in row index of x-ray determined by 50% "
  //             "dose is: "
  //          << (fwhm_rowcenterline_end - fwhm_rowcenterline_begin) / 1024.0 *
  //                 100.0
  //          << "%" << std::endl;

  std::vector<int> fwhm_col;
  for (auto it = centerColGrayscale.begin() + 20;
       it != centerColGrayscale.end() - 21; ++it) {
    if (*it < half_element) {
      fwhm_col.emplace_back(it - centerColGrayscale.begin());
    }
  }
  auto fwhm_colcenterline_begin = *fwhm_col.begin();
  auto fwhm_colcenterline_end = *(fwhm_col.end() - 1);

  // funny ,the 50% and the edge detect are the same
 /* std::cout << "the index of  50% percent in col index are: "
            << fwhm_colcenterline_begin << " " << fwhm_colcenterline_end
            << std::endl;

  std::cout << "the length of the edge in col index of x-ray determined by 50% "
               "dose is: "
            << (fwhm_colcenterline_end - fwhm_colcenterline_begin) / 1024.0 *
                   100.0
            << "%" << std::endl;*/

  auto FWHM = fwhm_colcenterline_end - fwhm_colcenterline_begin;

  // penumbra calculation
  auto eighty_element = 0.2 * max_element + 0.8 * center_element;
  auto twenty_element = 0.8 * max_element + 0.2 * center_element;

  // for now I am not going to use template
  std::vector<int> penumbra_row_left;
  std::vector<int> penumbra_row_right;
  for (auto it = centerlineGrayscale.begin() + 20;
       it != centerlineGrayscale.end() - 21; ++it) {
    if (*it<twenty_element&& * it> eighty_element && *it > *(it + 1)) {
      penumbra_row_left.emplace_back(it - centerlineGrayscale.begin());
    } else if ((*it<twenty_element&& * it> eighty_element && *it < *(it + 1))) {
      penumbra_row_right.emplace_back(it - centerlineGrayscale.begin());
    }
  }

  std::vector<int> penumbra_col_left;
  std::vector<int> penumbra_col_right;
  for (auto it = centerColGrayscale.begin() + 20;
       it != centerColGrayscale.end() - 21; ++it) {
    if (*it<twenty_element&& * it> eighty_element && *it > *(it + 1)) {
      penumbra_col_left.emplace_back(it - centerColGrayscale.begin());
    } else if ((*it<twenty_element&& * it> eighty_element && *it < *(it + 1))) {
      penumbra_col_right.emplace_back(it - centerColGrayscale.begin());
    }
  }
  std::vector<int> penumbra;
  penumbra.emplace_back(penumbra_col_left.size());
  penumbra.emplace_back(penumbra_col_right.size());
  penumbra.emplace_back(penumbra_row_left.size());
  penumbra.emplace_back(penumbra_row_right.size());
  auto penumbra_size = *std::max_element(penumbra.begin(), penumbra.end());

  // this have considered four components.
  //std::cout << "the penumbra of this filed is : " << penumbra_size;

  // find the eighty element position and do the flatness,this is from six
  // phase anyway I don't have to give a fuck right now
  std::vector<int> eighty_row_pos;

  for (auto it = centerlineGrayscale.begin() + 20;
       it != centerlineGrayscale.end() - 21; ++it) {
    if (*it < eighty_element) {
      eighty_row_pos.emplace_back(it - centerlineGrayscale.begin());
    }
  }
  auto eighty_row_begin = *eighty_row_pos.begin();
  auto eighty_row_end = *(eighty_row_pos.end() - 1);

  std::vector<double> eightyColGrayscaleleft;
  src_16_u_1.col(eighty_row_begin).copyTo(eightyColGrayscaleleft);

  auto eightyMinElementLeft = *std::min_element(
      eightyColGrayscaleleft.begin() + *v_cols_index_1.begin(),
      eightyColGrayscaleleft.begin() + *(v_cols_index_1.end() - 1));

  auto eight_index_element_left = centerlineGrayscale[eighty_row_begin];
  auto eighty_flatness_col_left =
      eight_index_element_left / eightyMinElementLeft;

  std::vector<double> eightyColGrayscaleright;
  src_16_u_1.col(eighty_row_end).copyTo(eightyColGrayscaleright);

  auto eightyMinElementRight = *std::min_element(
      eightyColGrayscaleright.begin() + *v_cols_index_1.begin(),
      eightyColGrayscaleright.begin() + *(v_cols_index_1.end() - 1));

  auto eight_index_element_right = centerlineGrayscale[eighty_row_end];
  auto eighty_flatness_col_right =
      eight_index_element_right / eightyMinElementRight;

  if (x == e_penumbra) return penumbra_size;
  if (x == e_symmetry) return symmetry;
  if (x == e_flatness) return flatness;
  if (x == e_FWHM) return FWHM;
}

template <int x>
double functionexample::process_two_images(const std::string& path_name_1,
                                           const std::string& path_name_2) {
  const std::string file_path_1 = path_name_1;
  const std::string file_path_2 = path_name_2;

  cv::Mat src_8_u_1, src_16_u_1, src_8_u_2, src_16_u_2;
  unsigned long rows1 = 0;
  unsigned long cols1 = 0;
  unsigned long rows2 = 0;
  unsigned long cols2 = 0;

  my_matrix(file_path_1, src_8_u_1, src_16_u_1, rows1, cols1);
  my_matrix(file_path_2, src_8_u_2, src_16_u_2, rows2, cols2);

  /* cv::imshow("src_8_u1", src_8_u_1);
   cv::imshow("src_16_u_1", src_16_u_1);*/

  cv::Mat binary1, binary2;

  double t1 = threshold(src_8_u_1, binary1, 0, 255,
                        cv::THRESH_BINARY | cv::THRESH_OTSU);
  /*std::cout << "threshold of first picture: %.2f\n" << t1 << std::endl;*/

  double t2 = threshold(src_8_u_2, binary2, 0, 255,
                        cv::THRESH_BINARY | cv::THRESH_OTSU);
  /*std::cout << "threshold of first picture: %.2f\n" << t2 << std::endl;*/

  /*cv::imshow("binary1", binary1);
  cv::imshow("binary2", binary2);*/

  // for this module I reckon that there is no need to capsule.
  cv::Mat dst1, dst2;
  cv::Canny(binary1, dst1, 0, 0, 3);
  cv::Canny(binary2, dst2, 0, 0, 3);
  // cv::imshow("Canny", dst);
  std::vector<std::vector<cv::Point>> contours1;
  std::vector<std::vector<cv::Point>> contours2;
  findContours(dst1.clone(), contours1, cv::RETR_EXTERNAL,
               cv::CHAIN_APPROX_SIMPLE);
  findContours(dst1.clone(), contours2, cv::RETR_EXTERNAL,
               cv::CHAIN_APPROX_SIMPLE);
  if (contours1.empty() || contours2.empty()) {
    return -1;
  }

  std::vector<cv::Vec2f> lines1, lines2;
  HoughLines(dst1, lines1, 1, CV_PI / 180, 150, 0, 0);
  HoughLines(dst2, lines2, 1, CV_PI / 180, 150, 0, 0);

  auto theta = lines1[1][1];
  auto theta2 = lines2[1][1];
  double Rad_to_deg = 45.0 / atan(1.0);
  double fAngle1 = theta * Rad_to_deg;
  double fAngle2 = theta2 * Rad_to_deg;

  double fAngle_difference = fAngle1 - fAngle2;

  //std::cout << "the difference of the angle is : " << fAngle_difference
  //          << std::endl;

  int centerRowIndex1 = 512, centerColIndex1 = 512;
  int centerRowIndex2 = 512, centerColIndex2 = 512;
  std::vector<int> v_cols_index_1;
  std::vector<int> v_cols_index_2;
  std::vector<int> v_rows_index_1;
  std::vector<int> v_rows_index_2;
  find_center(dst1, rows1, cols1, centerRowIndex1, centerColIndex1,
              v_rows_index_1, v_cols_index_1);
  find_center(dst2, rows2, cols2, centerRowIndex2, centerColIndex2,
              v_rows_index_2, v_cols_index_2);

  /* std::cout << centerColIndex1 << " " << centerRowIndex1 << " "
             << centerColIndex2 << " " << centerRowIndex2 << std::endl;*/
  double center_distance = sqrt(fabs(centerColIndex1 - centerColIndex2) *
                                    fabs(centerColIndex1 - centerColIndex2) +
                                fabs(centerRowIndex1 - centerRowIndex2) *
                                    fabs(centerRowIndex1 - centerRowIndex2));

 /* std::cout << "the difference between the two center is :" << center_distance
            << std::endl;*/

  if (x == e_center_distance) return center_distance;
  if (x == e_angle) return fAngle_difference;
}

Napi::Number functionexample::SymmetryWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();

  double returnValue =
      functionexample::process_images<e_symmetry>(first.ToString());

  return Napi::Number::New(env, returnValue);
}
Napi::Number functionexample::FlatnessWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();

  double returnValue =
      functionexample::process_images<e_flatness>(first.ToString());

  return Napi::Number::New(env, returnValue);
}

Napi::Number functionexample::PenumbraWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();

  double returnValue =
      functionexample::process_images<e_penumbra>(first.ToString());

  return Napi::Number::New(env, returnValue);
}

Napi::Number functionexample::FWHMWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();

  double returnValue =
      functionexample::process_images<e_FWHM>(first.ToString());

  return Napi::Number::New(env, returnValue);
}

Napi::Number functionexample::AngleWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();
  Napi::String second = info[1].As<Napi::String>();
  double returnValue = functionexample::process_two_images<e_angle>(
      first.ToString(), second.ToString());

  return Napi::Number::New(env, returnValue);
}
Napi::Number functionexample::CenterDistanceWrapped(
    const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
    Napi::TypeError::New(env, "Path as String expected")
        .ThrowAsJavaScriptException();
  }
  Napi::String first = info[0].As<Napi::String>();
  Napi::String second = info[1].As<Napi::String>();
  double returnValue = functionexample::process_two_images<e_center_distance>(
      first.ToString(), second.ToString());

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

  exports.Set("get_penumbra",
              Napi::Function::New(env, functionexample::PenumbraWrapped));
  exports.Set("get_fwhm",
              Napi::Function::New(env, functionexample::FWHMWrapped));
  exports.Set("get_centerDistance",
              Napi::Function::New(env, functionexample::CenterDistanceWrapped));
  exports.Set("get_angle",
              Napi::Function::New(env, functionexample::AngleWrapped));

  return exports;
}
