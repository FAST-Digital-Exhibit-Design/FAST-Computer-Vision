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

#pragma once
#include "pch.h"
#include "CalibrationData.h"
#include "FrameRateTimer.h"
#include "ExecutionTimer.h"
#include <QObject>

class Camera : public QObject
{
    Q_OBJECT

public:
	Camera();
	~Camera();
    void Run();
    bool GetIsConnected(); // TODO: Replace with a signal on connect or disconnect
    void CopyImageTo(cv::Mat& destinationImage);
    cv::Size GetResolution();
    unsigned int GetFrameNumber();
    double GetFrameRate();

signals:
    void CameraConnected();
    void CameraDisconnected();

public slots:
    void Calibrate(CalibrationData calibrationData);
    void ToggleCalibrationPreview(bool isOn);
    void ToggleCalibration(bool isOn);
    void UpdateGamma(double gamma);
    void UpdateRotation(bool isRotate);
    void ToggleRotation(bool isOn);

private:
    void Connect();
	void Disconnect();
    void GetFrame();
	bool PrintDeviceInfo(Spinnaker::GenApi::INodeMap& nodeMap);

	Spinnaker::SystemPtr spinnakerSystem;
	Spinnaker::CameraList cameraList;
	Spinnaker::CameraPtr pCamera;
	Spinnaker::ImagePtr pImage;

    bool isConnected;
    cv::Mat outputImage;
    unsigned int currentFrameNumber;

    double gamma;

    bool isApplyRotation;
    bool isRotate;

    bool isApplyCalibration;
    CalibrationData calibrationData;

    bool isApplyCalibrationPreview;
    CalibrationData calibrationPreviewData;


    std::mutex cameraParametersMutex;

    FrameRateTimer frameRateTimer;
    ExecutionTimer executionTimer;
};


