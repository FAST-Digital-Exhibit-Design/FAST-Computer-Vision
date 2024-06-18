cmake_minimum_required(VERSION 3.5)

project(fast-computer-vision VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
find_package(OpenCV REQUIRED)

unset(Spinnaker_FOUND)
unset(Spinnaker_INCLUDE_DIRS)
unset(Spinnaker_LIBRARIES)

find_path(Spinnaker_INCLUDE_DIRS NAMES Spinnaker.h
  HINTS $ENV{SPINNAKER_DIR}/include
)
find_library(Spinnaker_LIBRARIES NAMES Spinnaker_v140
  HINTS $ENV{SPINNAKER_DIR}/lib64/vs2015
)

message(STATUS "Spinnaker_INCLUDE_DIRS: [${Spinnaker_INCLUDE_DIRS}]")
message(STATUS "Spinnaker_LIBRARIES: [${Spinnaker_LIBRARIES}]")

if (Spinnaker_INCLUDE_DIRS AND Spinnaker_LIBRARIES)
  message(STATUS "Spinnaker found in the system")
  set(Spinnaker_FOUND 1)
endif (Spinnaker_INCLUDE_DIRS AND Spinnaker_LIBRARIES)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/MarkdownViewerWindow.h
        src/MarkdownViewerWindow.cpp
        src/MarkdownViewerWindow.ui
        src/AppManager.cpp
        src/AppManager.h
        src/Settings.cpp
        src/Settings.h
        src/Camera.cpp
        src/Camera.h
        src/CalibrationData.h
        src/Calibration.cpp
        src/Calibration.h
        src/DetectorParameterData.h
        src/MarkerData.h
        src/MarkerDetection.cpp
        src/MarkerDetection.h
        src/NetworkCommunication.cpp
        src/NetworkCommunication.h
        src/FrameRateTimer.cpp
        src/FrameRateTimer.h
        src/ExecutionTimer.cpp
        src/ExecutionTimer.h
        src/pch.cpp
        src/pch.h
        ${Spinnaker_INCLUDE_DIRS}/Spinnaker.h
        ${Spinnaker_INCLUDE_DIRS}/SpinGenApi/SpinnakerGenApi.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(fast-computer-vision
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET fast-computer-vision APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(fast-computer-vision SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(afast-computer-vision
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(fast-computer-vision PRIVATE ${Spinnaker_INCLUDE_DIRS})
target_link_libraries(fast-computer-vision PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network ${OpenCV_LIBS} ${Spinnaker_LIBRARIES})


set_target_properties(fast-computer-vision PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS fast-computer-vision
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(fast-computer-vision)
endif()

add_custom_command(
    TARGET fast-computer-vision POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/doc
            ${CMAKE_CURRENT_BINARY_DIR}/Documentation
)

add_custom_command(
    TARGET fast-computer-vision POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ../LICENSES
            ${CMAKE_CURRENT_BINARY_DIR}/LICENSES
)
