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
#include "Camera.h"
#include "FrameRateTimer.h"
#include "ExecutionTimer.h"
#include <QObject>
#include <QProgressDialog>
#include <QDir>
#include <QFile>

enum class CalibrationLoadResult {Succeeded, FileDoesNotExist, FileParseError};
enum class CalibrationSaveResult {Succeeded, CalibrationError, ImagesError, Failed};

class Calibration : public QObject
{
    Q_OBJECT

public:
    Calibration(Camera& camera);
    ~Calibration();
    void Run();
    void Pause();
    CalibrationLoadResult LoadCalibration();
    CalibrationSaveResult SaveCalibration();
    void CaptureImage();
    void CalibrateCamera(); //TODO should this return bool?
    void ClearImages();
    bool GetIsCalibrated();
    void CopyImageTo(cv::Mat& destinationImage);
    double GetFrameRate();

    void UpdateCalibrationParameters(cv::Size chessboardIntersections, float squareSize);

signals:
    void NumImagesChanged(int numImages);
    void MinimumImagesCaptured();
    void ComputeCalibrationProgress(int progress);
    void SaveCalibrationProgress(int progress);
    void CalibrationDone(double rmsError);


private:
    Camera & camera;
    cv::Mat inputImage;
    cv::Mat outputImage;

    bool isCaptureImage;
    bool isChessboardDetectedCurrent;
    bool isChessboardDetectedPrevious;
    bool isChessboardAcquired;
    bool isCalibrated;

    unsigned int currentFrameNumber;
    unsigned int lastFrameNumber;

    cv::Size chessboardIntersections;
    float squareSize;

    std::vector<std::vector<cv::Point3f>> capturedCoordinates;
    std::vector<std::vector<cv::Point2f>> capturedCorners;
    std::vector<cv::Mat> capturedImages;

    CalibrationData calibrationData;

    std::mutex calibrationParametersMutex;
    std::mutex outputImageMutex;

    FrameRateTimer frameRateTimer;
    ExecutionTimer chessboardDetectionTimer;

    ExecutionTimer executionTimer;
};
