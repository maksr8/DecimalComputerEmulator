cmake_minimum_required(VERSION 3.16)
project(DecimalComputerEmulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(DEFINED ENV{QT_DIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
endif()

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_library(EmulatorCore STATIC
    src/core/Memory.h
    src/core/Memory.cpp
    src/core/CPU.h
    src/core/CPU.cpp
    src/core/Config.h
    src/core/Computer.h
    src/core/Computer.cpp
)

qt_add_executable(EmulatorApp
    src/main.cpp
    src/gui/mainwindow.h src/gui/mainwindow.cpp src/gui/mainwindow.ui
)

target_link_libraries(EmulatorApp PRIVATE EmulatorCore Qt6::Widgets)
target_include_directories(EmulatorApp PRIVATE src)