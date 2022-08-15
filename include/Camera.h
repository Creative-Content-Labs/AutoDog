#ifndef CAMERA_H
#define CAMERA_H

#include "common_header.h"

class Camera {
  public:
    // constructor 
    Camera(std::string camera_path, double focal_length, cv::Point2d principar_point);

    std::string cameraPath() { return camera_path_; }
    double focalLength() { return focal_length_; }
    cv::Point2d principalPoints() { return principal_point_; }

  private:
    double focal_length_;
    cv::Point2d principal_point_;
    cv::Point2d camera_current_location_;

    cv::Mat camera_pose_;
    std::string camera_path_;
  
};

#endif