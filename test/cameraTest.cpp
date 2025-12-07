#include "camera_manager.h"
#include "device_manager.h"
#include "logger.h"
#include "ImageProcessor.h"
#include <string>
#include <iostream>
int main()
{
    DeviceManager device;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    RealImage image =  device.getRealImage("1");
    image_buffer_t out_image;
    std::vector<unsigned char> outJpeg;
    if(image.integrity)
    {
        ImageProcessor::avframeToRGB(image.frame.frame.get(),640,640,&out_image);
        ImageProcessor::compressToJpeg(&out_image,outJpeg);
        // std::string imageBase64 = ImageProcessor::jpegToBase64(outJpeg);
    }
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}