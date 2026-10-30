cmake_minimum_required(VERSION 3.21)
project(LabBooker VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Sql PrintSupport Test)

add_executable(LabBooker
    src/main.cpp
    src/domain.h
    src/database.h src/database.cpp
    src/services.h src/services.cpp
    src/mainwindow.h src/mainwindow.cpp
    resources/resources.qrc
)
target_link_libraries(LabBooker PRIVATE Qt6::Widgets Qt6::Sql Qt6::PrintSupport)
target_include_directories(LabBooker PRIVATE src)

enable_testing()
add_executable(LabBookerTests
    tests/test_core.cpp
    src/domain.h
    src/database.h src/database.cpp
    src/services.h src/services.cpp
    resources/resources.qrc
)
target_link_libraries(LabBookerTests PRIVATE Qt6::Test Qt6::Sql Qt6::Widgets Qt6::PrintSupport)
target_include_directories(LabBookerTests PRIVATE src)
add_test(NAME LabBookerTests COMMAND LabBookerTests)

install(TARGETS LabBooker RUNTIME DESTINATION bin)
