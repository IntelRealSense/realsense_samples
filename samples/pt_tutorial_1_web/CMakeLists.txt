cmake_minimum_required (VERSION 2.8.9)
project(${SAMPLE_PREFIX}pt_tutorial_1_web)

if(NOT INSTALL_PREFIX)
    add_definitions( -DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" )
endif()

# set path
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set(CONSOLE_DISPLAY_DIR ${COMMON_DIR}/console_display)
set(REMOTE_DISPLAY_DIR ${COMMON_DIR}/web_display)
set(TRANSPORT_DIR ${REMOTE_DISPLAY_DIR}/transporter)
set(COMMON_UTILS_DIR ${COMMON_DIR}/utils)

if (NOT TARGET transporter)
    # remote display and transporter (networking) library
    add_subdirectory(${REMOTE_DISPLAY_DIR} "${CMAKE_CURRENT_BINARY_DIR}/web_display_build")
endif()

add_definitions(-DPERSON_TRACKING_DATA_FILES="/usr/share/librealsense/pt/data/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -fmessage-length=0 --std=c++11 -pthread -fPIC -std=c++0x -fexceptions -frtti -ffunction-sections -fdata-sections")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/transporter
    /usr/include
    /usr/include/librealsense
    /usr/include/librealsense/object_recognition
    /usr/include/librealsense/slam
    /usr/include/librealsense/pt/
    ${REMOTE_DISPLAY_DIR}
    ${CONSOLE_DISPLAY_DIR}
    ${COMMON_UTILS_DIR}
    ${TRANSPORT_DIR}
)

link_directories(
    /usr/lib
    ${CMAKE_CURRENT_BINARY_DIR}/transport_build
)

set(SOURCES
    cpp/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

set(OPENCV_LIBRARIES opencv_imgproc opencv_core opencv_highgui)
set(RS_SDK_LIBRARIES realsense_projection realsense_image realsense_playback realsense_record realsense_log_utils)

set(PROJECT_LINK_LIBS
    web_display
    ${RS_SDK_LIBRARIES}
    realsense
    pthread
    realsense_persontracking
    ${OPENCV_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBS})

# copy web directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/cpp/ui-browser/src
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}_browser
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cpp/ui-browser/src/
    DESTINATION share/librealsense/samples/${PROJECT_NAME}_browser)
