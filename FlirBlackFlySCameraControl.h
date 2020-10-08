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
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;
using namespace std::chrono;
using namespace cv;
using namespace cv::cuda;

class FlirBlackFlySCameraControl {

public:

    typedef high_resolution_clock Time;
	typedef high_resolution_clock::time_point TP;
    typedef duration<double> SecD;
	typedef vector<pair<Mat,string>> ImageNamePairVec;

	vector<ImageNamePairVec> inpvVec_;
	
	SystemPtr system_;

	CameraList camList_;

	vector<thread> grabThreadVec_;

	vector<thread> saveThreadVec_;

	vector<thread> triggerThreadVec_;

	atomic<bool> streaming_, saving_;	

	vector<CameraPtr> pCamVec_;

	mutex saveMutex_;

	vector<gcstring> SNVec_;

	vector<Mat> cvImgVec_;

	GpuMat gpuImg0, gpuImg1;

	double gain_ = 1.;
	double exposure_ = 700.;

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
