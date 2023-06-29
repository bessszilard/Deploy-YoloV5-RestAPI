#include <string>
#include <vector>

#include "crow.h"
#include "cmdline.h"
#include "utils.hpp"
#include "detector.hpp"
#include <string>

// static const std::string ModelPath = "../best_yolox_nano.onnx";
// static const std::string ModelPath = "../yolov5s6.onnx";

int main(int argc, char *argv[])
{
    // TODOsz remove this
    const float confThreshold = 0.3f;
    const float iouThreshold = 0.4f;

    cmdline::parser cmd;
    cmd.add<std::string>("model_path", 'm', "Path to onnx model.", true, "../yolov5s6.onnx");
    cmd.add<std::string>("class_names", 'c', "Path to class names file.", true, "coco.names");
    cmd.add<std::string>("port", 'p', "Rest API port", true, "3000");
    cmd.add("gpu", '\0', "Inference on cuda device.");
    cmd.parse_check(argc, argv);

    crow::SimpleApp app;

    bool isGPU = cmd.exist("gpu");
    const std::string classNamesPath = cmd.get<std::string>("class_names");
    const std::vector<std::string> classNames = utils::loadNames(classNamesPath);
    const std::string modelPath = cmd.get<std::string>("model_path");

    if (classNames.empty())
    {
        std::cerr << "Error: Empty class names file." << std::endl;
        return -1;
    }

    YOLODetector detector = YOLODetector(modelPath, isGPU, cv::Size(640, 640));

    CROW_ROUTE(app, "/v1/object-detection/yolov5")
        .methods(crow::HTTPMethod::POST)([&detector, &confThreshold, &iouThreshold, &classNames](const crow::request &req)
                                         {
        crow::multipart::message msg(req);

        // TODOsz check parts size -> error if less than 1
        CROW_LOG_INFO << "Received image size:" << msg.parts[0].body.size();
        utils::writeToFile("image.jpg", msg.parts[0].body);
        auto image = cv::imread("image.jpg");
        std::vector<Detection> result = detector.detect(image, confThreshold, iouThreshold);

        std::string response;
        if (false == utils::getResponseString(result, response, classNames))
        {
            CROW_LOG_ERROR <<  "Failed to get response string";
            return crow::response(500, "Failed to get response string");
        }
        return crow::response(200, response); });

    const std::string &prortStr = cmd.get<std::string>("port");
    app.port(std::stoi(prortStr)).multithreaded().run();
}
