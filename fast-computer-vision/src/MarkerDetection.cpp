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

#include "MarkerDetection.h"

MarkerDetection::MarkerDetection(Camera& camera) :
    camera(camera),
    isDetected(false),
    currentFrameNumber(0),
    lastFrameNumber(0),
    markerCorners(0),
	rejectedCandidates(0),
    markerIds(0)
{

    camera.CopyImageTo(inputImage);

	{
        std::lock_guard<std::mutex> lockGuard(outputImageMutex);
		inputImage.copyTo(outputImage);
	}

    markerDictionary = cv::aruco::generateCustomDictionary(
        detectorParameters.markerDictionarySize,
        detectorParameters.markerNumBits);
	refineParameters = cv::aruco::RefineParameters::create();
	markerParameters = cv::aruco::DetectorParameters::create();

    arucoDetector = cv::aruco::ArucoDetector(markerDictionary, markerParameters, refineParameters);
}

MarkerDetection::~MarkerDetection()
{
}

void MarkerDetection::Pause()
{
    frameRateTimer.Reset();
}

void MarkerDetection::Run()
{
    currentFrameNumber = camera.GetFrameNumber();
    if (lastFrameNumber == currentFrameNumber) {
        return;
    }

    executionTimer.Start();

    camera.CopyImageTo(inputImage);

    markerCorners.clear();
    markerIds.clear();
    rejectedCandidates.clear();

    {
        std::lock_guard<std::mutex> lockGuard(detectorParametersMutex);
        markerParameters->adaptiveThreshWinSizeMin = detectorParameters.adaptiveThreshWinSizeMin;
        markerParameters->adaptiveThreshWinSizeMax = detectorParameters.adaptiveThreshWinSizeMax;
        markerParameters->adaptiveThreshWinSizeStep = detectorParameters.adaptiveThreshWinSizeStep;
        markerParameters->adaptiveThreshConstant = detectorParameters.adaptiveThreshConstant;

        markerParameters->minMarkerPerimeterRate = detectorParameters.minMarkerPerimeterRate;
        markerParameters->maxMarkerPerimeterRate = detectorParameters.maxMarkerPerimeterRate;
        markerParameters->polygonalApproxAccuracyRate = detectorParameters.polygonalApproxAccuracyRate;
        markerParameters->minCornerDistanceRate = detectorParameters.minCornerDistanceRate;
        markerParameters->minMarkerDistanceRate = detectorParameters.minMarkerDistanceRate;
        markerParameters->minDistanceToBorder = detectorParameters.minDistanceToBorder;

        markerParameters->markerBorderBits = detectorParameters.markerBorderBits;
        markerParameters->minOtsuStdDev = detectorParameters.minOtsuStdDev;
        markerParameters->perspectiveRemovePixelPerCell = detectorParameters.perspectiveRemovePixelPerCell;
        markerParameters->perspectiveRemoveIgnoredMarginPerCell = detectorParameters.perspectiveRemoveIgnoredMarginPerCell;

        markerParameters->maxErroneousBitsInBorderRate = detectorParameters.maxErroneousBitsInBorderRate;
        markerParameters->errorCorrectionRate = detectorParameters.errorCorrectionRate;

        if (markerDictionary->bytesList.rows != detectorParameters.markerDictionarySize ||
            markerDictionary->markerSize != detectorParameters.markerNumBits)
        {
            markerDictionary = cv::aruco::generateCustomDictionary(
                detectorParameters.markerDictionarySize,
                detectorParameters.markerNumBits);
        }
        arucoDetector = cv::aruco::ArucoDetector(markerDictionary, markerParameters, refineParameters);
    }

    cv::Rect2d trackingAreaInPixels;
    {
        std::lock_guard<std::mutex> lockGuard(trackingAreaMutex);
        trackingAreaInPixels = cv::Rect2d(trackingArea.x * inputImage.cols,
            trackingArea.y * inputImage.rows,
            trackingArea.width * inputImage.cols,
            trackingArea.height * inputImage.rows);
    }

    trackingImage = inputImage(trackingAreaInPixels);
    if (!trackingImage.empty()) {
        arucoDetector.detectMarkers(trackingImage, markerCorners, markerIds, rejectedCandidates);
    }

	try {
		isDetected = ((int)markerIds.size() > 0);
	}
	catch (...) {
		isDetected = false;
	}

	{
        std::lock_guard<std::mutex> lockGuard(trackingDataMutex);
        trackingData.clear();

		if (isDetected) {
            cv::Point2d trackingAreaOffset(trackingAreaInPixels.x, trackingAreaInPixels.y);
			int numMarkers = markerIds.size();

			for (int i = 0; i < numMarkers; i++) {
                MarkerData markerData;

                // ID
                markerData.id = markerIds[i];

                // Corners
				std::vector<cv::Point2f> corners = markerCorners[i];
                markerData.topLeft[0] = (corners[0].x + trackingAreaOffset.x) / outputImage.cols;
                markerData.topLeft[1] = (corners[0].y + trackingAreaOffset.y) / outputImage.rows;

                markerData.topRight[0] = (corners[1].x + trackingAreaOffset.x) / outputImage.cols;
                markerData.topRight[1] = (corners[1].y + trackingAreaOffset.y) / outputImage.rows;

                markerData.bottomRight[0] = (corners[2].x + trackingAreaOffset.x) / outputImage.cols;
                markerData.bottomRight[1] = (corners[2].y + trackingAreaOffset.y) / outputImage.rows;

                markerData.bottomLeft[0] = (corners[3].x + trackingAreaOffset.x) / outputImage.cols;
                markerData.bottomLeft[1] = (corners[3].y + trackingAreaOffset.y) / outputImage.rows;

                // Center point
                cv::Point2f center;
				for (int j = 0; j < markerCorners[i].size(); j++) {
					center += corners[j];
				}
				center /= float(markerCorners[i].size());
                markerData.center[0] = (center.x + trackingAreaOffset.x) / outputImage.cols;
                markerData.center[1] = (center.y + trackingAreaOffset.y) / outputImage.rows;

                // Angle
				cv::Point2f pointA((corners[1] + corners[2]) * 0.5);
				float radius = cv::norm(pointA - center);
				cv::Point2f pointB(center + cv::Point2f(radius, 0));
				cv::Point2f pointN(center + cv::Point2f(0, -radius));
				cv::Vec2f vectorA(pointA - center);
				cv::Vec2f vectorB(pointB - center);
				cv::Vec2f vectorN(pointN - center);
				vectorA = cv::normalize(vectorA);
				vectorB = cv::normalize(vectorB);
				vectorN = cv::normalize(vectorN);

				double cosineNormal = vectorA.dot(vectorN);
				double signNormal = cosineNormal < 0 ? -1 : 1;
                markerData.angle = signNormal * kRadiansToDegrees * acos(vectorA.dot(vectorB));

                // Size
                // Normalized as a square area
                markerData.size = (4 * radius * radius) / (outputImage.cols * outputImage.rows);


                trackingData[markerData.id] = markerData;
			}
        }
    }

    {
        std::lock_guard<std::mutex> lockGuard(outputImageMutex);
        inputImage.copyTo(outputImage);
    }
    lastFrameNumber = currentFrameNumber;

    frameRateTimer.Update();
    executionTimer.Stop();
    //std::cout << "Detection processing: " << executionTimer.duration << " ms" << std::endl;
}

void MarkerDetection::CopyImageTo(cv::Mat& destinationImage)
{
    cv::Mat guiImage;
    {
        std::lock_guard<std::mutex> lockGuard(outputImageMutex);
        outputImage.copyTo(guiImage);
    }

    DrawGuides(guiImage);
    DrawMarkers(guiImage);
    guiImage.copyTo(destinationImage);
}

bool MarkerDetection::GenerateMarkerImages(int imageSize)
{
    std::lock_guard<std::mutex> lockGuard(detectorParametersMutex);

    bool isImagesSaved = false;
    cv::Mat markerImage;
    try {
        for (int i = 0; i < detectorParameters.markerDictionarySize; i++) {
            cv::aruco::drawMarker(markerDictionary, i, imageSize, markerImage, 1);
            cv::imwrite(cv::format("markers/marker-%d.png", i), markerImage);
        }
        isImagesSaved = true;
    }
    catch(cv::Exception& exception) {
        std::cout << "GenerateMarkerImages() Error: " << exception.what() << std::endl;
    }

    return isImagesSaved;
}

std::map<int, MarkerData> MarkerDetection::GetTrackingData()
{
    std::lock_guard<std::mutex> lockGuard(trackingDataMutex);
    return trackingData;
}

double MarkerDetection::GetFrameRate()
{
    return frameRateTimer.frameRate;
}

unsigned int MarkerDetection::GetFrameNumber()
{
    return currentFrameNumber;
}

void MarkerDetection::UpdateTrackingArea(cv::Rect2d trackingArea)
{
    std::lock_guard<std::mutex> lockGuard(trackingAreaMutex);
    this->trackingArea = trackingArea;
}

void MarkerDetection::UpdateDetectorParameters(DetectorParameterData detectorParameters)
{
    std::lock_guard<std::mutex> lockGuard(detectorParametersMutex);
	this->detectorParameters = detectorParameters;
}

void MarkerDetection::DrawGuides(cv::Mat &image)
{
    // Draw guide for center cross-hairs
    cv::Point imageCenter(image.cols / 2.0, image.rows / 2.0);
    cv::line(image, cv::Point(imageCenter.x - 50, imageCenter.y),
        cv::Point(imageCenter.x + 50, imageCenter.y), cv::Scalar(230, 216, 173), 3, cv::LINE_AA);
    cv::line(image, cv::Point(imageCenter.x, imageCenter.y - 50),
        cv::Point(imageCenter.x, imageCenter.y + 50), cv::Scalar(230, 216, 173), 3, cv::LINE_AA);

    // Draw guide for table tracking area
    cv::Rect2d trackingAreaGuide;
    {
        std::lock_guard<std::mutex> lockGuard(trackingAreaMutex);
        trackingAreaGuide = cv::Rect2d(trackingArea.x * image.cols, trackingArea.y * image.rows,
            trackingArea.width * image.cols, trackingArea.height * image.rows);
    }
    cv::rectangle(image, trackingAreaGuide, cv::Scalar(230, 216, 173), 3, cv::LINE_AA);
}

void MarkerDetection::DrawMarkers(cv::Mat &image)
{
    std::lock_guard<std::mutex> lockGuard(trackingDataMutex);
    // Draw the markers that are being tracked
    for (auto iter = trackingData.begin(); iter != trackingData.end(); iter++ ) {
        MarkerData markerData = iter->second;
        cv::Scalar color = ScalarHSV2BGR((markerData.id * 7), 255, 255);
        cv::line(image,
            cv::Point2f(markerData.topLeft[0] * image.cols, markerData.topLeft[1] * image.rows),
            cv::Point2f(markerData.topRight[0] * image.cols, markerData.topRight[1] * image.rows),
            color, 1, cv::LINE_AA);
        cv::line(image,
            cv::Point2f(markerData.topRight[0] * image.cols, markerData.topRight[1] * image.rows),
            cv::Point2f(markerData.bottomRight[0] * image.cols, markerData.bottomRight[1] * image.rows),
            color, 1, cv::LINE_AA);
        cv::line(image,
            cv::Point2f(markerData.bottomRight[0] * image.cols, markerData.bottomRight[1] * image.rows),
            cv::Point2f(markerData.bottomLeft[0] * image.cols, markerData.bottomLeft[1] * image.rows),
            color, 1, cv::LINE_AA);
        cv::line(image,
            cv::Point2f(markerData.bottomLeft[0] * image.cols, markerData.bottomLeft[1] * image.rows),
            cv::Point2f(markerData.topLeft[0] * image.cols, markerData.topLeft[1] * image.rows),
            color, 1, cv::LINE_AA);
        cv::circle(image,
            cv::Point2f(markerData.center[0] * image.cols, markerData.center[1] * image.rows),
            4, color, -1, cv::LINE_AA);
        cv::putText(image, cv::format("%d,%d,%d", markerData.id, int(markerData.angle), int(markerData.size)),
            cv::Point2f(markerData.center[0] * image.cols, markerData.center[1] * image.rows),
            cv::FONT_HERSHEY_SIMPLEX, 0.75,
            cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    }
}

cv::Scalar MarkerDetection::ScalarHSV2BGR(uchar H, uchar S, uchar V)
{
    cv::Mat rgb;
    cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(H, S, V));
    cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
    return cv::Scalar(rgb.data[0], rgb.data[1], rgb.data[2]);
}
