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

#include "Settings.h"

Settings::Settings() : file("settings.xml") { }
Settings::~Settings() { }

bool Settings::Load()
{
    if (!file.exists()) {
        emit RequestSave();
        return false;
    }

    file.open(QIODevice::ReadOnly);

    QXmlStreamReader xmlReader(&file);
    QString elementName;
    QString elementText;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();

        if (xmlReader.isStartElement()) {
            elementName = xmlReader.name().toString();
            elementText = "";
        }
        else if (xmlReader.isCharacters() && !xmlReader.isWhitespace()) {
            elementText = xmlReader.text().toString();
            Parse(elementName, elementText);
        }
    }
    if (xmlReader.hasError()) {
        QString text = "<b>An error occurred trying to load settings.xml</b>";
        QString informativeText = "The settings file with the error will be moved to the SettingsError "
            "folder and a new settings file will be written with default values.\n\n" + xmlReader.errorString();
        file.close();

        QDir errorDirectory("SettingsError");
        if (!errorDirectory.exists()) {
            errorDirectory.mkpath(".");
        }

        QString date = QDateTime::currentDateTime().date().toString(Qt::ISODate).remove("-");
        QString time = QDateTime::currentDateTime().time().toString().remove(":");
        QFile::copy("settings.xml", errorDirectory.dirName() + "/settings-" + date + "-" + time + ".xml");

        emit Error(text, informativeText);
        emit RequestSave();
        return false;
    }

    file.close();
    return true;
}

void Settings::Save()
{
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("ApplicationSettings");

    xmlWriter.writeTextElement("startMinimized", QString::number(startMinimized));

    xmlWriter.writeComment("Basic settings");

    xmlWriter.writeTextElement("networkIpAddress", networkIpAddress);

    xmlWriter.writeTextElement("networkPort", QString::number(networkPort));

    xmlWriter.writeTextElement("gamma", QString::number(gamma));
    xmlWriter.writeTextElement("rotate", QString::number(rotate));

    xmlWriter.writeTextElement("trackingAreaX", QString::number(trackingAreaX));
    xmlWriter.writeTextElement("trackingAreaY", QString::number(trackingAreaY));
    xmlWriter.writeTextElement("trackingAreaWidth", QString::number(trackingAreaWidth));
    xmlWriter.writeTextElement("trackingAreaHeight", QString::number(trackingAreaHeight));

    xmlWriter.writeComment("Advanced settings");

    xmlWriter.writeTextElement("checkerboardHorizontal", QString::number(checkerboardHorizontal));
    xmlWriter.writeTextElement("checkerboardVertical", QString::number(checkerboardVertical));
    xmlWriter.writeTextElement("checkerboardSquareSize", QString::number(checkerboardSquareSize));

    xmlWriter.writeTextElement("markerNumBits", QString::number(markerNumBits));
    xmlWriter.writeTextElement("markerDictionarySize", QString::number(markerDictionarySize));
    xmlWriter.writeTextElement("markerImageSize", QString::number(markerImageSize));

    xmlWriter.writeTextElement("adaptiveThreshWinSizeMin", QString::number(adaptiveThreshWinSizeMin));
    xmlWriter.writeTextElement("adaptiveThreshWinSizeMax", QString::number(adaptiveThreshWinSizeMax));
    xmlWriter.writeTextElement("adaptiveThreshWinSizeStep", QString::number(adaptiveThreshWinSizeStep));
    xmlWriter.writeTextElement("adaptiveThreshConstant", QString::number(adaptiveThreshConstant));

    xmlWriter.writeTextElement("minMarkerPerimeterRate", QString::number(minMarkerPerimeterRate));
    xmlWriter.writeTextElement("maxMarkerPerimeterRate", QString::number(maxMarkerPerimeterRate));
    xmlWriter.writeTextElement("polygonalApproxAccuracyRate", QString::number(polygonalApproxAccuracyRate));
    xmlWriter.writeTextElement("minCornerDistanceRate", QString::number(minCornerDistanceRate));
    xmlWriter.writeTextElement("minMarkerDistanceRate", QString::number(minMarkerDistanceRate));

    xmlWriter.writeTextElement("minDistanceToBorder", QString::number(minDistanceToBorder));
    xmlWriter.writeTextElement("markerBorderBits", QString::number(markerBorderBits));
    xmlWriter.writeTextElement("minOtsuStdDev", QString::number(minOtsuStdDev));
    xmlWriter.writeTextElement("perspectiveRemovePixelPerCell", QString::number(perspectiveRemovePixelPerCell));
    xmlWriter.writeTextElement("perspectiveRemoveIgnoredMarginPerCell", QString::number(perspectiveRemoveIgnoredMarginPerCell));

    xmlWriter.writeTextElement("maxErroneousBitsInBorderRate", QString::number(maxErroneousBitsInBorderRate));
    xmlWriter.writeTextElement("errorCorrectionRate", QString::number(errorCorrectionRate));

    xmlWriter.writeEndElement(); // ApplicationSettings

    xmlWriter.writeEndDocument();

    if (xmlWriter.hasError()) {
        QString text = "<b>An error occurred trying to save settings.xml</b>";
        QString informativeText = "The content may not be formatted properly or you may not have permission to write the file.";
        emit Error(text, informativeText);
    }

    file.close();
}

void Settings::Parse(QString name, QString text)
{
    if (name == "startMinimized") {
        startMinimized = text.toInt();
    }

    else if (name == "networkIpAddress") {
        networkIpAddress = text;
    }
    else if (name == "networkPort") {
        networkPort = text.toInt();
    }

    else if (name == "gamma") {
        gamma = text.toDouble();
    }
    else if (name == "rotate") {
        rotate = text.toInt();
    }

    else if (name == "trackingAreaX") {
        trackingAreaX = text.toDouble();
    }
    else if (name == "trackingAreaY") {
        trackingAreaY = text.toDouble();
    }
    else if (name == "trackingAreaWidth") {
        trackingAreaWidth = text.toDouble();
    }
    else if (name == "trackingAreaHeight") {
        trackingAreaHeight = text.toDouble();
    }

    else if (name == "checkerboardHorizontal") {
        checkerboardHorizontal = text.toInt();
    }
    else if (name == "checkerboardVertical") {
        checkerboardVertical = text.toInt();
    }
    else if (name == "checkerboardSquareSize") {
        checkerboardSquareSize = text.toDouble();
    }

    else if (name == "markerNumBits") {
        markerNumBits = text.toInt();
    }
    else if (name == "markerDictionarySize") {
        markerDictionarySize = text.toInt();
    }
    else if (name == "markerImageSize") {
        markerImageSize = text.toInt();
    }

    else if (name == "adaptiveThreshWinSizeMin") {
        adaptiveThreshWinSizeMin = text.toInt();
    }
    else if (name == "adaptiveThreshWinSizeMax") {
        adaptiveThreshWinSizeMax = text.toInt();
    }
    else if (name == "adaptiveThreshWinSizeStep") {
        adaptiveThreshWinSizeStep = text.toInt();
    }
    else if (name == "adaptiveThreshConstant") {
        adaptiveThreshConstant = text.toInt();
    }

    else if (name == "minMarkerPerimeterRate") {
        minMarkerPerimeterRate = text.toDouble();
    }
    else if (name == "maxMarkerPerimeterRate") {
        maxMarkerPerimeterRate = text.toDouble();
    }
    else if (name == "polygonalApproxAccuracyRate") {
        polygonalApproxAccuracyRate = text.toDouble();
    }
    else if (name == "minCornerDistanceRate") {
        minCornerDistanceRate = text.toDouble();
    }
    else if (name == "minMarkerDistanceRate") {
        minMarkerDistanceRate = text.toDouble();
    }

    else if (name == "minDistanceToBorder") {
        minDistanceToBorder = text.toInt();
    }
    else if (name == "markerBorderBits") {
        markerBorderBits = text.toInt();
    }
    else if (name == "minOtsuStdDev") {
        minOtsuStdDev = text.toDouble();
    }
    else if (name == "perspectiveRemovePixelPerCell") {
        perspectiveRemovePixelPerCell = text.toInt();
    }
    else if (name == "perspectiveRemoveIgnoredMarginPerCell") {
        perspectiveRemoveIgnoredMarginPerCell = text.toDouble();
    }

    else if (name == "maxErroneousBitsInBorderRate") {
        maxErroneousBitsInBorderRate = text.toDouble();
    }
    else if (name == "errorCorrectionRate") {
        errorCorrectionRate = text.toDouble();
    }
}

