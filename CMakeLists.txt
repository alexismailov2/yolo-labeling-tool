cmake_minimum_required(VERSION 3.1.0)

project(YoloLabel)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
   set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

add_executable(${PROJECT_NAME}
   mainwindow.ui
   mainwindow.h
   mainwindow.cpp
   main.cpp
   datasetproject.cpp
   datasetproject.h
   BoundingBoxSelector.cpp
   BoundingBoxSelector.h
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  AUTOMOC ON
  AUTOUIC ON
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
  )