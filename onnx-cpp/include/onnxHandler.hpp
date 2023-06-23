#pragma 1

#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <onnxruntime_cxx_api.h>

// namespace Ort {
//     class Session;
//     class Env;
//     class SessionOptions;
// }

struct Prediction
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

class OnnxHandler
{
public:
    OnnxHandler(const std::string& p_model_path);

    bool predict(const std::string& p_imageFilepath, std::vector<Prediction>& p_results);

    static bool GetResponseString(std::vector<Prediction>& p_predictions, std::string& p_output);
private:
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    Ort::Session* session;
};
