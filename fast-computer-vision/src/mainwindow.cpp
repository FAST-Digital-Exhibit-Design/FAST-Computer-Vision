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

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , aboutWindow(new MarkdownViewerWindow(this))
    , userManualWindow(new MarkdownViewerWindow(this))
{
    ui->setupUi(this);

    aboutWindow->SetTextFile("Documentation/About.md");
    userManualWindow->SetTextFile("Documentation/UserManual.md");

    // Menu buttons
    connect(ui->actionAbout, &QAction::triggered, aboutWindow, &MarkdownViewerWindow::show);
    connect(ui->actionUserManual, &QAction::triggered, userManualWindow, &MarkdownViewerWindow::show);

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::UpdateMode);

    // --- Tracking Mode ---
    //

    // UI updates
    connect(&manager.camera, &Camera::CameraConnected, this, &MainWindow::OnCameraConnected);
    connect(&manager.camera, &Camera::CameraDisconnected, this, &MainWindow::OnCameraDisconnected);

    // Basic Settings
    //
    // Network UDP settings
    connect(ui->spinBox_ip_1, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateUdpParameters);
    connect(ui->spinBox_ip_2, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateUdpParameters);
    connect(ui->spinBox_ip_3, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateUdpParameters);
    connect(ui->spinBox_ip_4, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateUdpParameters);
    connect(ui->spinBox_port, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateUdpParameters);

    // Tracking area settings
    connect(ui->doubleSpinBox_trackingAreaX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::UpdateTrackingArea);
    connect(ui->doubleSpinBox_trackingAreaY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::UpdateTrackingArea);
    connect(ui->doubleSpinBox_trackingAreaWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::UpdateTrackingArea);
    connect(ui->doubleSpinBox_trackingAreaHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::UpdateTrackingArea);

    // Marker settings
    connect(ui->spinBox_markerDictionarySize, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->spinBox_markerNumBits, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->spinBox_markerImageSize, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);

    connect(ui->pushButton_generateMarkers, &QPushButton::pressed,
            this, &MainWindow::GenerateMarkerImages);

    // Image correction settings
    connect(ui->doubleSpinBox_gamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::UpdateCameraParameters);
    connect(ui->radioButton_rotate180, QOverload<bool>::of(&QRadioButton::toggled),
         this, &MainWindow::UpdateCameraParameters);

    // Camera calibration settings
    connect(ui->checkBox_toggleCalibration, &QCheckBox::toggled, this, &MainWindow::ToggleCalibration);
    connect(ui->pushButton_viewCalibrationImages, &QPushButton::pressed, this, &MainWindow::OpenCalibrationImages);
    connect(ui->pushButton_createNewCalibration, &QPushButton::pressed, this, &MainWindow::OnStartCalibration);

    // Advanced Settings
    //
    // Marker detector settings
    connect(ui->spinBox_adaptiveThreshWinSizeMin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->spinBox_adaptiveThreshWinSizeMax, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->spinBox_adaptiveThreshWinSizeStep, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->spinBox_adaptiveThreshConstant, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);

    connect(ui->doubleSpinBox_minMarkerPerimeterRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->doubleSpinBox_maxMarkerPerimeterRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->doubleSpinBox_polygonalApproxAccuracyRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->doubleSpinBox_minCornerDistanceRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->doubleSpinBox_minMarkerDistanceRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->spinBox_minDistanceToBorder, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);

    connect(ui->spinBox_markerBorderBits, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->doubleSpinBox_minOtsuStdDev, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->spinBox_perspectiveRemovePixelPerCell, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->doubleSpinBox_perspectiveRemoveIgnoredMarginPerCell, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);

    connect(ui->doubleSpinBox_maxErroneousBitsInBorderRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);
    connect(ui->doubleSpinBox_errorCorrectionRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateDetectorParameters);

    // Settings save/cancel buttons
    connect(ui->pushButton_saveSettings, &QPushButton::pressed, this, &MainWindow::SaveSettings);
    connect(ui->pushButton_loadSettings, &QPushButton::pressed, this, &MainWindow::LoadSettings);
    connect(&settings, &Settings::Error, this, &MainWindow::OnSettingsError);
    connect(&settings, &Settings::RequestSave, this, &MainWindow::SaveSettings);

    // --- Camera Calibration Mode ---
    //

    // UI updates
    connect(&manager.calibration, &Calibration::NumImagesChanged, this, &MainWindow::UpdateImageCaptureProgress);
    connect(&manager.calibration, &Calibration::MinimumImagesCaptured, this, &MainWindow::EnableStep3Calibrate);
    connect(&manager.calibration, &Calibration::CalibrationDone, this, &MainWindow::EnableStep4ReviewAndSave);

    // Calibration controls
    connect(ui->pushButton_captureImage, &QPushButton::pressed, this, &MainWindow::CaptureCalibrationImage);
    connect(ui->pushButton_calibrate, &QPushButton::pressed, this, &MainWindow::CalibrateCamera);
    connect(ui->checkBox_toggleCalibrationPreview, &QCheckBox::toggled, this, &MainWindow::ToggleCalibrationPreview);

    // Calibration save/cancel buttons
    connect(ui->pushButton_saveCalibration, &QPushButton::pressed, this, &MainWindow::SaveCalibration);
    connect(ui->pushButton_cancelCalibration, &QPushButton::pressed, this, &MainWindow::CancelCalibration);

    // --- Initialization ---
    //
    ui->groupBox_step3->setEnabled(false);
    ui->pushButton_calibrate->setEnabled(false);

    ui->groupBox_step4->setEnabled(false);
    ui->checkBox_toggleCalibrationPreview->setEnabled(false);
    ui->checkBox_toggleCalibrationPreview->setChecked(false);
    ui->label_rmsCalibrationError->setEnabled(false);
    ui->label_rmsCalibrationError->setText("");

    // Make sure the application starts in tracking mode and is initialize properly.
    // Setting just the stacked widget isn't enough because it doesn't send a signal unless
    // the stack index is different. But setting the mode through the app manager is enough either
    // because it can't change the UI of the stacked widget. So setting both is required.
    ui->stackedWidget->setCurrentIndex(0);
    UpdateMode((int)AppMode::Tracking);

    LoadSettings();
    UpdateUdpParameters();
    UpdateTrackingArea();
    UpdateCameraParameters();
    UpdateCalibrationParameters();
    UpdateDetectorParameters();

    // These connections need to happen after settings have loaded to avoid overwriting existing settings values
    // because they will auto-save when the UI control value changes.
    // Window Settings
    connect(ui->checkBox_startMinimized, &QCheckBox::toggled, this, &MainWindow::UpdateWindowParameters);
    // Calibration controls
    connect(ui->spinBox_checkerboardHorizontal, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateCalibrationParameters);
    connect(ui->spinBox_checkerboardVertical, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::UpdateCalibrationParameters);
    connect(ui->doubleSpinBox_checkerboardSquare, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::UpdateCalibrationParameters);

    manager.calibration.LoadCalibration();
    ui->checkBox_toggleCalibration->setEnabled(manager.calibration.GetIsCalibrated());
    ui->checkBox_toggleCalibration->setChecked(manager.calibration.GetIsCalibrated());

    ui->pushButton_saveSettings->setEnabled(false);
    ui->pushButton_loadSettings->setEnabled(false);

    connect(&timer, &QTimer::timeout, this, &MainWindow::UpdateUi);

    // Run at ~10fps so that GUI updates don't take up much CPU
    timer.start(100);

    if (settings.startMinimized) {
        setWindowState(Qt::WindowMinimized);
    }
}

MainWindow::~MainWindow()
{
    timer.stop();

    delete ui;
}

void MainWindow::OnCameraConnected()
{
    cv::Size cameraResolution = manager.camera.GetResolution();
    ui->label_camera_resolution->setText(QString("%1 x %2").arg(
        QString::number(cameraResolution.width), QString::number(cameraResolution.height)));
}

void MainWindow::OnCameraDisconnected()
{
    ui->label_view_image->setText("Connecting to camera...");
    ui->label_camera_resolution->setText("#### x ####");
}

void MainWindow::UpdateUi()
{
    // Update camera view image
    if (manager.camera.GetIsConnected()) {
        cv::Mat cameraImage;
        if (manager.GetMode() == AppMode::Tracking) {
            manager.markerDetection.CopyImageTo(cameraImage);
        }
        else if (manager.GetMode() == AppMode::Calibration) {
            manager.calibration.CopyImageTo(cameraImage);
        }

        QPixmap cameraPixmap = QPixmap::fromImage(
            QImage(cameraImage.data, cameraImage.cols, cameraImage.rows, cameraImage.step, QImage::Format_BGR888));
        QPixmap viewPixmap = cameraPixmap.scaled(ui->label_view_image->size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_view_image->setPixmap(viewPixmap);
    }

    // Update framerates
    ui->label_camera_fps->setText(QString::number(manager.camera.GetFrameRate(), 'f', 1));

    if (manager.GetMode() == AppMode::Tracking) {
        ui->label_detection_fps->setText(QString::number(manager.markerDetection.GetFrameRate(), 'f', 1));
    }
    else if (manager.GetMode() == AppMode::Calibration) {
        ui->label_detection_fps->setText(QString::number(manager.calibration.GetFrameRate(), 'f', 1));
    }

    ui->label_network_fps->setText(QString::number(manager.networkCommunication.GetFrameRate(), 'f', 1));
    ui->label_ui_fps->setText(QString::number(frameRateTimer.frameRate, 'f', 1));

    frameRateTimer.Update();
}

void MainWindow::UpdateMode(int modeIndex)
{
    AppMode mode = (AppMode)modeIndex;
    manager.SetMode(mode);

    if (mode == AppMode::Tracking) {
        ui->label_mode->setText("Tracking Mode");
        manager.camera.ToggleRotation(true);
    }
    else if (mode == AppMode::Calibration) {
        ui->label_mode->setText("Calibration Mode");
        manager.camera.ToggleRotation(false);
    }
}

void MainWindow::UpdateWindowParameters()
{
    settings.startMinimized = ui->checkBox_startMinimized->isChecked();
    settings.Save();
}

void MainWindow::UpdateUdpParameters()
{
    QString hostname = QString("%1.%2.%3.%4").arg(
        QString::number(ui->spinBox_ip_1->value()),
        QString::number(ui->spinBox_ip_2->value()),
        QString::number(ui->spinBox_ip_3->value()),
        QString::number(ui->spinBox_ip_4->value()));

    uint port = ui->spinBox_port->value();

    manager.networkCommunication.UpdateUdpParameters(QHostAddress(hostname), port);

    ui->pushButton_saveSettings->setEnabled(true);
    ui->pushButton_loadSettings->setEnabled(true);
}

void MainWindow::UpdateTrackingArea()
{
    // x,y are in the range of [0, 1]
    double x = ui->doubleSpinBox_trackingAreaX->value();
    double y = ui->doubleSpinBox_trackingAreaY->value();
    double maxWidth = 1.0 - x;
    double maxHeight = 1.0 - y;

    // Setting the maximum immediately contrains the current value.
    ui->doubleSpinBox_trackingAreaWidth->setMaximum(maxWidth);
    ui->doubleSpinBox_trackingAreaHeight->setMaximum(maxHeight);

    // The maximum value guarantees these values will be correctly constrained.
    double width = ui->doubleSpinBox_trackingAreaWidth->value();
    double height = ui->doubleSpinBox_trackingAreaHeight->value();

    cv::Rect2d trackingArea(x, y, width, height);

    manager.markerDetection.UpdateTrackingArea(trackingArea);

    ui->pushButton_saveSettings->setEnabled(true);
    ui->pushButton_loadSettings->setEnabled(true);
}

void MainWindow::UpdateCameraParameters()
{
    double gamma = ui->doubleSpinBox_gamma->value();
    manager.camera.UpdateGamma(gamma);

    bool isRotate = ui->radioButton_rotate180->isChecked();
    manager.camera.UpdateRotation(isRotate);

    ui->pushButton_saveSettings->setEnabled(true);
    ui->pushButton_loadSettings->setEnabled(true);
}

void MainWindow::UpdateCalibrationParameters()
{
    double squareSize = ui->doubleSpinBox_checkerboardSquare->value();
    int horizontalIntersections = ui->spinBox_checkerboardHorizontal->value() - 1;
    int verticalIntersections = ui->spinBox_checkerboardVertical->value() - 1;
    cv::Size checkerboardIntersections = cv::Size(horizontalIntersections, verticalIntersections);

    manager.calibration.UpdateCalibrationParameters(checkerboardIntersections, squareSize);

    settings.checkerboardHorizontal = ui->spinBox_checkerboardHorizontal->value();
    settings.checkerboardVertical = ui->spinBox_checkerboardVertical->value();
    settings.checkerboardSquareSize = ui->doubleSpinBox_checkerboardSquare->value();
    settings.Save();
}

void MainWindow::UpdateDetectorParameters()
{
    DetectorParameterData detectorParameters;

    detectorParameters.markerDictionarySize = ui->spinBox_markerDictionarySize->value();
    detectorParameters.markerNumBits = ui->spinBox_markerNumBits->value();

    detectorParameters.adaptiveThreshWinSizeMin = ui->spinBox_adaptiveThreshWinSizeMin->value();
    detectorParameters.adaptiveThreshWinSizeMax = ui->spinBox_adaptiveThreshWinSizeMax->value();

    if (detectorParameters.adaptiveThreshWinSizeMin > detectorParameters.adaptiveThreshWinSizeMax) {
        detectorParameters.adaptiveThreshWinSizeMin = detectorParameters.adaptiveThreshWinSizeMax;
        ui->spinBox_adaptiveThreshWinSizeMin->setValue(detectorParameters.adaptiveThreshWinSizeMin);
    }

    detectorParameters.adaptiveThreshWinSizeStep = ui->spinBox_adaptiveThreshWinSizeStep->value();
    detectorParameters.adaptiveThreshConstant = ui->spinBox_adaptiveThreshConstant->value();

    detectorParameters.minMarkerPerimeterRate = ui->doubleSpinBox_minMarkerPerimeterRate->value();
    detectorParameters.maxMarkerPerimeterRate = ui->doubleSpinBox_maxMarkerPerimeterRate->value();
    detectorParameters.polygonalApproxAccuracyRate = ui->doubleSpinBox_polygonalApproxAccuracyRate->value();
    detectorParameters.minCornerDistanceRate = ui->doubleSpinBox_minCornerDistanceRate->value();
    detectorParameters.minMarkerDistanceRate = ui->doubleSpinBox_minMarkerDistanceRate->value();

    detectorParameters.minDistanceToBorder = ui->spinBox_minDistanceToBorder->value();
    detectorParameters.markerBorderBits = ui->spinBox_markerBorderBits->value();
    detectorParameters.minOtsuStdDev = ui->doubleSpinBox_minOtsuStdDev->value();
    detectorParameters.perspectiveRemovePixelPerCell = ui->spinBox_perspectiveRemovePixelPerCell->value();
    detectorParameters.perspectiveRemoveIgnoredMarginPerCell = ui->doubleSpinBox_perspectiveRemoveIgnoredMarginPerCell->value();

    detectorParameters.maxErroneousBitsInBorderRate = ui->doubleSpinBox_maxErroneousBitsInBorderRate->value();
    detectorParameters.errorCorrectionRate = ui->doubleSpinBox_errorCorrectionRate->value();

    manager.markerDetection.UpdateDetectorParameters(detectorParameters);

    ui->pushButton_saveSettings->setEnabled(true);
    ui->pushButton_loadSettings->setEnabled(true);
}

void MainWindow::GenerateMarkerImages()
{
    setCursor(Qt::WaitCursor);

    QString markersFolderPath = QDir::currentPath() + "/markers";
    QDir markersDirectory(markersFolderPath);
    if (markersDirectory.exists()) {
        markersDirectory.removeRecursively();
    }
    markersDirectory.mkdir(".");

    int imageSize = ui->spinBox_markerImageSize->value();
    bool isSaved = manager.markerDetection.GenerateMarkerImages(imageSize);
    if (!isSaved) {
        QMessageBox msgBox;
        msgBox.setText("<b>An error occurred trying to save marker images</b>");
        msgBox.setInformativeText("You may not have permission to write the files.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    std::cout << "Open markers folder: " << markersFolderPath.toStdString() << std::endl;
    // Opens a folder window where the marker images are stored
    QDesktopServices::openUrl(QUrl::fromLocalFile(markersFolderPath));

    setCursor(Qt::ArrowCursor);
}

void MainWindow::LoadSettings()
{
    if (!settings.Load()) {
        return;
    }

    ui->checkBox_startMinimized->setChecked(settings.startMinimized);

    QStringList substrings = settings.networkIpAddress.split('.');
    if (substrings.length() == 4) {
        ui->spinBox_ip_1->setValue(substrings[0].toInt());
        ui->spinBox_ip_2->setValue(substrings[1].toInt());
        ui->spinBox_ip_3->setValue(substrings[2].toInt());
        ui->spinBox_ip_4->setValue(substrings[3].toInt());
    }
    ui->spinBox_port->setValue(settings.networkPort);

    ui->doubleSpinBox_gamma->setValue(settings.gamma);
    ui->radioButton_rotate180->setChecked(settings.rotate);
    ui->radioButton_rotate0->setChecked(!settings.rotate);

    ui->doubleSpinBox_trackingAreaX->setValue(settings.trackingAreaX);
    ui->doubleSpinBox_trackingAreaY->setValue(settings.trackingAreaY);
    ui->doubleSpinBox_trackingAreaWidth->setValue(settings.trackingAreaWidth);
    ui->doubleSpinBox_trackingAreaHeight->setValue(settings.trackingAreaHeight);

    ui->spinBox_checkerboardHorizontal->setValue(settings.checkerboardHorizontal);
    ui->spinBox_checkerboardVertical->setValue(settings.checkerboardVertical);
    ui->doubleSpinBox_checkerboardSquare->setValue(settings.checkerboardSquareSize);

    ui->spinBox_markerNumBits->setValue(settings.markerNumBits);
    ui->spinBox_markerDictionarySize->setValue(settings.markerDictionarySize);
    ui->spinBox_markerImageSize->setValue(settings.markerImageSize);

    ui->spinBox_adaptiveThreshWinSizeMin->setValue(settings.adaptiveThreshWinSizeMin);
    ui->spinBox_adaptiveThreshWinSizeMax->setValue(settings.adaptiveThreshWinSizeMax);
    ui->spinBox_adaptiveThreshWinSizeStep->setValue(settings.adaptiveThreshWinSizeStep);
    ui->spinBox_adaptiveThreshConstant->setValue(settings.adaptiveThreshConstant);

    ui->doubleSpinBox_minMarkerPerimeterRate->setValue(settings.minMarkerPerimeterRate);
    ui->doubleSpinBox_maxMarkerPerimeterRate->setValue(settings.maxMarkerPerimeterRate);
    ui->doubleSpinBox_polygonalApproxAccuracyRate->setValue(settings.polygonalApproxAccuracyRate);
    ui->doubleSpinBox_minCornerDistanceRate->setValue(settings.minCornerDistanceRate);
    ui->doubleSpinBox_minMarkerDistanceRate->setValue(settings.minMarkerDistanceRate);
    ui->spinBox_minDistanceToBorder->setValue(settings.minDistanceToBorder);

    ui->spinBox_markerBorderBits->setValue(settings.markerBorderBits);
    ui->doubleSpinBox_minOtsuStdDev->setValue(settings.minOtsuStdDev);
    ui->spinBox_perspectiveRemovePixelPerCell->setValue(settings.perspectiveRemovePixelPerCell);
    ui->doubleSpinBox_perspectiveRemoveIgnoredMarginPerCell->setValue(settings.perspectiveRemoveIgnoredMarginPerCell);

    ui->doubleSpinBox_maxErroneousBitsInBorderRate->setValue(settings.maxErroneousBitsInBorderRate);
    ui->doubleSpinBox_errorCorrectionRate->setValue(settings.errorCorrectionRate);

    ui->pushButton_saveSettings->setEnabled(false);
    ui->pushButton_loadSettings->setEnabled(false);
}

void MainWindow::SaveSettings()
{
    settings.startMinimized = ui->checkBox_startMinimized->isChecked();

    settings.networkIpAddress = QString("%1.%2.%3.%4").arg(
        QString::number(ui->spinBox_ip_1->value()),
        QString::number(ui->spinBox_ip_2->value()),
        QString::number(ui->spinBox_ip_3->value()),
        QString::number(ui->spinBox_ip_4->value()));
    settings.networkPort = ui->spinBox_port->value();

    settings.gamma = ui->doubleSpinBox_gamma->value();
    settings.rotate = ui->radioButton_rotate180->isChecked();

    settings.trackingAreaX = ui->doubleSpinBox_trackingAreaX->value();
    settings.trackingAreaY = ui->doubleSpinBox_trackingAreaY->value();
    settings.trackingAreaWidth = ui->doubleSpinBox_trackingAreaWidth->value();
    settings.trackingAreaHeight = ui->doubleSpinBox_trackingAreaHeight->value();

    settings.checkerboardHorizontal = ui->spinBox_checkerboardHorizontal->value();
    settings.checkerboardVertical = ui->spinBox_checkerboardVertical->value();
    settings.checkerboardSquareSize = ui->doubleSpinBox_checkerboardSquare->value();

    settings.markerNumBits = ui->spinBox_markerNumBits->value();
    settings.markerDictionarySize = ui->spinBox_markerDictionarySize->value();
    settings.markerImageSize = ui->spinBox_markerImageSize->value();

    settings.adaptiveThreshWinSizeMin = ui->spinBox_adaptiveThreshWinSizeMin->value();
    settings.adaptiveThreshWinSizeMax = ui->spinBox_adaptiveThreshWinSizeMax->value();
    settings.adaptiveThreshWinSizeStep = ui->spinBox_adaptiveThreshWinSizeStep->value();
    settings.adaptiveThreshConstant = ui->spinBox_adaptiveThreshConstant->value();

    settings.minMarkerPerimeterRate = ui->doubleSpinBox_minMarkerPerimeterRate->value();
    settings.maxMarkerPerimeterRate = ui->doubleSpinBox_maxMarkerPerimeterRate->value();
    settings.polygonalApproxAccuracyRate = ui->doubleSpinBox_polygonalApproxAccuracyRate->value();
    settings.minCornerDistanceRate = ui->doubleSpinBox_minCornerDistanceRate->value();
    settings.minMarkerDistanceRate = ui->doubleSpinBox_minMarkerDistanceRate->value();

    settings.minDistanceToBorder = ui->spinBox_minDistanceToBorder->value();
    settings.markerBorderBits = ui->spinBox_markerBorderBits->value();
    settings.minOtsuStdDev = ui->doubleSpinBox_minOtsuStdDev->value();
    settings.perspectiveRemovePixelPerCell = ui->spinBox_perspectiveRemovePixelPerCell->value();
    settings.perspectiveRemoveIgnoredMarginPerCell = ui->doubleSpinBox_perspectiveRemoveIgnoredMarginPerCell->value();

    settings.maxErroneousBitsInBorderRate = ui->doubleSpinBox_maxErroneousBitsInBorderRate->value();
    settings.errorCorrectionRate = ui->doubleSpinBox_errorCorrectionRate->value();

    settings.Save();

    ui->pushButton_saveSettings->setEnabled(false);
    ui->pushButton_loadSettings->setEnabled(false);
}

void MainWindow:: OnSettingsError(QString text, QString informativeText)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setInformativeText(informativeText);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
}

void MainWindow::OnStartCalibration()
{
    ui->checkBox_toggleCalibration->setChecked(false);

    ui->stackedWidget->setCurrentIndex(1);
    manager.calibration.ClearImages();

    ui->checkBox_prepare1->setChecked(false);
    ui->checkBox_prepare2->setChecked(false);
    ui->groupBox_step3->setEnabled(false);
    ui->pushButton_calibrate->setEnabled(false);
    ui->progressBar_calibration->setValue(0);
    ui->progressBar_calibration->setFormat(QString::number(0));
    ui->groupBox_step4->setEnabled(false);
    ui->checkBox_toggleCalibrationPreview->setChecked(false);
    ui->label_rmsCalibrationError->setText("");

    ui->pushButton_saveCalibration->setEnabled(false);
}

void MainWindow::SaveCalibration()
{
    setCursor(Qt::WaitCursor);

    QString calibrationFolderPath = QDir::currentPath() + "/calibration";
    QDir calibrationDirectory(calibrationFolderPath);
    if (calibrationDirectory.exists()) {
        calibrationDirectory.removeRecursively();
    }
    calibrationDirectory.mkdir(".");

    CalibrationSaveResult saveResult = manager.calibration.SaveCalibration();

    if (saveResult != CalibrationSaveResult::Succeeded) {
        QMessageBox msgBox;

        if (saveResult == CalibrationSaveResult::Failed) {
            msgBox.setText("<b>An error occurred trying to save calibration.yml and calibration images</b>");
            msgBox.setInformativeText("The content may not be formatted properly or you may not have permission to write the files.");
        }
        else if (saveResult == CalibrationSaveResult::CalibrationError) {
            msgBox.setText("<b>An error occurred trying to save calibration.yml</b>");
            msgBox.setInformativeText("The content may not be formatted properly or you may not have permission to write the file.");
        }
        else if (saveResult == CalibrationSaveResult::ImagesError) {
            msgBox.setText("<b>An error occurred trying to save the calibration images</b>");
            msgBox.setInformativeText("You may not have permission to write the files.");
        }

        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    manager.calibration.ClearImages();
    setCursor(Qt::ArrowCursor);

    ui->stackedWidget->setCurrentIndex(0);
    ui->checkBox_toggleCalibration->setEnabled(manager.calibration.GetIsCalibrated());
    ui->checkBox_toggleCalibration->setChecked(true);
    ui->checkBox_toggleCalibrationPreview->setChecked(false);
    ui->progressBar_calibration->setValue(0);
}

void MainWindow::CancelCalibration()
{
    setCursor(Qt::ArrowCursor);

    CalibrationLoadResult loadResult = manager.calibration.LoadCalibration();

    if (loadResult == CalibrationLoadResult::FileParseError) {
        QMessageBox msgBox;
        msgBox.setText("<b>An error occurred trying to load calibration.yml</b>");
        msgBox.setInformativeText("The content may not be formatted properly or you may not have permission to read the file.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    manager.calibration.ClearImages();

    ui->stackedWidget->setCurrentIndex(0);
    ui->checkBox_toggleCalibration->setEnabled(manager.calibration.GetIsCalibrated());
    ui->checkBox_toggleCalibration->setChecked(ui->checkBox_toggleCalibration->isEnabled());
    ui->checkBox_toggleCalibrationPreview->setChecked(false);
    ui->progressBar_calibration->setValue(0);
}

void MainWindow::CaptureCalibrationImage()
{
    setCursor(Qt::BusyCursor);
    manager.calibration.CaptureImage();
}

void MainWindow::UpdateImageCaptureProgress(int numImages)
{
    setCursor(Qt::ArrowCursor);
    ui->progressBar_calibration->setValue(numImages);
    ui->progressBar_calibration->setFormat(QString::number(numImages));
}

void MainWindow::EnableStep3Calibrate()
{
    ui->groupBox_step3->setEnabled(true);
    ui->pushButton_calibrate->setEnabled(true);
}

void MainWindow::CalibrateCamera()
{
    setCursor(Qt::WaitCursor);
    manager.calibration.CalibrateCamera();
}

void MainWindow::EnableStep4ReviewAndSave(double rmsError)
{
    setCursor(Qt::ArrowCursor);
    ui->groupBox_step4->setEnabled(true);

    ui->checkBox_toggleCalibrationPreview->setEnabled(true);
    ui->checkBox_toggleCalibrationPreview->setChecked(true);

    ui->label_rmsCalibrationError->setEnabled(true);
    ui->label_rmsCalibrationError->setText(QString::number(rmsError, 'f', 2));

    ui->pushButton_saveCalibration->setEnabled(true);
}

void MainWindow::OpenCalibrationImages()
{
    QString calibrationFolderPath = QDir::currentPath() + "//calibration";
    std::cout << "Open calibration folder: " << calibrationFolderPath.toStdString() << std::endl;

    // Opens a folder window where the calibration files are stored
    QDesktopServices::openUrl(QUrl::fromLocalFile(calibrationFolderPath));
}

void MainWindow::ToggleCalibration(bool isChecked)
{
    if (manager.calibration.GetIsCalibrated()) {
        if (isChecked) {
            ui->label_calibrationStatus->setText("Calibrated (On)");
            ui->label_calibrationStatus->setStyleSheet("color: darkblue;");
        }
        else {
            ui->label_calibrationStatus->setText("Calibrated (Off)");
            ui->label_calibrationStatus->setStyleSheet("color: darkblue;");
        }

        manager.camera.ToggleCalibration(isChecked);
    }
    else {
        ui->label_calibrationStatus->setText("Uncalibrated");
        ui->label_calibrationStatus->setStyleSheet("color: darkred;");
    }
}

void MainWindow::ToggleCalibrationPreview(bool isChecked)
{
    manager.camera.ToggleCalibrationPreview(isChecked);
}
