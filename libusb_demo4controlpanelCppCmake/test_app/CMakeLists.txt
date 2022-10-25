cmake_minimum_required(VERSION 3.14)

project(test_app VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(targetAppName ${PROJECT_NAME})



find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick)

include(CMakePrintHelpers)
cmake_print_variables(INCLUDE_DIRECTORIES)

set(PROJECT_SOURCES
    usinglibusb.cpp
    usinglibusb.h
        main.cpp
        qml.qrc
)


add_executable(${targetAppName} ${PROJECT_SOURCES})


target_include_directories(${targetAppName} PRIVATE ${LIBUSB_INCLUDE_DIRS} ../controlpanel_controller)

target_compile_definitions(${targetAppName}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
#Attention: below code I forgot '${}' so variable cannot evulated , cost me 1h to debug it. eton@22/10/17
target_link_libraries(${targetAppName}
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick ${LIBUSB_LIBRARIES} controlpanelController)


message(STATUS "debug:INCLUDE_DIRECTORIES=${INCLUDE_DIRECTORIES}")
MESSAGE(STATUS "debug variable infomation=${INCLUDE_DIRECTORIES}")
message(STATUS "CMAKE FINISH...--by eton")

