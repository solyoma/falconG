cmake_minimum_required(VERSION 3.16)

project(falconG VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ---------------------------
# Detect Qt major version
# ---------------------------
# Try Qt6 first
find_package(Qt6 COMPONENTS Core QUIET)

if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    # Fall back to Qt5
    find_package(Qt5 REQUIRED COMPONENTS Core)
    set(QT_VERSION_MAJOR 5)
endif()

message(STATUS "Using Qt${QT_VERSION_MAJOR}")

# ---------------------------
# Qt components (Qt5 vs Qt6)
# ---------------------------
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS
        Core Concurrent Gui Widgets
        Multimedia MultimediaWidgets
        WebEngineCore WebEngineWidgets
    )
else()
    find_package(Qt5 REQUIRED COMPONENTS
        Core Concurrent Gui Widgets
        Multimedia MultimediaWidgets
        WebEngineWidgets
    )
endif()

# ---------------------------
# Curl
# ---------------------------
find_package(CURL REQUIRED)

# ---------------------------
# Qt6 project setup (Qt5 ignores this)
# ---------------------------
if(QT_VERSION_MAJOR EQUAL 6)
    qt_standard_project_setup()
endif()

add_subdirectory(src)
