cmake_minimum_required(VERSION 3.5)
project (transporter)

add_definitions(-std=c++11)

# # seasocks
if (NOT TARGET seasocks_lib)
    include(ExternalProject)
    ExternalProject_Add(seasocks_lib
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/seasocks
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/seasocks
    )
endif()

if (DEFINED CMAKE_INSTALL_LIBDIR)
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
else()
set(INSTALL_LIBDIR lib) 
endif()

add_library(seasocks STATIC IMPORTED)
set_property(TARGET seasocks PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/seasocks/${INSTALL_LIBDIR}/libseasocks.a)
add_dependencies(seasocks seasocks_lib)

if (NOT TARGET transporter)
    add_library(transporter STATIC transporter.cpp)

    set(transporter_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/seasocks/include)
    target_link_libraries(transporter
        pthread
        seasocks
        )
endif()
add_dependencies(transporter seasocks)
