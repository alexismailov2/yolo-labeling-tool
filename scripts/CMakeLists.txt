cmake_minimum_required(VERSION 3.16)
project(yolo-cfg-generator)

if (NOT YOLO_MODEL_NAME)
   set(YOLO_MODEL_NAME yolov3-tiny)
endif()

if (NOT NEXT_STAGE)
   set(NEXT_STAGE OFF)
endif()

if (NOT NEXT_STAGE)
   message(STATUS "Start creating data file")
   execute_process(COMMAND ${CMAKE_COMMAND}
      -Bgenerate
      -DYOLO_DATASET_FOLDER_PATH=${YOLO_DATASET_FOLDER_PATH}
      -DYOLO_MODEL_NAME=${YOLO_MODEL_NAME}
      -DNEXT_STAGE=ON)

   message(STATUS "Start calculation anchors")
   execute_process(COMMAND ./darknet detector calc_anchors ${CMAKE_CURRENT_LIST_DIR}/${YOLO_MODEL_NAME}.data -num_of_clusters 6 -width 416 -height 416
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
      RESULT_VARIABLE CALC_ANCHORS_RESULT)

   if(NOT CALC_ANCHORS_RESULT EQUAL "0")
      message(FATAL_ERROR "calc anchors failed!")
      return()
   endif()

   message(STATUS "Create yolo config file")
   include(create_yolo_cfg_file.cmake)

   file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/anchors.txt)
else()
   message(STATUS "Create data file")
   include(create_data_file.cmake)
endif()
