#ifndef CAMERA_INFO_H
#define CAMERA_INFO_H
#include<string>
class CameraInfo{
public:
    CameraInfo(const std::string& cameraName,const std::string& cameraUrl)
        :camera_name(cameraName),camera_url(cameraUrl){}
    std::string camera_name;
    std::string camera_url;    
};

#endif