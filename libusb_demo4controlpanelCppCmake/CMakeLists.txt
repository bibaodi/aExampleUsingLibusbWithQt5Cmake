cmake_minimum_required(VERSION 3.14)

project(controlpanelUsbController VERSION 0.1 LANGUAGES CXX)

set(targetAppName ${PROJECT_NAME})

add_subdirectory(test_app)
add_subdirectory(controlpanel_controller)
