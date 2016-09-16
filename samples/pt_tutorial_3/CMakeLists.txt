cmake_minimum_required (VERSION 2.8.9)
project(sample_pt_tutorial_3)

add_definitions(-DPERSON_TRACKING_DATA_FILES="/usr/share/librealsense/pt/data/")

set(OPENCV_LIBRARIES opencv_imgproc opencv_core)
set(RS_SDK_LIBRARIES rs_projection rs_image rs_playback rs_record rs_log_utils)
set(PROJECT_LINK_LIBS
    ${RS_SDK_LIBRARIES}
    realsense
    pthread
    realsense_persontracking
    ${OPENCV_LIBRARIES}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -pthread -fPIC -std=c++0x -fexceptions -frtti")

set(SOURCES
    cpp/main.cpp
    cpp/camera_utils.cpp
    cpp/console_view.cpp
)

include_directories(
    cpp 
    ${SYSROOT}/usr/include
    ${SYSROOT}/usr/include/librealsense 
    ${SYSROOT}/usr/include/librealsense/pt)

link_directories(
    ${SYSROOT}/usr/lib)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
