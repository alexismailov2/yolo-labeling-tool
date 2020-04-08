project(wider_person_dataset_converter_to_dataset)

add_executable(${PROJECT_NAME}
   wider_person_dataset_convertor_to_darknet.cpp)

target_link_libraries(${PROJECT_NAME}
   ${OpenCV_LIBS}
   stdc++fs)