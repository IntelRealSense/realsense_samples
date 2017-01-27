cmake_minimum_required(VERSION 2.8)

project(${SAMPLE_PREFIX}slam_or_pt_tutorial_1)

# set path
set(SOURCE_BASE ${PROJECT_SOURCE_DIR}/)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set(CONSOLE_DISPLAY_DIR ${COMMON_DIR}/console_display)
set(COMMON_UTILS_DIR ${COMMON_DIR}/utils)

if(NOT INSTALL_PREFIX)
    add_definitions( -DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -fmessage-length=0 --std=c++11 -pthread -fPIC -std=c++0x -fexceptions -frtti -ffunction-sections -fdata-sections")
add_definitions(-DPERSON_TRACKING_DATA_FILES="/usr/share/librealsense/pt/data/")

include_directories(
    /usr/include
    /usr/include/librealsense
    /usr/include/librealsense/object_recognition
    /usr/include/librealsense/slam
    /usr/include/librealsense/pt/
    ${CONSOLE_DISPLAY_DIR}
    ${COMMON_UTILS_DIR}
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/lib
    /usr/local/lib
)

set(SOURCES
    cpp/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

set(OPENCV_LIBRARIES opencv_highgui opencv_imgproc opencv_core)
set(RS_SDK_LIBRARIES realsense_projection realsense_image realsense_lrs_image)
set(SLAM_LIBRARIES realsense_slam SP_Core tracker)

set(LIBRARIES_TO_LINK
    ${RS_SDK_LIBRARIES}
    realsense_lrs_image
    pthread
    realsense
    ${SLAM_LIBRARIES}
    realsense_object_recognition
    realsense_persontracking
    ${OPENCV_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES_TO_LINK})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
