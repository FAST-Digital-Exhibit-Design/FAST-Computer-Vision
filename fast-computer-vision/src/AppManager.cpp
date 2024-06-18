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

#include "AppManager.h"

AppManager::AppManager() :
    calibration(camera),
    markerDetection(camera),
    networkCommunication(markerDetection)
{
    isRunning = true;
    processingThread = std::thread(&AppManager::RunProcessingThread, this);
}

AppManager::~AppManager()
{
    isRunning = false;
    processingThread.join();
}

void AppManager::RunProcessingThread()
{
    while (isRunning) {
        camera.Run();

        if (camera.GetIsConnected()) {
            if (mode == AppMode::Calibration) {
                markerDetection.Pause();
                networkCommunication.Pause();
                calibration.Run();
            }
            else if (mode == AppMode::Tracking) {
                markerDetection.Run();
                networkCommunication.Run();
                calibration.Pause();
            }
        }
        else {
            markerDetection.Pause();
            networkCommunication.Pause();
            calibration.Pause();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

AppMode AppManager::GetMode()
{
    return mode;
}

void AppManager::SetMode(AppMode mode)
{
    this->mode = mode;
}

