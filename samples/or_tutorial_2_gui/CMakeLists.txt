cmake_minimum_required (VERSION 2.8.9)
project(${SAMPLE_PREFIX}or_tutorial_2_gui)

# set path
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set(CONSOLE_DISPLAY_DIR ${COMMON_DIR}/console_display)
set(GUI_DISPLAY_DIR ${COMMON_DIR}/gui_display)
set(COMMON_UTILS_DIR ${COMMON_DIR}/utils)

set(OPENCV_LIBRARIES opencv_highgui opencv_imgproc opencv_core)
set(RS_SDK_LIBRARIES realsense_projection realsense_image realsense_playback realsense_record realsense_log_utils)
set(PROJECT_LINK_LIBS
    ${RS_SDK_LIBRARIES}
    realsense
    pthread
    realsense_object_recognition
    ${OPENCV_LIBRARIES}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -fmessage-length=0 --std=c++11 -pthread -fPIC -std=c++0x -fexceptions -frtti -ffunction-sections -fdata-sections")

set(SOURCES
    cpp/main.cpp
)

include_directories(
    /usr/include
    /usr/include/librealsense
    /usr/include/librealsense/object_recognition
    ${CONSOLE_DISPLAY_DIR}
    ${GUI_DISPLAY_DIR}
    ${COMMON_UTILS_DIR}
)

link_directories(
    /usr/lib
    /usr/local/lib
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
