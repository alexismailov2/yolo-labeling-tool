cmake_minimum_required(VERSION 3.1.0)

project(YoloLabel)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

add_executable(${PROJECT_NAME}
  MainWindow.ui
  MainWindow.h
  MainWindow.cpp
  DatasetProject.cpp
  DatasetProject.h
  ValidationClassBoxes.ui
  ValidationClassBoxes.h
  ValidationClassBoxes.cpp
  ValidationClassBoxesListModel.h
  ValidationClassBoxesListModel.cpp
  BoundingBoxSelector.cpp
  BoundingBoxSelector.h
  Utils.cpp
  Utils.h
  main.cpp
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