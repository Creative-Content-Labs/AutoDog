#include "Camera.h"

Camera::Camera(std::string camera_path, double focal_length, cv::Point2d principal_point) {
    // init
    camera_current_location_ = cv::Point2d(0, 0);
    camera_path_ = camera_path;

    // calibration data
    principal_point_ = principal_point;
    focal_length_ = focal_length;
}