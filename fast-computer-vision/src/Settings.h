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
#include <QObject>
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDateTime>

class Settings : public QObject
{
    Q_OBJECT

public:
    Settings();
    ~Settings();

    void Save();
    bool Load();

    bool startMinimized = false;

    QString networkIpAddress = "255.255.255.255";
    uint networkPort = 50000;

    double gamma = 0.5;
    bool rotate = false;

    double trackingAreaX = 0;
    double trackingAreaY = 0;
    double trackingAreaWidth = 1;
    double trackingAreaHeight = 1;

    int checkerboardHorizontal = 25;
    int checkerboardVertical = 18;
    double checkerboardSquareSize = 30;

    int markerNumBits = 4;
    int markerDictionarySize = 24;
    int markerImageSize = 300;

    int adaptiveThreshWinSizeMin = 3;
    int adaptiveThreshWinSizeMax = 23;
    int adaptiveThreshWinSizeStep = 10;
    double adaptiveThreshConstant = 10;

    double minMarkerPerimeterRate = 0.02;
    double maxMarkerPerimeterRate = 2;
    double polygonalApproxAccuracyRate = 0.1;
    double minCornerDistanceRate = 0.05;
    double minMarkerDistanceRate = 0.05;
    int minDistanceToBorder = 3;

    int markerBorderBits = 1;
    double minOtsuStdDev = 5;
    int perspectiveRemovePixelPerCell = 8;
    double perspectiveRemoveIgnoredMarginPerCell = 0.25;

    double maxErroneousBitsInBorderRate = 0.35;
    double errorCorrectionRate = 0.6;

signals:
    void Error(QString text, QString informativeText);
    void RequestSave();

private:
    void Parse(QString name, QString text);

    QFile file;
};
