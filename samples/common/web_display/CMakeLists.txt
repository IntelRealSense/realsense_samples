cmake_minimum_required(VERSION 3.5)

project (web_display)
add_definitions(-std=c++11)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/transporter)

# find JPEG library
find_library(JPEG_TURBO NAMES libturbojpeg.a jpeg HINTS /usr/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    /usr/include
    /usr/include/librealsense
    ${CMAKE_CURRENT_SOURCE_DIR}/transporter
    )

add_library(${PROJECT_NAME} STATIC)

set(RS_SDK_LIBRARIES realsense_projection realsense_image)

target_link_libraries(
    ${PROJECT_NAME}
    ${JPEG_TURBO}
    transporter
    realsense
    pthread
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
