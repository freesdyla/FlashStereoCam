#ifndef FLIR_BLACKFLYS_CAMERA_CONTROL_H
#define FLIR_BLACKFLYS_CAMERA_CONTROL_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include "jetsonGPIO.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;
using namespace std::chrono;
using namespace cv;

class FlirBlackFlySCameraControl {

public:

    typedef high_resolution_clock Time;
	typedef high_resolution_clock::time_point TP;
    typedef duration<double> SecD;
	
	SystemPtr system_;

	CameraList camList_;

	std::vector<std::thread> grabThreadVec_;

	std::vector<std::thread> triggerThreadVec_;

	std::atomic<bool> streaming_;	

	std::mutex update_mutex_;

	std::vector<CameraPtr> pCamVec_;

	std::vector<gcstring> SNVec_;

	std::vector<Mat> cvImgVec_;

	FlirBlackFlySCameraControl();
	~FlirBlackFlySCameraControl();

	int Start();
	int Stop();
	int ConfigureTrigger(INodeMap& nodeMap);
	void GrabFrame(CameraPtr pCam, unsigned int cameraID);
	void StreamByTrigger();
	int setExposureTime(double duration_ms);

};

#endif
