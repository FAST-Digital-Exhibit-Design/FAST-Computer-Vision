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

    {
        std::lock_guard<std::mutex> lockGuard(trackingAreaMutex);
        trackingRectInPixels = cv::Rect2d(trackingAreaRect);
        trackingRectInPixels.x *= inputImage.cols;
        trackingRectInPixels.y *= inputImage.rows,
        trackingRectInPixels.width *= inputImage.cols;
        trackingRectInPixels.height *= inputImage.rows;

        trackingPointsInPixels = std::vector<cv::Point2d>(trackingAreaPoints);

        for(int i = 0; i < trackingPointsInPixels.size(); i++) {
            trackingPointsInPixels[i].x *= inputImage.cols;
            trackingPointsInPixels[i].y *= inputImage.rows;
        }
    }

    trackingImage = inputImage(trackingRectInPixels);
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
        drawingData.clear();

		if (isDetected) {
            cv::Point2f trackingAreaOffset(trackingRectInPixels.x, trackingRectInPixels.y);
			int numMarkers = markerIds.size();

			for (int i = 0; i < numMarkers; i++) {
                MarkerData markerData;
                MarkerData markerDrawingData;

                // ID
                markerData.id = markerIds[i];
                markerDrawingData.id = markerData.id;

                // Corners
				std::vector<cv::Point2f> corners = markerCorners[i];
                for (auto iter = corners.begin(); iter != corners.end(); iter++ ) {
                    iter->x += trackingAreaOffset.x;
                    iter->y += trackingAreaOffset.y;
                }


                cv::Point2f topLeftUV = InverseBilinearCoordinates(corners[0], trackingPointsInPixels);
                markerData.topLeft[0] = topLeftUV.x;
                markerData.topLeft[1] = topLeftUV.y;
                markerDrawingData.topLeft[0] = corners[0].x;
                markerDrawingData.topLeft[1] = corners[0].y;

                cv::Point2f topRightUV = InverseBilinearCoordinates(corners[1], trackingPointsInPixels);
                markerData.topRight[0] = topRightUV.x;
                markerData.topRight[1] = topRightUV.y;
                markerDrawingData.topRight[0] = corners[1].x;
                markerDrawingData.topRight[1] = corners[1].y;

                cv::Point2f bottomRightUV = InverseBilinearCoordinates(corners[2], trackingPointsInPixels);
                markerData.bottomRight[0] = bottomRightUV.x;
                markerData.bottomRight[1] = bottomRightUV.y;
                markerDrawingData.bottomRight[0] = corners[2].x;
                markerDrawingData.bottomRight[1] = corners[2].y;

                cv::Point2f bottomLeftUV = InverseBilinearCoordinates(corners[3], trackingPointsInPixels);
                markerData.bottomLeft[0] = bottomLeftUV.x;
                markerData.bottomLeft[1] = bottomLeftUV.y;
                markerDrawingData.bottomLeft[0] = corners[3].x;
                markerDrawingData.bottomLeft[1] = corners[3].y;

                // Center point
                cv::Point2f center;
				for (int j = 0; j < markerCorners[i].size(); j++) {
					center += corners[j];
				}
				center /= float(markerCorners[i].size());

                cv::Point2f centerUV = InverseBilinearCoordinates(corners[3], trackingPointsInPixels);
                markerData.center[0] = centerUV.x;
                markerData.center[1] = centerUV.y;
                markerDrawingData.center[0] = center.x;
                markerDrawingData.center[1] = center.y;

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
                markerDrawingData.angle = markerData.angle;

                // Size
                // Normalized as a square area
                markerData.size = (4 * radius * radius) / (outputImage.cols * outputImage.rows);
                markerDrawingData.size = markerData.size;

                trackingData[markerData.id] = markerData;
                drawingData[markerDrawingData.id] = markerDrawingData;
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

void MarkerDetection::UpdateTrackingArea(cv::Rect2d trackingAreaRect, std::vector<cv::Point2d> trackingAreaPoints)
{
    std::lock_guard<std::mutex> lockGuard(trackingAreaMutex);
    this->trackingAreaRect = trackingAreaRect;
    this->trackingAreaPoints = trackingAreaPoints;
}

void MarkerDetection::UpdateDetectorParameters(DetectorParameterData detectorParameters)
{
    std::lock_guard<std::mutex> lockGuard(detectorParametersMutex);
	this->detectorParameters = detectorParameters;
}

// The MIT License
// Copyright © 2014 Inigo Quilez
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org/
//
// Inverse bilinear interpolation: given four points defining a quadrilateral, compute the uv
// coordinates of any point in the plane that would give result to that point as a bilinear
// interpolation of the four points.
//
// The problem can be solved through a quadratic equation. More information in this article:
//
// https://iquilezles.org/articles/ibilinear
//
// Given a point p and a quad defined by four points {a,b,c,d}, return the bilinear
// coordinates of p in the quad. Will not be in the range [0..1]^2 if the point is
// outside the quad.
cv::Point2f MarkerDetection::InverseBilinearCoordinates(cv::Point2f point, std::vector<cv::Point2d> quadCorners)
{
    // A = top-left
    // B = top-right
    // C = bottom-right
    // D = bottom-left
    // X = tracked point in global 2D coordinates (x, y)
    //
    // P is the linear interpolation of A and B in u: P = A + (B-A)*u
    // Q is the linear interpolation of D and C in u: Q = D + (C-D)*u
    // X is the linear interpolation of P and Q in v: X = P + (Q-P)*v
    // X(u,v) = A + (B-A)*u + (D-A)*v + (A-B+C-D)*u*v
    //
    // E = B-A
    // F = D-A
    // G = A-B+C-D
    // H = X-A
    cv::Point2f result = cv::Point2f(-1.0, -1.0);

    if (quadCorners.size() < 4) {
        return result;
    }

    cv::Point2d topLeft = quadCorners[0];
    cv::Point2d topRight = quadCorners[1];
    cv::Point2d bottomRight = quadCorners[2];
    cv::Point2d bottomLeft = quadCorners[3];

    cv::Point2f E = topRight - topLeft;
    cv::Point2f F = bottomLeft - topLeft;
    cv::Point2f G = topLeft - topRight + bottomRight - bottomLeft;
    cv::Point2f H = point - cv::Point2f(topLeft);

    float k2 = G.cross(F);
    float k1 = E.cross(F) + H.cross(G);
    float k0 = H.cross(E);

    // if edges are parallel, this is a linear equation
    if (abs(k2) < 0.001) {
        float u = (H.x*k1 + F.x*k0) / (E.x*k1 - G.x*k0);
        float v = -k0/k1;
        result = cv::Point2f(u, v);
    }
    // otherwise, it's a quadratic
    else
    {
        float w = k1*k1 - 4.0*k0*k2;
        if (w < 0.0) {
            result = cv::Point2f(-1.0, -1.0);
            return result;
        }
        w = sqrt( w );

        float ik2 = 0.5/k2;
        float v = (-k1 - w)*ik2;
        float u = (H.x - H.x*v) / (E.x + G.x*v);

        if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0 ) {
           v = (-k1 + w)*ik2;
           u = (H.x - F.x*v) / (E.x + G.x*v);
        }
        result = cv::Point2f(u, v);
    }

    return result;
}

void MarkerDetection::DrawGuides(cv::Mat &image)
{
    // Draw guide for coordinates
    cv::Point coordinateCenter(30, 30);
    cv::line(image, cv::Point(coordinateCenter.x + 1, coordinateCenter.y),
        cv::Point(coordinateCenter.x + 30, coordinateCenter.y), cv::Scalar(0, 0, 255), 2);
    cv::putText(image, "X",
            cv::Point(coordinateCenter.x + 30 + 5, coordinateCenter.y + 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.5,
            cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    cv::line(image, cv::Point(coordinateCenter.x, coordinateCenter.y + 1),
        cv::Point(coordinateCenter.x, coordinateCenter.y + 30), cv::Scalar(0, 255, 0), 2);
    cv::putText(image, "Y",
            cv::Point(coordinateCenter.x - 5, coordinateCenter.y + 30 + 18),
            cv::FONT_HERSHEY_SIMPLEX, 0.5,
            cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    //cv::circle(image, coordinateCenter, 2, cv::Scalar(255, 255, 255), -1);

    // Draw guide for center cross-hairs
    cv::Point imageCenter(image.cols / 2.0, image.rows / 2.0);
    cv::line(image, cv::Point(imageCenter.x - 50, imageCenter.y),
        cv::Point(imageCenter.x + 50, imageCenter.y), cv::Scalar(255, 255, 255), 2);
    cv::line(image, cv::Point(imageCenter.x, imageCenter.y - 50),
        cv::Point(imageCenter.x, imageCenter.y + 50), cv::Scalar(255, 255, 255), 2);
    cv::putText(image, "Center",
        imageCenter + cv::Point(-25, 75),
        cv::FONT_HERSHEY_SIMPLEX, 0.5,
        cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

    // Draw guide for table tracking area
    cv::Rect2d trackingGuideRect;
    std::vector<cv::Point2d> trackingGuidePoints;
    {
        std::lock_guard<std::mutex> lockGuard(trackingAreaMutex);
        trackingGuideRect = cv::Rect2d(trackingRectInPixels);
        trackingGuidePoints = std::vector<cv::Point2d>(trackingPointsInPixels);
    }

    cv::Point2d rectTL = trackingGuideRect.tl();
    cv::Point2d rectTR = cv::Point2d(trackingGuideRect.br().x, trackingGuideRect.tl().y);
    cv::Point2d rectBR = trackingGuideRect.br();
    cv::Point2d rectBL = cv::Point2d(trackingGuideRect.tl().x, trackingGuideRect.br().y);

    cv::line(image, rectTL, rectTL + cv::Point2d(20, 0), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
    cv::line(image, rectTL, rectTL + cv::Point2d(0, 20), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
    cv::line(image, rectTR, rectTR - cv::Point2d(20, 0), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
    cv::line(image, rectTR, rectTR + cv::Point2d(0, 20), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);

    cv::line(image, rectBR, rectBR - cv::Point2d(20, 0), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
    cv::line(image, rectBR, rectBR - cv::Point2d(0, 20), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
    cv::line(image, rectBL, rectBL + cv::Point2d(20, 0), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
    cv::line(image, rectBL, rectBL - cv::Point2d(0, 20), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);

    //cv::rectangle(image, trackingGuideRect, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);

    for(int i = 0; i < trackingGuidePoints.size(); i++) {
        int j = (i + 1) % trackingGuidePoints.size();
        cv::line(image, trackingGuidePoints[i], trackingGuidePoints[j], cv::Scalar(255, 255, 0), 2, cv::LINE_AA);
        cv::circle(image, trackingGuidePoints[i], 4, cv::Scalar(255, 255, 0), -1, cv::LINE_AA);

        cv::Point2d offset = cv::Point2d(trackingGuidePoints[i].x > imageCenter.x ? -4 : 1,
            trackingGuidePoints[i].y > imageCenter.y ? -1 : 1);
        offset *= 30;
        cv::putText(image, CornerNames[i],
            trackingGuidePoints[i] + offset,
            cv::FONT_HERSHEY_SIMPLEX, 0.5,
            cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    }
}

void MarkerDetection::DrawMarkers(cv::Mat &image)
{
    std::lock_guard<std::mutex> lockGuard(trackingDataMutex);
    // Draw the markers that are being tracked
    for (auto iter = drawingData.begin(); iter != drawingData.end(); iter++ ) {
        MarkerData markerData = iter->second;

        // H -> 0-180
        //cv::Scalar color = ScalarHSV2BGR((markerData.id * 7), 255, 255);
        cv::Scalar color = cv::Scalar(255, 0, 255);
        cv::line(image,
            cv::Point2f(markerData.topLeft[0], markerData.topLeft[1]),
            cv::Point2f(markerData.topRight[0], markerData.topRight[1]),
            color, 1, cv::LINE_AA);
        cv::line(image,
            cv::Point2f(markerData.topRight[0], markerData.topRight[1]),
            cv::Point2f(markerData.bottomRight[0], markerData.bottomRight[1]),
            color, 1, cv::LINE_AA);
        cv::line(image,
            cv::Point2f(markerData.bottomRight[0], markerData.bottomRight[1]),
            cv::Point2f(markerData.bottomLeft[0], markerData.bottomLeft[1]),
            color, 1, cv::LINE_AA);
        cv::line(image,
            cv::Point2f(markerData.bottomLeft[0], markerData.bottomLeft[1]),
            cv::Point2f(markerData.topLeft[0], markerData.topLeft[1]),
            color, 1, cv::LINE_AA);
        cv::circle(image,
            cv::Point2f(markerData.center[0], markerData.center[1]),
            3, color, -1, cv::LINE_AA);
//        cv::putText(image, cv::format("%d,%d,%d", markerData.id, int(markerData.angle), int(markerData.size)),
//            cv::Point2f(markerData.center[0], markerData.center[1]),
//            cv::FONT_HERSHEY_SIMPLEX, 0.75,
//            cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
        cv::putText(image, cv::format("%d", markerData.id),
            cv::Point2f(markerData.center[0], markerData.center[1]),
            cv::FONT_HERSHEY_SIMPLEX, 0.5,
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
