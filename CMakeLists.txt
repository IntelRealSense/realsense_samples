cmake_minimum_required(VERSION 2.8.9)
project(realsense_samples)

if(NOT SAMPLE_PREFIX)
set(SAMPLE_PREFIX "sample_")
endif()

add_definitions( -DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" )
install(FILES README.md DESTINATION share/doc/librealsense-samples)
add_subdirectory(samples)
