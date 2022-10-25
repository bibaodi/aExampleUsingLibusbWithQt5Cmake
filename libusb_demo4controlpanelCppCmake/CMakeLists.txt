cmake_minimum_required(VERSION 3.14)

project(libusb_demo4controlpanelCppCmake VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(targetAppName ${PROJECT_NAME})

include(FindPkgConfig)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick)

find_package(PkgConfig)
MESSAGE(STATUS "PKG_CONFIG-ENV=[$ENV{PKG_CONFIG_PATH}], PKG_CONFIG_FOUND=${PKG_CONFIG_FOUND}")

pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

MESSAGE(STATUS "libusb status:include=[${LIBUSB_INCLUDE_DIRS}].")
include(CMakePrintHelpers)
cmake_print_variables(INCLUDE_DIRECTORIES)

set(PROJECT_SOURCES
    usinglibusb.cpp
    usinglibusb.h
    controlpanel_usb_controller.h
    controlpanel_usb_controller.cpp
    semaphore_cpp17.h
    libcrc_cpp.h
    controlpanel_protocal.h controlpanel_protocal.cpp
        main.cpp
        qml.qrc
)


add_executable(${targetAppName} ${PROJECT_SOURCES})

cmake_print_properties(
    TARGETS ${targetAppName}
    PROPERTIES INCLUDE_DIRECTORIES
)

#include_directories(SYSTEM PRIATE LIBUSB_INCLUDE_DIRS)
target_include_directories(${targetAppName} PRIVATE ${LIBUSB_INCLUDE_DIRS})

target_compile_definitions(${targetAppName}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
#Attention: below code I forgot '${}' so variable cannot evulated , cost me 1h to debug it. eton@22/10/17
target_link_libraries(${targetAppName}
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick ${LIBUSB_LIBRARIES}
pthread)


message(STATUS "debug:INCLUDE_DIRECTORIES=${INCLUDE_DIRECTORIES}")
MESSAGE(STATUS "debug variable infomation=${INCLUDE_DIRECTORIES}")
message(STATUS "CMAKE FINISH...--by eton")
