cmake_minimum_required(VERSION 2.8.9)
project(realsense_samples)

# If not set on commandline, use Yocto cross-compiler SDK environment as root
if(NOT SYSROOT)
set(SYSROOT "$ENV{SDKTARGETSYSROOT}")
endif()

add_subdirectory(samples)
