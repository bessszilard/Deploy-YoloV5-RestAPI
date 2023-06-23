cmake_minimum_required(VERSION 3.20)

project(ONNX_Runtime_Depth_Estimation_EXAMPLE)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_executable(main main.cpp)
target_include_directories(main PRIVATE ${ONNXRUNTIME_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(main PRIVATE ${ONNXRUNTIME_LIB} ${OpenCV_LIBRARIES})
