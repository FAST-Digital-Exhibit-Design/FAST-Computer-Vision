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

#include "FrameRateTimer.h"

FrameRateTimer::FrameRateTimer() :
    frameRate(0)
{
    startTime = std::chrono::high_resolution_clock::now();
}
void FrameRateTimer::Update()
{
    MeasureElapsedTime();
    frameRate = (.9 * frameRate) + (.1 * 1000.0 / duration);
}
void FrameRateTimer::Reset()
{
    MeasureElapsedTime();
    frameRate = 0;
}
void FrameRateTimer::MeasureElapsedTime()
{
    endTime = std::chrono::high_resolution_clock::now();
    elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    duration = elapsedTime.count();
    startTime = endTime;
}
