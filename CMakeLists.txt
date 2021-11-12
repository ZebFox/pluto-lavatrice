# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

#add_definitions(-DGEL_PAGEMANAGER_CONFIGURATION_HEADER="view/view_types.h")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32-template)
