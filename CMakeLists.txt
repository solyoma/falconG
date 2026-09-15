cmake_minimum_required(VERSION 3.16)
project(falconG VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt6 REQUIRED COMPONENTS
    Core Concurrent Gui Widgets
    Multimedia MultimediaWidgets
    WebEngineCore WebEngineWidgets
)
qt_standard_project_setup()


add_subdirectory(src)
