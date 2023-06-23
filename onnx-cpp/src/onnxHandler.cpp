#include "../include/onnxHandler.hpp" // TODOsz fix this in the make file
#include "opencv2/opencv.hpp"
#include <fstream>
#include <numeric>

/**
 * @brief Define names based depends on Unicode path support
 */
#define tcout                  std::cout
#define file_name_t            std::string
#define imread_t               cv::imread
#define NMS_THRESH 0.5
#define BBOX_CONF_THRESH 0.7
// TODOsz ^^ remove these later

static const int INPUT_W = 416;
static const int INPUT_H = 416;
static const int NUM_CLASSES = 80; // COCO has 80 classes. Modify this value on your own dataset.


struct GridAndStride
{
    int grid0;
    int grid1;
    int stride;
};

template <typename T>
T vectorProduct(const std::vector<T>& v)
{
    return accumulate(v.begin(), v.end(), 1, std::multiplies<T>());
}



cv::Mat static_resize(cv::Mat& img) {
    float r = std::min(INPUT_W / (img.cols*1.0), INPUT_H / (img.rows*1.0));
    // r = std::min(r, 1.0f);
    int unpad_w = r * img.cols;
    int unpad_h = r * img.rows;
    cv::Mat re(unpad_h, unpad_w, CV_8UC3);
    cv::resize(img, re, re.size());
    //cv::Mat out(INPUT_W, INPUT_H, CV_8UC3, cv::Scalar(114, 114, 114));
    cv::Mat out(INPUT_H, INPUT_W, CV_8UC3, cv::Scalar(114, 114, 114));
    re.copyTo(out(cv::Rect(0, 0, re.cols, re.rows)));
    return out;
}

std::vector<std::string> readLabels(std::string& labelFilepath)
{
    std::vector<std::string> labels;
    std::string line;
    std::ifstream fp(labelFilepath);
    while (std::getline(fp, line))
    {
        labels.push_back(line);
    }
    return labels;
}

static void generate_grids_and_stride(const int target_w, const int target_h, std::vector<int>& strides, std::vector<GridAndStride>& grid_strides)
{
    for (auto stride : strides)
    {
        int num_grid_w = target_w / stride;
        int num_grid_h = target_h / stride;
        for (int g1 = 0; g1 < num_grid_h; g1++)
        {
            for (int g0 = 0; g0 < num_grid_w; g0++)
            {
                grid_strides.push_back((GridAndStride){g0, g1, stride});
            }
        }
    }
}


static void generate_yolox_proposals(std::vector<GridAndStride> grid_strides, const float* feat_ptr, float prob_threshold, std::vector<Prediction>& predictions)
{

    const int num_anchors = grid_strides.size(); std::cout <<"size grid_strides: " << grid_strides.size()<< std::endl;

    for (int anchor_idx = 0; anchor_idx < num_anchors; anchor_idx++)
    {
        const int grid0 = grid_strides[anchor_idx].grid0;
        const int grid1 = grid_strides[anchor_idx].grid1;
        const int stride = grid_strides[anchor_idx].stride;

        /*std::cout << "grid 0: "<< grid0<< std::endl;
        std::cout << "grid 1: "<< grid1<< std::endl;
        std::cout << "stride: "<< stride<< std::endl;*/

    const int basic_pos = anchor_idx * (NUM_CLASSES + 5);


        float x_center = (feat_ptr[basic_pos + 0] + grid0) * stride;
        float y_center = (feat_ptr[basic_pos + 1] + grid1) * stride;
        float w = exp(feat_ptr[basic_pos + 2]) * stride;
        float h = exp(feat_ptr[basic_pos + 3]) * stride;

        

        float x0 = x_center - w * 0.5f;
        float y0 = y_center - h * 0.5f;


        float box_objectness = feat_ptr[basic_pos + 4];

        for (int class_idx = 0; class_idx < NUM_CLASSES; class_idx++)
        {
            //float box_cls_score = std::exp(feat_ptr[basic_pos + class_idx]) / total;
             float box_cls_score = feat_ptr[basic_pos + 5 + class_idx];
            //std::cout << "Probability " << class_idx<< ": "<< box_cls_score<< std::endl;
            float box_prob = box_objectness * box_cls_score; ///std::cout <<"box_prob: " << box_prob<< std::endl;
            if (box_prob > prob_threshold)
            {
                Prediction obj;
                obj.rect.x = x0;
                obj.rect.y = y0;
                obj.rect.width = w;
                obj.rect.height = h;
                obj.label = class_idx;
                obj.prob = box_prob;

                predictions.push_back(obj);
            }

        } // class loop

    } // point anchor loop
}

static inline float intersection_area(const Prediction& a, const Prediction& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<Prediction>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
        #pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}


static void qsort_descent_inplace(std::vector<Prediction>& predictions)
{
    if (predictions.empty())
        return;

    qsort_descent_inplace(predictions, 0, predictions.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Prediction>& faceobjects, std::vector<int>& picked, float nms_threshold)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++)
    {
        const Prediction& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const Prediction& b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}



static void decode_outputs(const float* prob, std::vector<Prediction>& predictions, float scale, const int img_w, const int img_h) {
        std::vector<Prediction> proposals;
        std::vector<int> strides = {8, 16, 32};
        std::vector<GridAndStride> grid_strides;

        generate_grids_and_stride(INPUT_W, INPUT_H, strides, grid_strides);
        generate_yolox_proposals(grid_strides, prob,  BBOX_CONF_THRESH, proposals);
        qsort_descent_inplace(proposals);

        std::vector<int> picked;
        nms_sorted_bboxes(proposals, picked, NMS_THRESH);
        int count = picked.size();
        predictions.resize(count);

        for (int i = 0; i < count; i++)
        {
            predictions[i] = proposals[picked[i]];

            // adjust offset to original unpadded
            float x0 = (predictions[i].rect.x) / scale;
            float y0 = (predictions[i].rect.y) / scale;
            float x1 = (predictions[i].rect.x + predictions[i].rect.width) / scale;
            float y1 = (predictions[i].rect.y + predictions[i].rect.height) / scale;

            // clip
            x0 = std::max(std::min(x0, (float)(img_w - 1)), 0.f);
            y0 = std::max(std::min(y0, (float)(img_h - 1)), 0.f);
            x1 = std::max(std::min(x1, (float)(img_w - 1)), 0.f);
            y1 = std::max(std::min(y1, (float)(img_h - 1)), 0.f);

            predictions[i].rect.x = x0;
            predictions[i].rect.y = y0;
            predictions[i].rect.width = x1 - x0;
            predictions[i].rect.height = y1 - y0;
        }
}

OnnxHandler::OnnxHandler(const std::string& p_modelPath)
{
    std::string instanceName{"image-classification-inference"};
    // std::string modelFilepath{"../best_yolox_nano.onnx"};
    // std::string modelFilepath{"../yolov5s.onnx"};
    // std::string imageFilepath{"image.jpg"};
    std::string labelFilepath{"../synset.txt"};

    std::vector<std::string> labels{readLabels(labelFilepath)};

    env = Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, instanceName.c_str());
    // static Ort::SessionOptions sessionOptions;
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    session = new Ort::Session(env, p_modelPath.c_str(), sessionOptions);
}

/**
 * @brief Operator overloading for printing vectors
 * @tparam T
 * @param os
 * @param v
 * @return std::ostream&
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    os << "[";
    for (int i = 0; i < v.size(); ++i)
    {
        os << v[i];
        if (i != v.size() - 1)
        {
            os << ", ";
        }
    }
    os << "]";
    return os;
}


bool OnnxHandler::predict(const std::string& p_imageFilepath, std::vector<Prediction>& p_results)
{
    Ort::AllocatorWithDefaultOptions allocator;
    size_t numInputNodes = session->GetInputCount();
    size_t numOutputNodes = session->GetOutputCount();

    Ort::AllocatedStringPtr inputNameUPtr = session->GetInputNameAllocated(0, allocator);
    const char* inputName = inputNameUPtr.get();
    //std::cout << "Input Name: " << inputName << std::endl;

    Ort::TypeInfo inputTypeInfo = session->GetInputTypeInfo(0);
    auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();

    ONNXTensorElementDataType inputType = inputTensorInfo.GetElementType();
    //std::cout << "Input Type: " << inputType << std::endl;

    std::vector<int64_t> inputDims = inputTensorInfo.GetShape();
    //std::cout << "Input Dimensions: " << inputDims << std::endl;

    Ort::AllocatedStringPtr outputNameUPtr = session->GetOutputNameAllocated(0, allocator);
    const char* outputName = outputNameUPtr.get();
    //std::cout << "Output Name: " << outputName << std::endl;

    Ort::TypeInfo outputTypeInfo = session->GetOutputTypeInfo(0);
    auto outputTensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();

    ONNXTensorElementDataType outputType = outputTensorInfo.GetElementType();
    //std::cout << "Output Type: " << outputType << std::endl;

    std::vector<int64_t> outputDims = outputTensorInfo.GetShape();
    //std::cout << "Output Dimensions: " << outputDims << std::endl;

    cv::Mat imageBGR = cv::imread(p_imageFilepath, cv::ImreadModes::IMREAD_COLOR);
    cv::Mat preprocessedImage;

    cv::Mat resizedImage = static_resize(imageBGR);
    cv::dnn::blobFromImage(resizedImage, preprocessedImage);

    size_t inputTensorSize = vectorProduct(inputDims);
    std::vector<float> inputTensorValues(inputTensorSize);
    inputTensorValues.assign(preprocessedImage.begin<float>(),
                            preprocessedImage.end<float>()); 

    size_t outputTensorSize = vectorProduct(outputDims);
    std::vector<float> outputTensorValues(outputTensorSize);

    std::vector<const char*> inputNames{inputName};
    std::vector<const char*> outputNames{outputName};
    std::vector<Ort::Value> inputTensors;
    std::vector<Ort::Value> outputTensors;

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    inputTensors.push_back(Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensorValues.data(), inputTensorSize, inputDims.data(),
        inputDims.size()));
    outputTensors.push_back(Ort::Value::CreateTensor<float>(
        memoryInfo, outputTensorValues.data(), outputTensorSize,
        outputDims.data(), outputDims.size()));

    session->Run(Ort::RunOptions{nullptr}, inputNames.data(), inputTensors.data(), 
                1, outputNames.data(), outputTensors.data(), 1);

    cv::Mat image = imread_t(p_imageFilepath);
    int img_w = image.cols;
    int img_h = image.rows;
    float scale = std::min(INPUT_W / (image.cols*1.0), INPUT_H / (image.rows*1.0));
    // std::vector<Prediction> predictions;

    const float * net_pred = outputTensorValues.data();
    decode_outputs(net_pred, p_results, scale, img_w, img_h);

    return true;
}

bool OnnxHandler::GetResponseString(std::vector<Prediction>& p_predictions, std::string& p_output)
{
    if (p_predictions.size() == 0)
    {
        std::cout << "Invalid prediction list" << std::endl;
        return false;
    }
    p_output = "[";
    for (int i = 0; i < p_predictions.size(); i++)
    {
        const auto & pred = p_predictions[i];
        p_output += "{\"xmin\":" + std::to_string(pred.rect.x) + ",\"ymin\":" + std::to_string(pred.rect.y) + \
                    ",\"xmax\":" + std::to_string(pred.rect.x + pred.rect.width) +  \
                    ",\"ymax\":" + std::to_string(pred.rect.y + pred.rect.height) + \
                    ",\"confidence\":" + std::to_string(pred.prob) + \
                    ",\"class\":" + std::to_string(pred.label) + \
                    ",\"name\":" + "\"alma\"" + "}";
        if (i < p_predictions.size() - 1)
            p_output += ",";
    }
    p_output += "]";
    return true;
}