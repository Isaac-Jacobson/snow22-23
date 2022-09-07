//=============================================================================
// Copyright � 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: MultipleCameraEx.cpp,v 1.17 2010-02-26 01:00:50 soowei Exp $
//=============================================================================

#include "stdafx.h"

#include "FlyCapture2.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <Windows.h>
#include <chrono>
#include <future>
#include <fstream>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <cuda_runtime.h>
#include <string>

using namespace FlyCapture2;
using namespace std;

Error error;

Error CheckCamBuffer(Camera* Cam, Image &Im)
{
	Error Er;
	Cam->RetrieveBuffer(&Im);
	return Er;
}

void PrintError(Error error)
{
	error.PrintErrorTrace();
}

int setProperty(Camera &camera, Property &property, PropertyType type,
	const float f)
{
	property.type = type;
	error = camera.GetProperty(&property);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	property.absControl = true;
	property.onePush = false;
	property.onOff = true;
	property.autoManualMode = false;

	property.absValue = f;

	error = camera.SetProperty(&property);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}


	return 0;
}

string get_time()
{
	//Function to get the current timestamp and turn it into a string
	SYSTEMTIME st;
	GetLocalTime(&st);
	string stime;
	unsigned int year = st.wYear;
	unsigned int month = st.wMonth;
	unsigned int hours = st.wHour;
	unsigned int minutes = st.wMinute;
	unsigned int seconds = st.wSecond;
	unsigned int milliseconds = st.wMilliseconds;
	unsigned int day = st.wDay;
	//Concatenate the time stamp string
	stime += to_string(year);
	stime += "-";
	stime += to_string(month);
	stime += "-";
	stime += to_string(day);
	stime += "-";
	stime += to_string(hours);
	stime += "-";
	stime += to_string(minutes);
	stime += "-";
	stime += to_string(seconds);
	stime += "-";
	stime += to_string(milliseconds);
	return stime;
}

static const void saveImage(Image& im, int count, int picNum, string id, string root) {
	//creating a folder for each hour of capturing
	std::cout << "saveImage called, Camera ID: " << id << " Count: " << count << endl;
	SYSTEMTIME st;
	GetLocalTime(&st);
	string time_stamped_folder = "";
	unsigned int year = st.wYear;
	unsigned int month = st.wMonth;
	unsigned int day = st.wDay;
	unsigned int hours = st.wHour;
	
	//concatenate string to create folder path
	time_stamped_folder = to_string(month) + "_" + to_string(day) + "_" + to_string(year) + "__" + to_string(hours);
	string time_stamped_path = root + "/" + time_stamped_folder;
	if (CreateDirectory(time_stamped_path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		//Directory created
	}

	//cout << "saving. ID: " << id << "\n";
	string t = get_time();
	
	// Create formatted string so that images are sorted as they get put into the folder
	char fileName[100];
	char* c_time_stamped_path = new char[time_stamped_path.length() + 1];
	strcpy_s(c_time_stamped_path, time_stamped_path.length() + 1, time_stamped_path.c_str());
	// c_time_stamped_path is correct now

	stringstream ss;
	ss.str(id);
	int newid = stoi(id);
	int testvar = id.length();

	char* c_id = new char[id.length() + 1];
	strcpy_s(c_id, id.length() + 1, id.c_str());

	char* c_t = new char[t.length() + 1];
	strcpy_s(c_t, t.length() + 1, t.c_str());

	//cout << c_time_stamped_path << " and " << c_id << " and " << c_t << "\n";
	sprintf_s(fileName, "%s/Flake%.6u_Cam%s_%u_%s.bmp", c_time_stamped_path, count, c_id, picNum, c_t);
	//printf("%s", fileName);

	string file = time_stamped_path + "/Flake" + to_string(count) + "_Cam" + id + "_" + to_string(picNum) + "_" + t + ".bmp";
	
	//im.Save(file.c_str());
	im.Save(fileName);

	std::terminate;
}

class SSS_Camera
{
public:
	unsigned int id;
	Camera* cam;
	Image Im;
	Image Im1;
	Image Im2, Im3, Im4, Im5, Im6;
	string path;
	string msg;
	string filename;

	SSS_Camera()
	{
		//Default constructor...
	}

	//Constructor method...
	SSS_Camera(unsigned int id, Camera* cam, string path)
	{
		Image im;
		Image im1; //im2;, im3, im4, im5, im6;
		//Image im1;
		this->id = id;
		this->cam = cam;
		this->Im = im;
		this->path = path;
		this->msg = "Successfully started image capture for camera " + to_string(this->id);
		if (this->id == 1)
		{
			//When high speed camera is being run
			this->Im1 = im1;
			//this->Im2 = im2;
			//this->Im3 = im3;
			//this->Im4 = im4;
			//this->Im5 = im5;
			//this->Im6 = im6;
		}   // This section commented out to try to get camera 1 to only take 1 image
	}
	//The method to be run within the separate threads
	void run_Cam(std::atomic<bool>& program_is_running, std::atomic<int>& imagesCapturedSinceSave, std::atomic<bool>& allowSave, std::atomic<int>& globalFlakeCount)
	{
		Error error;
		bool hasPicture0 = false;
		bool hasPicture1 = false;
		//bool hasPicture2 = false;
		//bool hasPicture3 = false;
		//bool hasPicture4 = false;
		//bool hasPicture5 = false;
		//bool hasPicture6 = false;
		//cout << this->msg << endl;
		string id = to_string(this->id);
		string root = this->path;
		unsigned int count = 0;
		unsigned int rows = 0;
		unsigned int cols = 0;
		int return_val = 0;
		

		if (this->id == 1)                          //change 23 to 1 to enable old fall speed mode, 23 is strobing method
		{
			unsigned int captureTime, saveDelay;
			SYSTEMTIME st;
			while (program_is_running)
			{
				
				//std::cout << "Here" << endl;
				Error error0;
				Error error1;
				//Error error2, error3, error4, error5, error6;
				//cout << "Cam: " << this->cam << endl;
				//cout << "This Id: " << this->id << "ID: " << id << endl;
				if (hasPicture0 == true) {
					std::cout << "Has Picture Before 0: true" << hasPicture0 << endl;
					
					
					return_val = 1;
				}
				if (hasPicture1 == true) {
					std::cout << "Has Picture Before 1: true" << hasPicture1 << endl;

					return_val = 0;
				}
				/*if (hasPicture2 == true) {
					std::cout << "Has Picture Before 2: true" << hasPicture2 << endl;
				}
				if (hasPicture3 == true) {
					std::cout << "Has Picture Before 3: true" << hasPicture3 << endl;
				}
				if (hasPicture4 == true) {
					std::cout << "Has Picture Before 4: true" << hasPicture4 << endl;
				}
				if (hasPicture5 == true) {
					std::cout << "Has Picture Before 5: true" << hasPicture5 << endl;
				}
				if (hasPicture6 == true) {
					std::cout << "Has Picture Before 6: true" << hasPicture6 << endl;
				}*/
				
				if (hasPicture0 == false) {
					error0 = this->cam->RetrieveBuffer(&this->Im); // checks if image was captured, returns PGERROR_OK is true
					
					
				}
				if (hasPicture0 == true && hasPicture1 == false) {
					error1 = this->cam->RetrieveBuffer(&this->Im1);
					
				}
				/*if (hasPicture0 == true && hasPicture1 == true && hasPicture2 == false) {
					error2 = this->cam->RetrieveBuffer(&this->Im2);
						
				}
				if (hasPicture0 == true && hasPicture1 == true && hasPicture2 == true && hasPicture3 == false) {
					error3 = this->cam->RetrieveBuffer(&this->Im3);
				}
				if (hasPicture0 == true && hasPicture1 == true && hasPicture2 == true && hasPicture3 == true && hasPicture4 == false) {
					error4 = this->cam->RetrieveBuffer(&this->Im4);
				}
				if (hasPicture0 == true && hasPicture1 == true && hasPicture2 == true && hasPicture3 == true && hasPicture4 == true && hasPicture5 == false) {
					error5 = this->cam->RetrieveBuffer(&this->Im5);
				}
				if (hasPicture0 == true && hasPicture1 == true && hasPicture2 == true && hasPicture3 == true && hasPicture4 == true && hasPicture5 == true && hasPicture6 == false) {
					error6 = this->cam->RetrieveBuffer(&this->Im6);
				}*/

				
					// This method is called many times a second, not just on flash

					if (error0 == PGRERROR_OK) {
						//cout << "high speed captured first frame\n";
						std::cout << "PGERROR_OK 0" << endl;
						hasPicture0 = true;

						// Set the shutter property of the camera
						Property shutterProp;
						float HS_Shutter = 20.0f;//500.0f;
						float HS_Gain = 1.0f;
						if (setProperty(*cam, shutterProp, SHUTTER, HS_Shutter) != 0)
						{
							std::cout << "  SMAS: could not set shutter property." << endl;
						}
						else
						{
							std::cout << "  Successfully set shutter time for camera " << fixed << setprecision(2) << HS_Shutter << " ms" << endl;
						}

						// Set the gain property of the camera
						Property gainProp;



						if (setProperty(*cam, gainProp, GAIN, HS_Gain) != 0)
						{
							std::cout << "  SMAS: could not set gain property." << endl;
						}
						else
						{
							std::cout << "  Successfully set gain to " << HS_Gain << " dB" << endl;
						}

					}
					if (error1 == PGRERROR_OK) {
						//cout << "high speed captured second frame\n";
						std::cout << "PGERROR_OK 1" << endl;
						hasPicture1 = true;

						Property shutterProp;
						float HS_Shutter = 4.0f;//500.0f;
						float HS_Gain = 20.0f;
						if (setProperty(*cam, shutterProp, SHUTTER, HS_Shutter) != 0)
						{
							std::cout << "  SMAS: could not set shutter property." << endl;
						}
						else
						{
							std::cout << "  Successfully set shutter time for camera " << fixed << setprecision(2) << HS_Shutter << " ms" << endl;
						}

						// Set the gain property of the camera
						Property gainProp;



						if (setProperty(*cam, gainProp, GAIN, HS_Gain) != 0)
						{
							std::cout << "  SMAS: could not set gain property." << endl;
						}
						else
						{
							std::cout << "  Successfully set gain to " << HS_Gain << " dB" << endl;
						}
					}
					//if (error2 == PGRERROR_OK) {
					//	//cout << "high speed captured second frame\n";
					//	std::cout << "PGERROR_OK 2" << endl;
					//	hasPicture2 = true;
					//}
					//if (error3 == PGRERROR_OK) {
					//	//cout << "high speed captured second frame\n";
					//	std::cout << "PGERROR_OK 3" << endl;
					//	hasPicture3 = true;
					//}
					//if (error4 == PGRERROR_OK) {
					//	//cout << "high speed captured second frame\n";
					//	std::cout << "PGERROR_OK 4" << endl;
					//	hasPicture4 = true;
					//}
					//if (error5 == PGRERROR_OK) {
					//	//cout << "high speed captured second frame\n";
					//	std::cout << "PGERROR_OK 5" << endl;
					//	hasPicture5 = true;
					//}
					//if (error6 == PGRERROR_OK) {
					//	//cout << "high speed captured second frame\n";
					//	std::cout << "PGERROR_OK 6" << endl;
					//	hasPicture6 = true;
					//}

					if (hasPicture0 == true) {
						std::cout << "Has Picture 0: true" << hasPicture0 << endl;
					}
					if (hasPicture1 == true) {
						std::cout << "Has Picture 1: true" << hasPicture1 << endl;
					}
					/*if (hasPicture2 == true) {
						std::cout << "Has Picture 2: true" << hasPicture2 << endl;
					}
					if (hasPicture3 == true) {
						std::cout << "Has Picture 3: true" << hasPicture3 << endl;
					}
					if (hasPicture4 == true) {
						std::cout << "Has Picture 4: true" << hasPicture4 << endl;
					}
					if (hasPicture5 == true) {
						std::cout << "Has Picture 5: true" << hasPicture5 << endl;
					}
					if (hasPicture6 == true) {
						std::cout << "Has Picture 6: true" << hasPicture6 << endl;
					}*/

					//code works up to here at least

					Image* Img = &this->Im;
					if (count == 0)
					{
						rows = Img->GetRows();
						cols = Img->GetCols(); //Rows and columns should be the same for every image
					}

					//PGRERROR_OK indicates image sucessfully captured

					if (hasPicture0 == true && hasPicture1 == true) //&& hasPicture2 == true && hasPicture3 == true && hasPicture4 == true && hasPicture5 == true && hasPicture6 == true)
					{
						//cout << "image captured by " << id << "\n";
						GetLocalTime(&st);
						captureTime = st.wMilliseconds;
						//cout << "Incrementing counter for Cam " << id << "\n";
						imagesCapturedSinceSave++;
						hasPicture0 = false;
						hasPicture1 = false;
						/*hasPicture2 = false;
						hasPicture3 = false;
						hasPicture4 = false;
						hasPicture5 = false;
						hasPicture6 = false;*/
					}
					//std::cout << "allowSave = " << allowSave << " Camera ID: " << id << endl;
					if (allowSave) {
						imagesCapturedSinceSave = 0;
						allowSave = false;
						std::cout << "About to Call saveImage, Camera ID: " << id << endl;
						thread saveThread1(saveImage, Im, (int)globalFlakeCount, 1, id, root);
						saveThread1.detach();
						std::cout << "About to Call saveImage, Camera ID: " << id << endl;
						thread saveThread2(saveImage, Im1, (int)globalFlakeCount, 2, id, root);
						saveThread2.detach();
						/*
						std::cout << "About to Call saveImage, Camera ID: " << id << endl;
						thread saveThread3(saveImage, Im2, (int)globalFlakeCount, 3, id, root);
						saveThread3.detach();
						std::cout << "About to Call saveImage, Camera ID: " << id << endl;
						thread saveThread4(saveImage, Im3, (int)globalFlakeCount, 4, id, root);
						saveThread4.detach();
						std::cout << "About to Call saveImage, Camera ID: " << id << endl;
						thread saveThread5(saveImage, Im4, (int)globalFlakeCount, 5, id, root);
						saveThread5.detach(); 
						std::cout << "About to Call saveImage, Camera ID: " << id << endl;
						thread saveThread6(saveImage, Im5, (int)globalFlakeCount, 6, id, root);
						saveThread6.detach();
						std::cout << "About to Call saveImage, Camera ID: " << id << endl;
						thread saveThread7(saveImage, Im6, (int)globalFlakeCount, 7, id, root);
						saveThread7.detach();*/
					}
					Sleep(DWORD(2));
				}
			
			}
		else
		{
			while (program_is_running)
			{
				error = this->cam->RetrieveBuffer(&this->Im);
				//PGRERROR_OK indicates image sucessfully captured
				if (error == PGRERROR_OK)
				{
					//cout << "image captured by " << id << "\n";
					//cout << "Incrementing counter for Cam " << id << "\n";
					imagesCapturedSinceSave++;
				}
				bool t = true;
				if (allowSave.compare_exchange_strong(t, false)) {
					imagesCapturedSinceSave = 0;
					thread saveThread(saveImage, Im, (int)globalFlakeCount, 1, id, root);
					saveThread.detach();
				}
				Sleep(DWORD(2));
			}
		}
		//return return_val;
	}
};


class Snowflake_System
{
	/*This class will maintain all of the overarching processes of the system, including:
	->Number of cameras and general camera management
	->Asychronous timing in order to timeout certain process
	->Ability to start additional asynchronus threads for other necessary programs such as the empty image filter
	->Ability to time processes using the chrono library
	->Ability to error check the system and break out of certain processes
	*/
public:
	unsigned int numActiveCameras;
	string start_time_stamp;
	string end_time_stamp;
	Camera** sortedCams;
	unsigned int finalCount;
	string lastPic_time_stamp;
	string current_time;

	//The constructor method...
	Snowflake_System(unsigned int &numCams, Camera** sortedCams)
	{
		this->numActiveCameras = numCams;
		this->sortedCams = sortedCams;
		this->start_time_stamp = get_time();
	}
	//Method to clear the camera buffers 
	void clearCamBuffers()
	{
		unsigned int num = this->numActiveCameras;
		unsigned int buffCount = 1;
		//Check to see if all buffers are empty and wait until the buffer is not empty
		Image Im;
		future<Error> err;
		while (buffCount != 0)
		{
			buffCount = 0;
			for (unsigned int i = 0; i < num; i++)
			{
				chrono::system_clock::time_point duration = chrono::system_clock::now() + chrono::seconds(1);
				err = async(CheckCamBuffer, this->sortedCams[i], Im); //Run asynchronus thread
				if (future_status::ready == err.wait_until(duration)) //Check to see if the buffer ever finishes running
				{
					buffCount++; //Increment the buffer couter if it did not finish
				}
			}

			std::cout << "Number of Residual Buffers Found: " << to_string(buffCount) << endl;
		}
	}

	void set_time(string &t)
	{
		//This function will set the current time of the system corresponding to each picture that is taken
		this->lastPic_time_stamp = t;
	}

	void set_current_time()
	{
		this->current_time = get_time(); //Update the current time
	}
};

void PrintBuildInfo()
{
	FC2Version fc2Version;
	Utilities::GetLibraryVersion(&fc2Version);

	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	std::cout << version.str() << endl;

	ostringstream timeStamp;
	timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
	std::cout << timeStamp.str() << endl << endl;
}

void PrintCameraInfo(CameraInfo* pCamInfo)
{	
	std::cout << endl;
	std::cout << "*** CAMERA INFORMATION ***" << endl;
	std::cout << "Serial number -" << pCamInfo->serialNumber << endl;
	std::cout << "Camera model - " << pCamInfo->modelName << endl;
	std::cout << "Camera vendor - " << pCamInfo->vendorName << endl;
	std::cout << "Sensor - " << pCamInfo->sensorInfo << endl;
	std::cout << "Resolution - " << pCamInfo->sensorResolution << endl;
	std::cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
	std::cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;


}

void static updateAtomics(int numThreads, std::atomic<bool>& program_is_running, std::atomic<int>* imagesCapturedSinceSave, std::atomic<bool>* allowSave, std::atomic<int>& globalFlakeCount) {
	bool droppedFrame = false;
	bool readyToSave;
	while (program_is_running) {
		readyToSave = true;
		int frameDropCam = -1;
		for (int i = 0; i < numThreads; i++) { // numthreads = 7 = number of cameras
			//std::cout << "imagesCapturedSinceSave[i]: " <<imagesCapturedSinceSave[i] << "; thread number: " << i << endl;
			//cout << imagesCapturedSinceSave[i];
			switch (imagesCapturedSinceSave[i]) {
				case 2: // a frame was dropped, if imagesCapturedSinceSave[i] == 2
					droppedFrame = true;
					readyToSave = false;
					frameDropCam = i;
					break;
				case 0: // still waiting for capture
					readyToSave = false;
					break;
			}
			if (droppedFrame) {
				break;
			}
		}
		// the following handles when a save is skipped
		if (droppedFrame) {
			cout << "FRAME COUNT MANAGER: skipped save: " << frameDropCam << "\n";
			for (int i = 0; i < numThreads; i++) {
				switch (imagesCapturedSinceSave[i]) {
					case 1:
						imagesCapturedSinceSave[i] = 0;
						break;
					case 2:
						imagesCapturedSinceSave[i] = 1;
						break;
				}
			}
			droppedFrame = false;
		}
		if (readyToSave) {
			++globalFlakeCount;
			// set allow save flags to true to let runCam thread save captures
			cout << "FRAME COUNT MANAGER: allowing saves. " << globalFlakeCount << "\n";
			for (int i = 0; i < numThreads; i++) {
				allowSave[i] = true;
			}
		}
		readyToSave = false;
		Sleep(DWORD(2));
	}
}

int main(int /*argc*/, char** /*argv*/)
{
	//*******************************************************************************************************************
	//*******************************************************************************************************************
	//Parameters (Values that can be changed)
	string p = "C:/Test_07_21_21";// "F:\Flake Images"; // CHANGE THIS STRING TO CHANGE THE FOLDER WHERE IMAGES ARE SAVED. FOLDER MUST ALREADY EXIST.
	
	bool calibration = true;		//True uses calShutter and calGain instead of default value
									//False runs normal settings (Use for actual recording)
	
	bool highSpeedTrail = true;		//True uses HS_Shutter and HS_Gain for the high speed camera 
									//False uses same values as the other cameras (No camera streak/trail)
	
									//NOTE: Calibration overrides highSpeedTrail (if calibration == true, higSpeedTrail ignored)
	//Manual Values for shutter and gain
	float calShutter = 4.0f;		
	float calGain = 20.0f;
	float HS_Shutter = 4.0f;//500.0f;
	float HS_Gain = 20.0f;

	//*******************************************************************************************************************
	//*******************************************************************************************************************
	// Commands to start running the background program
	fstream logFile;
	string path = "C:/LogFiles/log_" + get_time() + ".txt";
	logFile.open(path, fstream::out);
	logFile << "Starting Time: " << get_time() << "\n";
	PrintBuildInfo();
	EmbeddedImageInfo	EmbeddedInfo;

	BusManager busMgr;
	unsigned int numCameras;
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	std::cout << "Number of cameras detected: " << numCameras << endl;
	logFile << "Number of cameras detected: " << numCameras << "\n";

	if (numCameras < 1)
	{
		std::cout << "Insufficient number of cameras... press Enter to exit." << endl;
		logFile << "INSUFFICIENT NUMBER OF CAMERAS DETECTED. THIS ERROR IS FATAL. ";
		cin.ignore();
		return -1;
	}
	
	Camera** ppCameras = new Camera*[numCameras];

	// Connect to all detected cameras and attempt to set them to
	// a common video mode and frame rate
	for (unsigned int i = 0; i < numCameras; i++)
	{
		ppCameras[i] = new Camera();

		PGRGuid guid;
		error = busMgr.GetCameraFromIndex(i, &guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Connect to a camera
		error = ppCameras[i]->Connect(&guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Get the camera information
		CameraInfo camInfo;
		error = ppCameras[i]->GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		PrintCameraInfo(&camInfo);
	}
	Camera** ppCamerasSorted = new Camera*[numCameras];
	for (unsigned int i = 0; i < numCameras; i++)
	{
		PGRGuid guid;
		error = busMgr.GetCameraFromIndex(i, &guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		CameraInfo camInfo;
		error = ppCameras[i]->GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		//Initializing the camera serial numbers and sorting the array of camera objects
		if (camInfo.serialNumber == 15444692) {
			ppCamerasSorted[0] = ppCameras[i]; //15444692 = camera 0
			std::cout << "Camera 0 is declared" << "\n";
		}
		if (camInfo.serialNumber == 13510226) {
			ppCamerasSorted[1] = ppCameras[i]; //SPEED CAMERA!!! 13510226 = camera 1
			std::cout << "Camera 1 is declared" << "\n";
		}
		if (camInfo.serialNumber == 15444697) {
			ppCamerasSorted[2] = ppCameras[i]; //15444697 = camera 2
			std::cout << "Camera 2 is declared" << "\n";
		}
		if (camInfo.serialNumber == 15444696) {
			ppCamerasSorted[3] = ppCameras[i]; //15444696 = camera 3
			std::cout << "Camera 3 is declared" << "\n";
		}
		if (camInfo.serialNumber == 15405697) {
			ppCamerasSorted[4] = ppCameras[i]; //15405697 = camera 4
			std::cout << "Camera 4 is declared" << "\n";
		}
		if (camInfo.serialNumber == 15444687) {
			ppCamerasSorted[5] = ppCameras[i]; //15444687 = camera 5
			std::cout << "Camera 5 is declared" << "\n";
		}
		if (camInfo.serialNumber == 15444691) {
			ppCamerasSorted[6] = ppCameras[i]; //15444691 = camera 6
			std::cout << "Camera 6 is declared" << "\n";
		}
	}
	cout << "The whole array: " << ppCamerasSorted << endl;
	cout << "The 0th instance: " << ppCamerasSorted[1] << endl;
	for (unsigned int i = 0; i < numCameras; i++)
	{

		FC2Config Config;
		//if (i == 1) {
		//	FrameRate fr = FRAMERATE_240;
		//	std::cout << "numFrames: " << fr << endl;
		//	//FrameRate fr = FRAMERATE_FORCE_32BITS;
		//}
		//FrameRate fr = FRAMERATE_FORCE_32BITS;
		//PCIeBusSpeed PCIE = PCIE_BUSSPEED_5_0;
		//BusSpeed speed = BUSSPEED_S_FASTEST;

		// Set buffered mode
		error = ppCamerasSorted[i]->GetConfiguration(&Config);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		Config.numBuffers = 50;
		Config.grabMode = BUFFER_FRAMES;
		Config.highPerformanceRetrieveBuffer = true;
		Config.grabTimeout = 0; // was originally 0, set to 100000 for testing on 9/10/21 -Peter
		//Config.registerTimeoutRetries = 5;
		//Config.registerTimeout = 10000;
		//Config.asyncBusSpeed = BUSSPEED_S_FASTEST;
		ppCamerasSorted[i]->SetConfiguration(&Config);
		error = ppCamerasSorted[i]->GetConfiguration(&Config);
		std::cout << "Buffer Mode: " << Config.grabMode << endl;
		std::cout << "Number of Buffers: " << Config.numBuffers << endl;
		std::cout << "Grab Timeout: " << Config.grabTimeout << endl;
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		PGRGuid guid;

		// Get current trigger settings
		TriggerMode triggerMode;
		error = ppCamerasSorted[i]->GetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Set camera to trigger mode 0 (standard ext. trigger)
		//   Set GPIO to receive input from pin 0

		triggerMode.onOff = true;
		triggerMode.mode = 0;
		triggerMode.parameter = 0;
		triggerMode.polarity = 0;
		triggerMode.source = 0;
		error = ppCamerasSorted[i]->SetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		std::cout << "  Successfully set to external trigger, low polarity, source GPIO 0." << endl;

		// Set the shutter property of the camera
		Property shutterProp;
		float k_shutterVal = 4.0f;		//Default Value
			
		if (calibration) {
			k_shutterVal = calShutter;
		}
		else if (i == 1){ //&& highSpeedTrail) {
			k_shutterVal = HS_Shutter;
		}

		if (setProperty(*ppCamerasSorted[i], shutterProp, SHUTTER, k_shutterVal) != 0)
		{
			std::cout << "  SMAS: could not set shutter property." << endl;
		}
		else
		{
			std::cout << "  Successfully set shutter time for camera " << i << " to " << fixed << setprecision(2) << k_shutterVal << " ms" << endl;
		}

		// Set the gain property of the camera
		Property gainProp;
		float k_gainVal = 20.0f;
		
		if (calibration) {
			k_gainVal = calGain;
		}
		else if (i == 1) { // && highSpeedTrail) {
			k_gainVal = HS_Gain;
		}

		if (setProperty(*ppCamerasSorted[i], gainProp, GAIN, k_gainVal) != 0)
		{
			std::cout << "  SMAS: could not set gain property." << endl;
		}
		else
		{
			std::cout << "  Successfully set gain to " << fixed << setprecision(2) << k_gainVal << " dB" << endl;
		}

		// Get current embedded image info
		error = ppCamerasSorted[i]->GetEmbeddedImageInfo(&EmbeddedInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// If camera supports timestamping, set to true
		if (EmbeddedInfo.timestamp.available == true)
		{
			EmbeddedInfo.timestamp.onOff = true;
			std::cout << "  Successfully enabled timestamping." << endl;
		}
		else
		{
			std::cout << "Timestamp is not available!" << endl;
		}

		// If camera supports frame counting, set to true
		if (EmbeddedInfo.frameCounter.available == true)
		{
			EmbeddedInfo.frameCounter.onOff = true;
			std::cout << "  Successfully enabled frame counting." << endl;
		}
		else
		{
			std::cout << "Framecounter is not avalable!" << endl;
		}

		// Sets embedded info
		error = ppCamerasSorted[i]->SetEmbeddedImageInfo(&EmbeddedInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Initializing camera capture capability
		error = ppCamerasSorted[i]->StartCapture();
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			std::cout << "Error starting to capture images. Error from camera " << i << endl
				<< "Press Enter to exit." << endl;
			cin.ignore();
			return -1;
		}

		
	}

	std::cout << "\nBeginning capture. Waiting for signal..." << endl;
	logFile << "The sequential setup has been completed. All cameras detected have been declared. \n";
	SSS_Camera* CamList = new SSS_Camera[numCameras];
	
	// TODO: ideally the above should be inputted as an argument when starting execution of the application
	bool exitC = false;
	for (unsigned int i = 0; i < numCameras; i++)
	{
		//Clear all camera data first
		error = ppCamerasSorted[i]->ResetStats();
		CamList[i] = SSS_Camera(i, ppCamerasSorted[i], p);
	}
	logFile << "All objects for the parallel threads have been declared. \n";

	// This is the portion of the code that starts to perform the parallel setup
	int nThreads = numCameras;
	thread* threadList = new thread[nThreads];

	std::atomic<bool> program_is_running{ true };

	std::atomic<int>* imagesCapturedSinceSave = new std::atomic<int>[nThreads]; // number of images caputured by each thread since last save
	std::atomic<bool>* allowSave = new std::atomic<bool>[nThreads]; // 0: don't save, 1: save

	std::atomic<int> globalFlakeCount = -1;

	for (int i = 0; i < nThreads; i++) {
		imagesCapturedSinceSave[i] = 0;
		allowSave[i] = false;
	}

	// thread to coordinate saving images
	thread updateAtomicsThread(updateAtomics, nThreads, std::ref(program_is_running), imagesCapturedSinceSave, allowSave, std::ref(globalFlakeCount));
	std::cout << "updateAtomics called: " << endl;
	for (int i = 0; i < nThreads; i++) {
		std::cout << "imagesCapturedSinceSave: " << imagesCapturedSinceSave[i] << "; thread number: " << i << "\n" << endl;
	}

	for (unsigned int i = 0; i < nThreads; i++)
	{
		threadList[i] = thread(&SSS_Camera::run_Cam, CamList[i], std::ref(program_is_running), std::ref(imagesCapturedSinceSave[i]), std::ref(allowSave[i]), std::ref(globalFlakeCount));
		
	}
	logFile << "Threads are running. Time is: " << get_time() << "\n";
	logFile.close();
	string command;
	std::cout << "There are now " + to_string(nThreads) + " threads running!" << endl;
	std::cout << "Enter 'exit' to just stop the camera capture and exit the application. " << endl << endl;
	while (true)
	{
		cin >> command;
		if (command == "exit")
		{
			std::cout << "Exiting...\n";
			break;
		}
		std::cout << "You typed: " << command << " which is not a valid command. ";
	}
	exitC = true;
	if (command == "exit")
	{
		program_is_running = false;
		for (unsigned int i = 0; i < numCameras; i++)
		{
			ppCameras[i]->StopCapture();
			ppCameras[i]->Disconnect();
			delete ppCameras[i];
		}
		std::cout << "Cameras disconnected.\n";
		delete[] ppCameras;
		delete[] ppCamerasSorted;
		delete[] imagesCapturedSinceSave;
		delete[] allowSave;
		return 0;
	}
	for (unsigned int i = 0; i < nThreads; i++)
	{
		threadList[i].join(); // Wait for the threads to finish...
		updateAtomicsThread.join();
	}
	return 0;
}
