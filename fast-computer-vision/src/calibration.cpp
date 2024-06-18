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

#include "calibration.h"

Calibration::Calibration(Camera& camera) :
    camera(camera),
    isCaptureImage(false),
    isChessboardDetectedCurrent(false),
    isChessboardDetectedPrevious(false),
    isChessboardAcquired(false),
    isCalibrated(false),
    currentFrameNumber(0),
    lastFrameNumber(0),
    chessboardIntersections(24, 17),
    squareSize(1.0)
{
    camera.CopyImageTo(inputImage);
    {
        std::lock_guard<std::mutex> lockGuard(outputImageMutex);
        inputImage.copyTo(outputImage);
    }
}

Calibration::~Calibration()
{
}

void Calibration::Pause()
{
    frameRateTimer.Reset();
}

void Calibration::Run()
{
    currentFrameNumber = camera.GetFrameNumber();
    if (lastFrameNumber == currentFrameNumber) {
        return;
    }

    camera.CopyImageTo(inputImage);

    cv::Mat grayInputImage;
    cv::cvtColor(inputImage, grayInputImage, cv::COLOR_BGR2GRAY);
    cv::cvtColor(grayInputImage, inputImage, cv::COLOR_GRAY2BGR);

    std::vector<cv::Point2f> chessboardCorners;
    isChessboardDetectedCurrent = false;

    if (isCaptureImage) {
        std::lock_guard<std::mutex> lockGuard(calibrationParametersMutex);
        // More info about findChessboardCornersSB()
        // https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html#gadc5bcb05cb21cf1e50963df26986d7c9
        executionTimer.Start();
        if (isChessboardDetectedPrevious) {
            //std::cout << "Slow - ";
            isChessboardDetectedCurrent = findChessboardCornersSB(grayInputImage, chessboardIntersections, chessboardCorners,
                cv::CALIB_CB_EXHAUSTIVE | cv::CALIB_CB_NORMALIZE_IMAGE);
        }
        else {
            //std::cout << "Fast - ";
            isChessboardDetectedCurrent = findChessboardCorners(grayInputImage, chessboardIntersections, chessboardCorners,
                cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
        }
        executionTimer.Stop();
        //std::cout << "Checkerboard detection: " << executionTimer.duration << " ms" << std::endl;

        if (isChessboardDetectedCurrent && !isChessboardDetectedPrevious) {
            chessboardDetectionTimer.Start();
        }
        else if ((isChessboardDetectedCurrent && isChessboardDetectedPrevious)) {
            chessboardDetectionTimer.MeasureElapsedTime();

            if (chessboardDetectionTimer.duration > 2000) {
                capturedCorners.push_back(chessboardCorners);
                cv::Mat capturedImage;
                inputImage.copyTo(capturedImage);
                capturedImages.push_back(capturedImage);
                inputImage = cv::Scalar(0, 0, 0);
                isChessboardAcquired = true;

                emit NumImagesChanged(capturedImages.size());
                if (capturedImages.size() >= 12) {
                    emit MinimumImagesCaptured();
                }
            }
        }

        cv::drawChessboardCorners(inputImage, chessboardIntersections, cv::Mat(chessboardCorners), isChessboardDetectedCurrent);
        isChessboardDetectedPrevious = isChessboardDetectedCurrent;
    }

    {
        std::lock_guard<std::mutex> lockGuard(outputImageMutex);
        inputImage.copyTo(outputImage);
    }

    lastFrameNumber = currentFrameNumber;
    frameRateTimer.Update();

    if (isChessboardAcquired) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        isChessboardDetectedCurrent = false;
        isChessboardDetectedPrevious = false;
        isChessboardAcquired = false;
        isCaptureImage = false;
    }
}

void Calibration::UpdateCalibrationParameters(cv::Size chessboardIntersections, float squareSize)
{
    std::lock_guard<std::mutex> lockGuard(calibrationParametersMutex);
    this->chessboardIntersections = chessboardIntersections;
    this->squareSize = squareSize;
}

CalibrationLoadResult Calibration::LoadCalibration()
{
    CalibrationLoadResult loadResult = CalibrationLoadResult::Succeeded;
    cv::Size imageSize;

    if (!QFile("calibration.yml").exists()) {
        loadResult = CalibrationLoadResult::FileDoesNotExist;
        return loadResult;
    }

    try {
        cv::FileStorage fileStorage("calibration.yml", cv::FileStorage::READ);
        if (fileStorage.isOpened()) {
            fileStorage["imageSize"] >> imageSize;
            fileStorage["cameraMatrix"] >> calibrationData.cameraMatrix;
            fileStorage["distortionCoefficients"] >> calibrationData.distortionCoefficients;
         }
         fileStorage.release();
    }
    catch(cv::Exception& exception) {
        loadResult = CalibrationLoadResult::FileParseError;
        std::cout << "LoadCalibration() Error: " << exception.what() << std::endl;
    }

    if (loadResult == CalibrationLoadResult::Succeeded) {
        cv::initUndistortRectifyMap(
            calibrationData.cameraMatrix,
            calibrationData.distortionCoefficients, cv::Mat(),
            cv::getOptimalNewCameraMatrix(
                calibrationData.cameraMatrix,
                calibrationData.distortionCoefficients,
                imageSize, 1,
                imageSize, 0),
            imageSize, CV_16SC2,
            calibrationData.distortMap, calibrationData.undistortMap);

        calibrationData.type = CalibrationType::Saved;
        camera.Calibrate(calibrationData);
        camera.ToggleCalibration(true);
        isCalibrated = true;
    }

    return loadResult;
}

CalibrationSaveResult Calibration::SaveCalibration()
{
    CalibrationSaveResult saveResult = CalibrationSaveResult::Succeeded;

    // Save the calibration file
    try {
        cv::FileStorage fileStorage("calibration.yml", cv::FileStorage::WRITE);
        if (fileStorage.isOpened()) {
            fileStorage << "imageSize" << inputImage.size();
            fileStorage << "cameraMatrix" << calibrationData.cameraMatrix;
            fileStorage << "distortionCoefficients" << calibrationData.distortionCoefficients;
        }
        fileStorage.release();
    }
    catch(cv::Exception& exception) {
        saveResult = CalibrationSaveResult::CalibrationError;
        std::cout << "SaveCalibration() Calibration Error: " << exception.what() << std::endl;
    }

    // Then save the calibration images
    try {
        QDir calibrationDirectory("Calibration");
        if (!calibrationDirectory.exists()) {
            calibrationDirectory.mkpath(".");
        }

        for (int i = 0; i < capturedImages.size(); i++) {
            cv::imwrite(cv::format("%s/image_%02d.png",
                calibrationDirectory.dirName().toStdString().c_str(), i+1), capturedImages[i]);
        }
    }
    catch(cv::Exception& exception) {
        if (saveResult == CalibrationSaveResult::CalibrationError) {
            saveResult = CalibrationSaveResult::Failed;
        }
        else {
            saveResult = CalibrationSaveResult::ImagesError;
        }
        std::cout << "SaveCalibration() Images Error: " << exception.what() << std::endl;
    }

    calibrationData.type = CalibrationType::Saved;
    camera.Calibrate(calibrationData);
    camera.ToggleCalibration(true);
    isCalibrated = true;

    return saveResult;
}

void Calibration::CaptureImage()
{
    isCaptureImage = true;
}

void Calibration::CalibrateCamera()
{
    std::vector<cv::Point3f> chessboardCoordinates;
    {
        std::lock_guard<std::mutex> lockGuard(calibrationParametersMutex);

        for(int y = 0; y < chessboardIntersections.height; y++)
            for(int x = 0; x < chessboardIntersections.width; x++) {
                chessboardCoordinates.push_back(cv::Point3f(squareSize * x, squareSize * y, 0));
        }
    }

    capturedCoordinates.clear();
    for (int i = 0; i < capturedImages.size(); i++) {
        capturedCoordinates.push_back(chessboardCoordinates);
    }

    cv::Mat rotationMatrix, translationMatrix;
    double rmsError = cv::calibrateCamera(
        capturedCoordinates, capturedCorners, cv::Size(inputImage.cols, inputImage.rows),
        calibrationData.cameraMatrix, calibrationData.distortionCoefficients,
        rotationMatrix, translationMatrix);

    cv::initUndistortRectifyMap(
        calibrationData.cameraMatrix,
        calibrationData.distortionCoefficients, cv::Mat(),
        cv::getOptimalNewCameraMatrix(
            calibrationData.cameraMatrix,
            calibrationData.distortionCoefficients,
            inputImage.size(), 1,
            inputImage.size(), 0),
        inputImage.size(), CV_16SC2,
        calibrationData.distortMap, calibrationData.undistortMap);

    calibrationData.type = CalibrationType::Preview;
    camera.Calibrate(calibrationData);

    emit CalibrationDone(rmsError);
}

void Calibration::ClearImages()
{
    capturedCorners.clear();
    capturedImages.clear();
    emit NumImagesChanged(capturedImages.size());
}

bool Calibration::GetIsCalibrated()
{
    return isCalibrated;
}

void Calibration::CopyImageTo(cv::Mat& destinationImage)
{
    std::lock_guard<std::mutex> lockGuard(outputImageMutex);
    outputImage.copyTo(destinationImage);
}

double Calibration::GetFrameRate()
{
    return frameRateTimer.frameRate;
}

