cmake_minimum_required(VERSION 3.16)
project(DecimalComputerSimulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(DEFINED ENV{QT_DIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
endif()

find_package(Qt6 COMPONENTS Widgets QuickWidgets Graphs Qml REQUIRED)

add_library(SimulatorCore STATIC
    src/core/Memory.h src/core/Memory.cpp
    src/core/CPU.h src/core/CPU.cpp
    src/core/Config.h
    src/core/Computer.h src/core/Computer.cpp)

qt_add_executable(SimulatorApp
    src/main.cpp
    src/gui/mainwindow.h src/gui/mainwindow.cpp src/gui/mainwindow.ui
    resources.qrc
    src/gui/CpuStats.h
    assets/windows_resource.rc
    src/assembler/Assembler.h
    src/assembler/Assembler.cpp
    src/assembler/Disassembler.h
    src/assembler/Disassembler.cpp
)

set_target_properties(SimulatorApp PROPERTIES
    WIN32_EXECUTABLE $<$<CONFIG:Release>:TRUE>
)

target_link_libraries(SimulatorApp PRIVATE SimulatorCore Qt6::Widgets Qt6::QuickWidgets Qt6::Graphs Qt6::Qml)
target_include_directories(SimulatorApp PRIVATE 
    src 
    ${CMAKE_CURRENT_BINARY_DIR}
)
