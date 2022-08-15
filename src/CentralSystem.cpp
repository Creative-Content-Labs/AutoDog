#include "CentralSystem.h"
using namespace rp::standalone::rplidar;

CentralSystem::CentralSystem(bool mode, bool use_lidar) {
    // Camera init
    printf("[SYSTEM]: Initialization Camera....");
    cv::Point2d center_point = cv::Point2d(601.8873, 183.1104);
    if ( mode ) { // Real Mode 
        camera_ = std::make_shared<Camera>(CAMERA_PATH, FOCAL_LENGTH, center_point);
    }
    else { // simulation mode 
        camera_ = std::make_shared<Camera>(SIMULATION_DATA_PATH, FOCAL_LENGTH, center_point);
    }
    printf("\t[OK]\n");

    use_lidar_ = use_lidar;
    // LiDAR init 
    if (use_lidar_){
        printf("[SYSTEM]: Initialization LiDAR....");
        lidar_ = std::make_shared<Lidar>(PORT, BAUDRATE);
        printf("\t[OK]\n");
    }

    // DogStatus init 
    printf("[SYSTEM]: Initialization Dog Status....");
    dog_status_ = std::make_shared<DogStatus>();
    printf("\t[OK]\n");

    // visual odometry init
    printf("[SYSTEM]: Initialization Visual Odometry....");
    vo_ = std::make_shared<VisualOdometry>(camera_);
    printf("\t[OK]\n");
    system_status_ = true;

    this->printfSystemInformation(mode);
    dog_status_->setSystemStatus(true);
}

void CentralSystem::printfSystemInformation(bool mode) {
    printf("==========================System Information===========================\n");
    printf("AUTO DOG SYSTEM v1.0.0\n");
    printf("System Mode = %s\n", mode ? "Real Mode" : "Simulation Mode" );
    if ( mode )
        printf("Camera PATH = %s\n", CAMERA_PATH);
    else 
        printf("Camera PATH = %s\n", SIMULATION_DATA_PATH);

    if (use_lidar_) {
        printf("LiDAR PATH  = %s\n", PORT);
        printf("Boud Rate   = %d\n", BAUDRATE);
    }
    printf("System Status = %s\n", system_status_ ? "OK!" : "NO!");
    printf("Auto System Start...\n");
    printf("=======================================================================\n\n");
    printf("If you want exit system, Press ESC key\n");
}

void CentralSystem::cameraCaptureThread(std::shared_ptr<Camera> camera, std::shared_ptr<DogStatus> dog_status) {
    cv::VideoCapture cap(camera->cameraPath());
    if (!cap.isOpened()) {
        printf("[ERROR] Check Your Video or Camrea path!!!\n");
        return;
    }

    cv::Mat current_frame;
    while(1) {
        // read current_frame 
        cap >> current_frame;
        if (!dog_status->getSystemStatus()) {
            break;
        }

        if (current_frame.empty()) {
            continue;
        }

        dog_status->setCurrentFrame(current_frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void CentralSystem::computeTrajectoryThread(std::shared_ptr<Camera> camera, std::shared_ptr<VisualOdometry> vo, std::shared_ptr<DogStatus> dog_status) {
    cv::Mat current_frame;

    while(1) {
        // read current_frame 
        if (!dog_status->getSystemStatus()) {
            break;
        }

        current_frame = dog_status->getCurrentFrame();
        if (current_frame.empty()) {
            continue;
        }

        vo->addFrame(current_frame); 
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void CentralSystem::scanLidarThread(std::shared_ptr<Lidar> lidar, std::shared_ptr<DogStatus> dog_status) {
    while(1) {
        if (!dog_status->getSystemStatus()) {
            break;
        }

        sl_lidar_response_measurement_node_hq_t nodes[8192];
        size_t count = _countof(nodes);
        lidar->grabScanedLidarData(nodes, count);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void CentralSystem::controlMotorThread() {

 }

void CentralSystem::startProgram() {
    camera_capture_thread_ = std::thread(&CentralSystem::cameraCaptureThread, camera_, dog_status_);
    compute_traj_thread_ = std::thread(&CentralSystem::computeTrajectoryThread, camera_, vo_, dog_status_);
    if(use_lidar_)
        scan_lidar_thread_ = std::thread(&CentralSystem::scanLidarThread, lidar_, dog_status_);

    cv::Mat current_display;
    cv::Mat traj_display = cv::Mat::zeros(cv::Size(1000, 1000), CV_8UC3);

    while (1) {
        if (!dog_status_->getSystemStatus()) {
            break;
        }

        current_display = dog_status_->getCurrentFrame();

        cv::Point2d current_location = vo_->getCurrentLocation();
        int curr_loc_x = int(current_location.x) + 500;
        int curr_loc_y = int(current_location.y) + 500;

        cv::drawMarker(traj_display, cv::Point(curr_loc_x, curr_loc_y), cv::Scalar(0, 0, 255), cv::MARKER_SQUARE, 5, 2);
        // traj_display = dog_status_->getTrajData();

        if (current_display.empty()) 
            continue;

        cv::imshow("display", current_display);
        cv::imshow("Tracjectory", traj_display);
        int key = cv::waitKey(27);
        if (key == 27) {
            dog_status_->setSystemStatus(false);
            break;
        }
    }
    cv::destroyAllWindows();

    
    camera_capture_thread_.join();
    compute_traj_thread_.join();
    
    if(use_lidar_)
        scan_lidar_thread_.join();
    
    printf("[SYSTEM]: Exit system!\n");
}