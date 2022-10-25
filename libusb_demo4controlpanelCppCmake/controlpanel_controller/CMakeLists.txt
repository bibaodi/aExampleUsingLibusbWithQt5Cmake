cmake_minimum_required(VERSION 3.14)

project(controlpanelController VERSION 1.0.0 DESCRIPTION "The library for control panel control." LANGUAGES CXX)

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
    controlpanel_usb_controller.h
    controlpanel_usb_controller.cpp
    semaphore_cpp17.h
    libcrc_cpp.h
    controlpanel_protocal.h controlpanel_protocal.cpp
)


add_library(${targetAppName} ${PROJECT_SOURCES})

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
  PRIVATE Qt${QT_VERSION_MAJOR}::Core ${LIBUSB_LIBRARIES} pthread)

set_target_properties(${targetAppName} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${targetAppName} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${targetAppName} PROPERTIES PUBLIC_HEADER controlpanel_usb_controller.h)

install(TARGETS ${targetAppName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

message(STATUS "controlpanelController: CMAKE FINISH...--by eton")

