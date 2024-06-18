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
#include "Camera.h"
#include "MarkerData.h"
#include "DetectorParameterData.h"
#include "FrameRateTimer.h"
#include "ExecutionTimer.h"
#include <QObject>

class MarkerDetection : public QObject
{
    Q_OBJECT

public:
	MarkerDetection(Camera& camera);
	~MarkerDetection();
    void Pause();
    void Run();
    void CopyImageTo(cv::Mat& destinationImage);
    bool GenerateMarkerImages(int imageSize);
    std::map<int, MarkerData> GetTrackingData();
    unsigned int GetFrameNumber();
    double GetFrameRate();

public slots:
    void UpdateTrackingArea(cv::Rect2d trackingArea);
    void UpdateDetectorParameters(DetectorParameterData detectorParameters);

private:
    void DrawGuides(cv::Mat &image);
    void DrawMarkers(cv::Mat &image);
    cv::Scalar ScalarHSV2BGR(uchar H, uchar S, uchar V);

    Camera & camera;
	cv::Mat inputImage;
    cv::Mat trackingImage;
    cv::Mat outputImage;

    bool isDetected;
    std::map<int, MarkerData> trackingData;

    cv::Rect2d trackingArea;
    cv::Rect2d trackingAreaInPixels;

    unsigned int currentFrameNumber;
    unsigned int lastFrameNumber;

	DetectorParameterData detectorParameters;
	cv::Ptr<cv::aruco::Dictionary> markerDictionary;
	cv::Ptr<cv::aruco::DetectorParameters> markerParameters;
	cv::Ptr<cv::aruco::RefineParameters> refineParameters;

	cv::aruco::ArucoDetector arucoDetector;
	std::vector<std::vector<cv::Point2f>> markerCorners;
	std::vector<std::vector<cv::Point2f>> rejectedCandidates;
	std::vector<int> markerIds;

    std::mutex outputImageMutex;
    std::mutex trackingAreaMutex;
    std::mutex trackingDataMutex;
    std::mutex detectorParametersMutex;

    FrameRateTimer frameRateTimer;
    ExecutionTimer executionTimer;
};

