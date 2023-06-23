#include <string>
#include <vector>

#include "crow.h"
#include "onnxHandler.hpp"

void WriteToFile(std::string const& fileName, std::string const& data)
{
    std::ofstream binFile(fileName, std::ios::out | std::ios::binary);
    if (binFile.is_open())
    {
        size_t len = data.size();
        binFile.write((char*)&data[0], len);
        // No need. The file will be closed when the function returns.
        // binFile.close();
    }
}


int main(int argc, char* argv[])
{
    bool useCUDA{false};
    const char* useCUDAFlag = "--use_cuda";
    const char* useCPUFlag = "--use_cpu";
    if (argc == 1)
    {
        useCUDA = false;
    }
    else if ((argc == 2) && (strcmp(argv[1], useCUDAFlag) == 0))
    {
        useCUDA = true;
    }
    else if ((argc == 2) && (strcmp(argv[1], useCPUFlag) == 0))
    {
        useCUDA = false;
    }
    else if ((argc == 2) && (strcmp(argv[1], useCUDAFlag) != 0))
    {
        useCUDA = false;
    }
    else
    {
        throw std::runtime_error{"Too many arguments."};
    }

    if (useCUDA)
    {
        std::cout << "Inference Execution Provider: CUDA" << std::endl;
    }
    else
    {
        std::cout << "Inference Execution Provider: CPU" << std::endl;
    }

    crow::SimpleApp app;
    OnnxHandler onnxHandler("../best_yolox_nano.onnx");

    CROW_ROUTE(app, "/v1/object-detection/yolov5").methods(crow::HTTPMethod::POST)
    ([&onnxHandler](const crow::request& req) {
        crow::multipart::message msg(req);

        // TODOsz check parts size -> error if less than 1

        CROW_LOG_INFO << "Received image size:" << msg.parts[0].body.size();

        WriteToFile("image.jpg", msg.parts[0].body);
        
        std::vector<Prediction> predictions;
        onnxHandler.predict("image.jpg", predictions);

        std::string response;
        if (false == OnnxHandler::GetResponseString(predictions, response))
        {
            CROW_LOG_ERROR <<  "Failed to get response string";
            return crow::response(500, "Failed to get response string");
        }
        return crow::response(200, response);
    });

    app.port(3000).multithreaded().run();
}

