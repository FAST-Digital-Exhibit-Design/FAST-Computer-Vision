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

#include "NetworkCommunication.h"

NetworkCommunication::NetworkCommunication(MarkerDetection& markerDetection) :
    markerDetection(markerDetection),
    currentFrameNumber(0),
    lastFrameNumber(0)
{
}

NetworkCommunication::~NetworkCommunication()
{
}

void NetworkCommunication::Pause()
{
    frameRateTimer.Reset();
}

void NetworkCommunication::Run()
{
    currentFrameNumber = markerDetection.GetFrameNumber();
    if (lastFrameNumber == currentFrameNumber) {
        return;
    }

    executionTimer.Start();

    trackingData.clear();
    trackingData = markerDetection.GetTrackingData();

    try {
        QByteArray byteArray;

        byteArray.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&currentFrameNumber), sizeof(unsigned int)));

        unsigned int numMarkers = trackingData.size();
        byteArray.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&numMarkers), sizeof(unsigned int)));

        for (auto iter = trackingData.begin(); iter != trackingData.end(); iter++ ) {
            MarkerData markerData = iter->second;
            byteArray.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&markerData.id), sizeof(int)));
            byteArray.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&markerData.center[0]), sizeof(float)));
            byteArray.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&markerData.center[1]), sizeof(float)));
            byteArray.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&markerData.angle), sizeof(float)));
            byteArray.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&markerData.size), sizeof(float)));
        }

        // QSocket must created in the thread where it will be used
        QUdpSocket socket;
        {
            // Prevent changes to the UDP parameters when sending data
            std::lock_guard<std::mutex> lockGuard(udpParametersMutex);
            socket.writeDatagram(byteArray, address, port);
        }

        lastFrameNumber = currentFrameNumber;
    }
    catch(...) {}

    frameRateTimer.Update();
    executionTimer.Stop();
    //std::cout << "Network communication: " << executionTimer.duration << " ms" << std::endl;
}

void NetworkCommunication::UpdateUdpParameters(QHostAddress address, uint port)
{
    std::lock_guard<std::mutex> lockGuard(udpParametersMutex);
    this->address = address;
    this->port = port;
}

double NetworkCommunication::GetFrameRate()
{
    return frameRateTimer.frameRate;
}
