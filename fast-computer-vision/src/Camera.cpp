//=============================================================================
// FAST Computer Vision
// A computer vision application to track ArUco markers.
//
// Copyright (C) 2024 Museum of Science, Boston
// <https://www.mos.org/>
//
// This program was developed through a grant to the Museum of Science, Boston
// from the Institute of Museum and Library Services under
// Award #MG-249646-OMS-21. For more information about this grant, see
// <https://www.imls.gov/grants/awarded/mg-249646-oms-21>.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see
// <https://www.gnu.org/licenses/gpl-3.0.html>.
//=============================================================================

#include "Camera.h"

Camera::Camera() :
    isConnected(false),
    currentFrameNumber(0),
    gamma(0.5),
    isApplyCalibration(false),
    isApplyCalibrationPreview(false)
{
    // Retrieve singleton reference to system object
    spinnakerSystem = Spinnaker::System::GetInstance();

    // Print out current library version
    const Spinnaker::LibraryVersion spinnakerLibraryVersion = spinnakerSystem->GetLibraryVersion();
    std::cout << "Spinnaker library version: "
        << spinnakerLibraryVersion.major << "."
        << spinnakerLibraryVersion.minor << "."
        << spinnakerLibraryVersion.type << "."
        << spinnakerLibraryVersion.build << std::endl << std::endl;

    pCamera = NULL;

    outputImage = cv::Mat(kImageHeight, kImageWidth, CV_8UC3);
}

Camera::~Camera()
{
    Disconnect();
    spinnakerSystem->ReleaseInstance();
}

void Camera::Run()
{
    if (isConnected) {
        GetFrame();
    }
    else {
        Connect();
    }
}

bool Camera::GetIsConnected()
{
    return isConnected;
}

void Camera::CopyImageTo(cv::Mat& destinationImage)
{
    outputImage.copyTo(destinationImage);
}

cv::Size Camera::GetResolution()
{
    return outputImage.size();
}

unsigned int Camera::GetFrameNumber()
{
    return currentFrameNumber;
}

double Camera::GetFrameRate()
{
    return frameRateTimer.frameRate;
}

void Camera::Calibrate(CalibrationData calibrationData)
{
    if (calibrationData.type == CalibrationType::Preview) {
        this->calibrationPreviewData = calibrationData;
    }
    else if (calibrationData.type == CalibrationType::Saved) {
        this->calibrationData = calibrationData;
    }
}

void Camera::ToggleCalibrationPreview(bool isOn)
{
    isApplyCalibrationPreview = isOn;
}

void Camera::ToggleCalibration(bool isOn)
{
    isApplyCalibration = isOn;
}

void Camera::UpdateGamma(double gamma)
{
    std::lock_guard<std::mutex> lockGuard(cameraParametersMutex);
    this->gamma = gamma;

    if (isConnected) {
        try {
            pCamera->Gamma.SetValue(gamma);
        }
        catch (Spinnaker::Exception& exception) {
            std::cout << "UpdateGamma() Error: " << exception.what() << std::endl;
        }
    }
}

void Camera::UpdateRotation(bool isRotate)
{
    this->isRotate = isRotate;
}

void Camera::ToggleRotation(bool isOn)
{
    isApplyRotation = isOn;
}

void Camera::Connect()
{
    frameRateTimer.Reset();

    cv::Mat zeros = cv::Mat::zeros(outputImage.size(), outputImage.type());
    zeros.copyTo(outputImage);

    // Retrieve list of cameras from the system
    cameraList = spinnakerSystem->GetCameras();
    unsigned int numCameras = cameraList.GetSize();

    // Fail if no cameras are detected
    if (numCameras < 1) {
        return;
    }

    // Select the first camera
    // There should never be more than one camera, but if there is just try the first one
    pCamera = cameraList.GetByIndex(0);

    try {
        // Retrieve TL device nodemap and print device information
        Spinnaker::GenApi::INodeMap& nodeMapTLDevice = pCamera->GetTLDeviceNodeMap();
        PrintDeviceInfo(nodeMapTLDevice);

        // Initialize camera
        pCamera->Init();

        {
            std::lock_guard<std::mutex> lockGuard(cameraParametersMutex);
            std::cout << std::endl << std::endl << "*** CAMERA CONFIGURATION ***" << std::endl << std::endl;

            // Retrieve stream parameters device nodemap
            Spinnaker::GenApi::INodeMap& streamNodeMap = pCamera->GetTLStreamNodeMap();
            // Set buffer handling mode
            Spinnaker::GenApi::CEnumerationPtr ptrHandlingMode = streamNodeMap.GetNode("StreamBufferHandlingMode");
            Spinnaker::GenApi::CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
            ptrHandlingModeEntry = ptrHandlingMode->GetEntryByName("NewestOnly");
            ptrHandlingMode->SetIntValue(ptrHandlingModeEntry->GetValue());
            std::cout << "Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName() << std::endl;

            // Set other camera parameters
            pCamera->AcquisitionMode.SetValue(Spinnaker::AcquisitionMode_Continuous);
            pCamera->AcquisitionFrameRateEnable.SetValue(false);
            pCamera->Gamma.SetValue(gamma);
        }

        // Begin acquiring images
        pCamera->BeginAcquisition();
        std::cout << std::endl << std::endl << "*** CAMERA ACQUISITION ***" << std::endl << std::endl;
        std::cout << "Acquiring images..." << std::endl;
    }
    catch (Spinnaker::Exception& exception) {
        std::cout << "Connect() Error: " << exception.what() << std::endl;
        Disconnect();
    }

    isConnected = true;
    emit CameraConnected();
    return;
}

void Camera::Disconnect()
{
    if (pCamera != NULL) {
        try {
            // Release image pointer
            if (pImage->IsInUse()) {
                pImage->Release();
            }
            pImage = NULL;

            // End acquisition
            if (pCamera->IsStreaming()) {
                pCamera->EndAcquisition();
            }

            // Deinitialize camera
            if (pCamera->IsInitialized()) {
                pCamera->DeInit();
            }
        }
        catch (Spinnaker::Exception& exception) {
            std::cout << "Disconnect() Error: " << exception.what() << std::endl;
        }

        pCamera = NULL;
    }

    try {
        // Clear camera list before releasing system
        cameraList.Clear();
    }
    catch (Spinnaker::Exception& exception) {
        std::cout << "Disconnect() Error: " << exception.what() << std::endl;
    }

    emit CameraDisconnected();
}

void Camera::GetFrame()
{
    try {
        if (pCamera == NULL) {
            isConnected = false;
            emit CameraDisconnected();
            return;
        }
        if (!pCamera->IsInitialized() || !pCamera->IsStreaming()) {
            isConnected = false;
            emit CameraDisconnected();
            return;
        }
    }
    catch (Spinnaker::Exception& exception) {
        std::cout << "GetFrame() Camera Error: " << exception.what() << std::endl;

        isConnected = false;
        emit CameraDisconnected();
        return;
    }

    try {
        // Retrieve next received image
        pImage = pCamera->GetNextImage();

        executionTimer.Start();
        bool isImageReady = !(pImage->IsIncomplete() || pImage == NULL);
        if (isImageReady) {
            // Convert image to OpenCV mat
            Spinnaker::ImagePtr convertedImage = pImage->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::HQ_LINEAR);
            cv::Mat rawImage = cv::Mat(
                convertedImage->GetHeight() + convertedImage->GetYPadding(),
                convertedImage->GetWidth() + convertedImage->GetXPadding(),
                CV_8UC3, convertedImage->GetData(),
                convertedImage->GetStride());
            
            // Apply the camera calibration
            // This is done before rotating the image because calibration is calculated at 0 degrees.
            cv::Mat calibratedImage;
            if (isApplyCalibrationPreview && calibrationPreviewData.type == CalibrationType::Preview) {
                cv::remap(rawImage, calibratedImage, calibrationPreviewData.distortMap, calibrationPreviewData.undistortMap, cv::INTER_LINEAR);
            }
            else if (isApplyCalibration && calibrationData.type == CalibrationType::Saved) {
                cv::remap(rawImage, calibratedImage, calibrationData.distortMap, calibrationData.undistortMap, cv::INTER_LINEAR);
            }
            else {
                calibratedImage = rawImage;
            }

            // Rotate the image 180 degrees if needed
            cv::Mat rotatedImage;
            if (isApplyRotation && isRotate) {
                cv::rotate(calibratedImage, outputImage, cv::ROTATE_180);
            }
            else {
                calibratedImage.copyTo(outputImage);
            }

            currentFrameNumber++;
            frameRateTimer.Update();
        }

        // Release image pointer
        if (pImage->IsInUse()) {
            pImage->Release();
        }

        executionTimer.Stop();
        //std::cout << "Camera processing: " << executionTimer.duration << " ms" << std::endl;
    }
    catch (Spinnaker::Exception& exception) {
        std::cout << "GetFrame() Image Error: " << exception.what() << std::endl;
    }
}

bool Camera::PrintDeviceInfo(Spinnaker::GenApi::INodeMap& nodeMap)
{
    bool result = true;

    std::cout << std::endl << "*** DEVICE INFORMATION ***" << std::endl << std::endl;

    try
    {
        Spinnaker::GenApi::FeatureList_t features;
        Spinnaker::GenApi::CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
        if (IsAvailable(category) && IsReadable(category))
        {
            category->GetFeatures(features);

            Spinnaker::GenApi::FeatureList_t::const_iterator it;
            for (it = features.begin(); it != features.end(); ++it)
            {
                Spinnaker::GenApi::CNodePtr pfeatureNode = *it;
                std::cout << pfeatureNode->GetName() << " : ";
                Spinnaker::GenApi::CValuePtr pValue = (Spinnaker::GenApi::CValuePtr)pfeatureNode;
                std::cout << (IsReadable(pValue) ? pValue->ToString().c_str() : "Node not readable");
                std::cout << std::endl;
            }
        }
        else
        {
            std::cout << "Device control information not available." << std::endl;
        }
    }
    catch (Spinnaker::Exception& exception)
    {
        std::cout << "PrintDeviceInfo() Error: " << exception.what() << std::endl;
        result = false;
    }

    return result;
}
