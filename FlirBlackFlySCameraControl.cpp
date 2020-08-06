#include "FlirBlackFlySCameraControl.h"


FlirBlackFlySCameraControl::FlirBlackFlySCameraControl()
{
	streaming_ = false;
	// Retrieve singleton reference to system object
	system_ = System::GetInstance();
}

FlirBlackFlySCameraControl::~FlirBlackFlySCameraControl()
{
	system_->ReleaseInstance();
}

int FlirBlackFlySCameraControl::Start()
{
	if(streaming_.load())
	{
		cout << "Cameras started. Stop first" << endl;
		return -1;
	}

	int result = 0;


	// Retrieve list of cameras from the system
	camList_ = system_->GetCameras();

	unsigned int numCameras = camList_.GetSize();

	cout << "Number of cameras detected: " << numCameras << endl << endl;

	// Finish if there are no cameras
	if (numCameras == 0)
	{
		cout << "Not enough cameras!" << endl;
		return -1;
	}

	streaming_.store(true);

	try
	{
		// configure each camera
		for (unsigned int i = 0; i < numCameras; i++)
		{
			
			CameraPtr pCam = camList_.GetByIndex(i);

			pCamVec_.push_back(pCam);

			pCam->Init();

			// Retrieve device serial number for filename
			gcstring deviceSerialNumber("");

			INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

			CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");

			if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
			{
				deviceSerialNumber = ptrStringSerial->GetValue();

				SNVec_.push_back(deviceSerialNumber);

				//cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
			}


			INodeMap& nodeMap = pCam->GetNodeMap();

			int err = ConfigureTrigger(nodeMap);

			// Retrieve the enumeration node from the nodemap
		    CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");
		    if (IsAvailable(ptrPixelFormat) && IsWritable(ptrPixelFormat))
		    {
		        // Retrieve the desired entry node from the enumeration node
		        CEnumEntryPtr ptrPixelFormatMono8 = ptrPixelFormat->GetEntryByName("BayerRG8");
		        if (IsAvailable(ptrPixelFormatMono8) && IsReadable(ptrPixelFormatMono8))
		        {
		            // Retrieve the integer value from the entry node
		            int64_t pixelFormatMono8 = ptrPixelFormatMono8->GetValue();

		            // Set integer as new value for enumeration node
		            ptrPixelFormat->SetIntValue(pixelFormatMono8);

		            cout << "Pixel format set to " << ptrPixelFormat->GetCurrentEntry()->GetSymbolic() << "..." << endl;
		        }
		        else
		        {
		            cout << "Pixel format mono 8 not available..." << endl;
		        }
		    }
		    else
		    {
		        cout << "Pixel format not available..." << endl;
		    }

		    CEnumerationPtr ptrGainAuto = nodeMap.GetNode("GainAuto");
		    if (!IsAvailable(ptrGainAuto) || !IsWritable(ptrGainAuto))
		    {
		        cout << "Unable to disable automatic gain (node retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    CEnumEntryPtr ptrGainAutoOff = ptrGainAuto->GetEntryByName("Off");
		    if (!IsAvailable(ptrGainAutoOff) || !IsReadable(ptrGainAutoOff))
		    {
		        cout << "Unable to disable automatic gain (enum entry retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    ptrGainAuto->SetIntValue(ptrGainAutoOff->GetValue());

		    CEnumerationPtr ptrBalanceWhiteAuto = nodeMap.GetNode("BalanceWhiteAuto");
		    if (!IsAvailable(ptrBalanceWhiteAuto) || !IsWritable(ptrBalanceWhiteAuto))
		    {
		        cout << "Unable to disable automatic white balance (node retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    CEnumEntryPtr ptrBalanceWhiteAutoOff = ptrBalanceWhiteAuto->GetEntryByName("Off");
		    if (!IsAvailable(ptrBalanceWhiteAutoOff) || !IsReadable(ptrBalanceWhiteAutoOff))
		    {
		        cout << "Unable to disable automatic gain (enum entry retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    ptrBalanceWhiteAuto->SetIntValue(ptrBalanceWhiteAutoOff->GetValue());


		    CEnumerationPtr ptrExposureAuto = nodeMap.GetNode("ExposureAuto");
		    if (!IsAvailable(ptrExposureAuto) || !IsWritable(ptrExposureAuto))
		    {
		        cout << "Unable to disable automatic exposure (node retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    CEnumEntryPtr ptrExposureAutoOff = ptrExposureAuto->GetEntryByName("Off");
		    if (!IsAvailable(ptrExposureAutoOff) || !IsReadable(ptrExposureAutoOff))
		    {
		        cout << "Unable to disable automatic exposure (enum entry retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    ptrExposureAuto->SetIntValue(ptrExposureAutoOff->GetValue());


			CEnumerationPtr ptrExposureMode = nodeMap.GetNode("ExposureMode");
		    if (!IsAvailable(ptrExposureMode) || !IsWritable(ptrExposureMode))
		    {
		        cout << "Unable to disable automatic exposure (node retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    CEnumEntryPtr ptrExposureModeTimed = ptrExposureMode->GetEntryByName("Timed");
		    if (!IsAvailable(ptrExposureModeTimed) || !IsReadable(ptrExposureModeTimed))
		    {
		        cout << "Unable to disable automatic exposure (enum entry retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    ptrExposureMode->SetIntValue(ptrExposureModeTimed->GetValue());
		    cout << "Exposure mode timed..." << endl;

			//unit microseconds
			pCam->ExposureTime.SetValue(700.);
			pCam->Gain.SetValue(30.);
			pCam->GammaEnable.SetValue(false);

		    CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
		    if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
		    {
		        cout << "Unable to set acquisition mode to continuous (node retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
		    if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
		    {
		        cout << "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting..." << endl << endl;
		        return -1;
		    }
		    int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
		    ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

		    pCam->BeginAcquisition();

			cout << "Camera " << i << " start acquisition in hardware trigger mode." << endl;
		}


		triggerThreadVec_.push_back(std::thread(&FlirBlackFlySCameraControl::StreamByTrigger, this));
		
	}
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

	return result;
}


int FlirBlackFlySCameraControl::Stop()
{
	int result = 0;

	streaming_.store(false);

	for(auto & t : triggerThreadVec_)
		t.join();

	for(auto & t: grabThreadVec_)
		t.join();


	triggerThreadVec_.clear();

	grabThreadVec_.clear();

	pCamVec_.clear();


	// Clear camera list before releasing system
	try
	{
		for(unsigned int i=0; i< camList_.GetSize(); i++)
		{
			
			CameraPtr pCam = camList_.GetByIndex(i);

			pCam->EndAcquisition();
			
			pCam->DeInit();
		}

	}
	catch (Spinnaker::Exception& e)
	{
	    cout << "Error: " << e.what() << endl;
	    result = -1;
	}

    camList_.Clear();


	return result;
}

void FlirBlackFlySCameraControl::StreamByTrigger()
{
	unsigned int imageCnt = 0;

	unsigned int numCam = camList_.GetSize();
	
	cvImgVec_.clear();

	cvImgVec_.resize(numCam);
	
	while(streaming_.load())
	{

		imageCnt++;

#if 1
		//trigger the camera via GPIO
		jetsonXavierGPIONumber pin16 = gpio256;
		gpioExport(pin16);
		gpioSetDirection(pin16, outputPin);
		gpioSetValue(pin16, on);
		usleep(500);
		gpioSetValue(pin16, off);     
		gpioUnexport(pin16);
#endif

		//multi-threading ~40ms,	single thread ~50ms
#if 1

		TP t0 = Time::now();

		grabThreadVec_.clear();
		if(numCam > 1)
		{
			for(int i=1; i<camList_.GetSize(); i++)
			{
				CameraPtr pCam = pCamVec_[i];
				grabThreadVec_.push_back(std::thread(&FlirBlackFlySCameraControl::GrabFrame, this, pCam, i));
			}
		}

		GrabFrame(pCamVec_[0], 0);

		for(auto &t : grabThreadVec_)
			t.join();

		grabThreadVec_.clear();

		TP t1 = Time::now();
		SecD duration = duration_cast<SecD>(t1-t0);
		cout<<duration.count()<<" s\n";	
#endif
	
#if 0
		try
		{
			TP t0 = Time::now();
			for(int cameraID = 0; cameraID<numCam; cameraID++)
			{
				CameraPtr pCam = pCamVec_[cameraID];

				GrabFrame(pCam, cameraID);
			}

			TP t1 = Time::now();
			SecD duration = duration_cast<SecD>(t1-t0);
			cout<<duration.count()<<" s\n";
        }
        catch (Spinnaker::Exception& e)
        {
            cout << "Error: " << e.what() << endl;
            streaming_.store(false);
        }
#endif

		for(int i=0; i<cvImgVec_.size(); i++)
		{
			Mat resizedImg;
			resize(cvImgVec_[i], resizedImg, Size(), 0.25, 0.25);
			string name = to_string(i);
			imshow(SNVec_[i].c_str(), resizedImg);
			waitKey(2);

            // Create a unique filename
            ostringstream filename;
			
			filename << "/media/agcypher/SSD/";
			
            filename << "Trigger-";
            filename << SNVec_[i].c_str() << "-";
            filename << imageCnt << ".jpg";

            // Save image
            //pResultImage->Save(filename.str().c_str());

            //cout << "Image saved at " << filename.str() << endl;
		}
	}	

}

void FlirBlackFlySCameraControl::GrabFrame(CameraPtr pCam, unsigned int cameraID)
{
    int result = 0;
    int err = 0;

	try
	{
	    // Retrieve the next received image
		CameraPtr pCam = pCamVec_[cameraID];

        ImagePtr pResultImage = pCam->GetNextImage(1000);

        if (pResultImage->IsIncomplete())
        {
            cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl
                 << endl;
        }
        else
        {
            ImagePtr convertedImage = pResultImage->Convert(PixelFormat_BGR8);

            const auto XPadding = convertedImage->GetXPadding();
            const auto YPadding = convertedImage->GetYPadding();
            const auto rowsize = convertedImage->GetWidth();
            const auto colsize = convertedImage->GetHeight();

            // Image data contains padding. When allocating cv::Mat container size, you need to account for the X,Y
			//XPadding: 0; YPadding: 0; stride: 7344
            Mat cvImg = Mat((int)(colsize + YPadding), (int)(rowsize + XPadding), CV_8UC3, convertedImage->GetData(), convertedImage->GetStride());

			// need to deep copy, otherwise, the two images are the same
			cvImgVec_[cameraID] = cvImg.clone();

        }


        // Release image
        //
        // *** NOTES ***
        // Images retrieved directly from the camera (i.e. non-converted
        // images) need to be released in order to keep from filling the
        // buffer.
        //
        pResultImage->Release();
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        streaming_.store(false);
    }

}


// This function configures the camera to use a trigger. First, trigger mode is
// set to off in order to select the trigger source. Once the trigger source
// has been selected, trigger mode is then enabled, which has the camera
// capture only a single image upon the execution of the chosen trigger.
int FlirBlackFlySCameraControl::ConfigureTrigger(INodeMap& nodeMap)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING TRIGGER ***" << endl << endl;

    try
    {
        //
        // Ensure trigger mode off
        //
        // *** NOTES ***
        // The trigger must be disabled in order to configure whether the source
        // is software or hardware.
        //
        CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
        if (!IsAvailable(ptrTriggerMode) || !IsReadable(ptrTriggerMode))
        {
            cout << "Unable to disable trigger mode (node retrieval). Aborting..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
        if (!IsAvailable(ptrTriggerModeOff) || !IsReadable(ptrTriggerModeOff))
        {
            cout << "Unable to disable trigger mode (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

        cout << "Trigger mode disabled..." << endl;

        //
        // Set TriggerSelector to FrameStart
        //
        // *** NOTES ***
        // For this example, the trigger selector should be set to frame start.
        // This is the default for most cameras.
        //
        CEnumerationPtr ptrTriggerSelector = nodeMap.GetNode("TriggerSelector");
        if (!IsAvailable(ptrTriggerSelector) || !IsWritable(ptrTriggerSelector))
        {
            cout << "Unable to set trigger selector (node retrieval). Aborting..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerSelectorFrameStart = ptrTriggerSelector->GetEntryByName("FrameStart");
        if (!IsAvailable(ptrTriggerSelectorFrameStart) || !IsReadable(ptrTriggerSelectorFrameStart))
        {
            cout << "Unable to set trigger selector (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerSelector->SetIntValue(ptrTriggerSelectorFrameStart->GetValue());

        cout << "Trigger selector set to frame start..." << endl;

        //
        // Select trigger source
        //
        // *** NOTES ***
        // The trigger source must be set to hardware or software while trigger
        // mode is off.
        //
        CEnumerationPtr ptrTriggerSource = nodeMap.GetNode("TriggerSource");
        if (!IsAvailable(ptrTriggerSource) || !IsWritable(ptrTriggerSource))
        {
            cout << "Unable to set trigger mode (node retrieval). Aborting..." << endl;
            return -1;
        }


        // Set trigger mode to hardware ('Line0')
        CEnumEntryPtr ptrTriggerSourceHardware = ptrTriggerSource->GetEntryByName("Line0");
        if (!IsAvailable(ptrTriggerSourceHardware) || !IsReadable(ptrTriggerSourceHardware))
        {
            cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerSource->SetIntValue(ptrTriggerSourceHardware->GetValue());

        cout << "Trigger source set to hardware..." << endl; 


		CFloatPtr ptrTriggerDelay = nodeMap.GetNode("TriggerDelay");
	    if (!IsAvailable(ptrTriggerDelay) || !IsWritable(ptrTriggerDelay))
	    {
	        cout << "Unable to set trigger delay. Aborting..." << endl << endl;
	        return -1;
	    }

	    // Ensure desired trigger delay time does not exceed the maximum
	    const double triggerDelayMax = ptrTriggerDelay->GetMax();
	    double triggerDelayToSet = 28.0;

	    if (triggerDelayToSet > triggerDelayMax)
	    {
	        triggerDelayToSet = triggerDelayMax;
	    }

	    ptrTriggerDelay->SetValue(triggerDelayToSet);

        //
        // Turn trigger mode on
        //
        // *** LATER ***
        // Once the appropriate trigger source has been set, turn trigger mode
        // on in order to retrieve images using the trigger.
        //

        CEnumEntryPtr ptrTriggerModeOn = ptrTriggerMode->GetEntryByName("On");
        if (!IsAvailable(ptrTriggerModeOn) || !IsReadable(ptrTriggerModeOn))
        {
            cout << "Unable to enable trigger mode (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOn->GetValue());

        cout << "Trigger mode turned back on..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

int FlirBlackFlySCameraControl::setExposureTime(double duration_ms)
{

	int result = 0;

	try
	{
		for(int i=0; i<pCamVec_.size(); i++)
		{
			pCamVec_[i]->ExposureTime.SetValue(duration_ms);
		}
	}
	catch (Spinnaker::Exception& e)
	{
		cout << "Error: " << e.what() << endl;
        result = -1;

	}

	return result;
}


