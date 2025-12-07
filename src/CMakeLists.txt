cmake_minimum_required(VERSION 3.15)
project(src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MQTT_LIBS
    paho-mqttpp3  # C++ 库（libpaho-mqttpp3.so）
    paho-mqtt3as  # C 库（带 SSL 异步，libpaho-mqtt3as.so）
)



# -------------------------- 1. 收集src目录源文件 --------------------------
set(SRC_FILES
    ../test/cameraTest.cpp
    business_layer/camera/camera_manager.cpp
    business_layer/camera/camera.cpp
    business_layer/device/device_manager.cpp
    business_layer/task/task.cpp
    business_layer/task/job_scheduler.cpp
    business_layer/mqtt/mqtt_service.cpp
    business_layer/mqtt/mqtt_command_dispatcher.cpp
    data_layer/video_capture.cpp
    data_layer/video_storage.cpp
    data_layer/sensor_reader.cpp
    ../include/common/FileUtils/FileUtils.cpp
    ../include/common/utils/yolo8/postprocess.cc
    ../include/common/utils/yolo8/yolov8.cc
    ../include/common/Imageprocess/ImageProcessor.cpp
    ../include/common/timestamp/SegmentManager.cpp
    ../include/common/logger/logger.cpp
    ../include/common/base64/base64.cpp
)

# -------------------------- 2. 生成主程序可执行文件 --------------------------
add_executable(main ${SRC_FILES})  # 可执行文件名：ai_recognition

# -------------------------- 3. 配置头文件路径 --------------------------
target_include_directories(main PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/data_layer
    ${CMAKE_SOURCE_DIR}/include/data_layer/camera
    ${CMAKE_SOURCE_DIR}/include/business_layer
    ${CMAKE_SOURCE_DIR}/include/business_layer/camera
    ${CMAKE_SOURCE_DIR}/include/business_layer/device
    ${CMAKE_SOURCE_DIR}/include/business_layer/task
    ${CMAKE_SOURCE_DIR}/include/business_layer/mqtt
    ${CMAKE_SOURCE_DIR}/include/business_layer/sensor_service
    ${CMAKE_SOURCE_DIR}/include/common
    ${CMAKE_SOURCE_DIR}/include/common/FileUtils
    ${CMAKE_SOURCE_DIR}/include/common/utils/yolo8
    ${CMAKE_SOURCE_DIR}/include/common/Imageprocess
    ${CMAKE_SOURCE_DIR}/include/common/logger
    ${CMAKE_SOURCE_DIR}/include/common/timestamp
    ${CMAKE_SOURCE_DIR}/include/common/threads
    ${CMAKE_SOURCE_DIR}/include/common/base64
    ${CMAKE_SOURCE_DIR}/third_party/mqtt
    ${CMAKE_SOURCE_DIR}/third_party/json
    
    ${STB_INCLUDES}
    ${LIBJPEG_INCLUDES}
    ${LIBRKNNRT_INCLUDES}
    ${LIBRGA_INCLUDES}
    ${LIBSNDFILE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/include/common/rknn/utils
    ${OpenCV_INCLUDE_DIRS}
)
# link_directories(
#     ${PROJECT_SOURCE_DIR}/lib  # 告诉编译器从项目 lib 目录找库
# )

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
# -------------------------- 4. 链接依赖库 --------------------------
target_link_libraries(main PUBLIC
    fileutils
    imagedrawing
    imageutils
    audioutils
    ${LIBJPEG}
    ${LIBRKNNRT}
    ${LIBRGA}
    ${LIBSNDFILE}
    ${LIBFFTW}
    ${OpenCV_LIBS}
    # FFmpeg 库
    avutil
    swscale
    avcodec
    avformat
    
    #mqtt库
    paho-mqttpp3
    paho-mqtt3as

    OpenSSL::Crypto
    Threads::Threads


    # 系统库
    dl
    pthread
    m
)

# set_target_properties(main PROPERTIES
#     INSTALL_RPATH "${PROJECT_SOURCE_DIR}/lib"
#     BUILD_WITH_INSTALL_RPATH ON
# )
# -------------------------- 5. 可选：安装主程序 --------------------------
install(TARGETS main DESTINATION bin)  # 安装到install/bin目录