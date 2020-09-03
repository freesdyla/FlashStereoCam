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



	typedef vector<pair<Mat,string>> ImageNamePairVec;

	vector<ImageNamePairVec> inpvVec_;
	
	SystemPtr system_;

	CameraList camList_;

	std::vector<thread> grabThreadVec_;

	vector<thread> saveThreadVec_;

	vector<thread> triggerThreadVec_;

	atomic<bool> streaming_;	

	vector<CameraPtr> pCamVec_;

	mutex saveMutex_;

	std::vector<gcstring> SNVec_;

	std::vector<Mat> cvImgVec_;

	FlirBlackFlySCameraControl();
	~FlirBlackFlySCameraControl();

	int Start();
	int Stop();
	int ConfigureTrigger(INodeMap& nodeMap);
	void GrabFrame(CameraPtr pCam, unsigned int cameraID/*, Mat *concatImg*/);
	void StreamByTrigger();
	int setExposureTime(double duration_ms);
	void SaveImage();

};

#endif
