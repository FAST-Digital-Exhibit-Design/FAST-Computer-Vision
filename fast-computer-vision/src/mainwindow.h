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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFile>
#include <QIODevice>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTime>
#include <QDir>
#include <QDesktopServices>
#include <QMessageBox>
#include "pch.h"
#include "MarkdownViewerWindow.h"
#include "AppManager.h"
#include "Settings.h"
#include "FrameRateTimer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MarkdownViewerWindow *aboutWindow;
    MarkdownViewerWindow *userManualWindow;
    AppManager manager;
    Settings settings;

    FrameRateTimer frameRateTimer;
    QTimer timer;

private slots:
    void UpdateUi();
    void OnCameraConnected();
    void OnCameraDisconnected();

    void UpdateMode(int modeIndex);
    void UpdateWindowParameters();
    void UpdateUdpParameters();
    void UpdateTrackingArea();
    void UpdateCameraParameters();
    void UpdateCalibrationParameters();
    void UpdateDetectorParameters();
    void GenerateMarkerImages();
    void OpenCalibrationImages();
    void OnStartCalibration();

    void LoadSettings();
    void SaveSettings();
    void OnSettingsError(QString text, QString informativeText);

    void CaptureCalibrationImage();
    void UpdateImageCaptureProgress(int numImages);
    void EnableStep3Calibrate();
    void CalibrateCamera();
    void EnableStep4ReviewAndSave(double rmsError);
    void SaveCalibration();
    void CancelCalibration();

    void ToggleCalibration(bool isChecked);
    void ToggleCalibrationPreview(bool isChecked);
};
#endif // MAINWINDOW_H
