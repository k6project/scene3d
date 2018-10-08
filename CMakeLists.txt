cmake_minimum_required(VERSION 3.1.3)
project(scene3d C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIBNAPP_SOURCES
    inc/napp/macros.h
    inc/napp/appmain.h
    inc/napp/window.h
    inc/napp/callback.h
    inc/napp/filesys.h
    src/napp/common.h
    src/napp/common.c
)

set(GLVIEW_SOURCES
    src/glview/main.cpp
    src/glview/utils.hpp
    src/glview/utils.cpp
)

set(GEOMAKE_SOURCES
    src/geomake/geomake.cpp
    src/geomake/writers.hpp
    src/geomake/writers.cpp
)

# MacOS

if(APPLE)

set(PLATFORM_MACRO _NAPP_MACOS_ VK_USE_PLATFORM_MACOS_MVK)
set(VULKAN_INC $ENV{VULKAN_SDK}macOS/include)

set(LIBNAPP_SOURCES
    ${LIBNAPP_SOURCES}
    src/napp/macos_impl.m
)

find_library(LIB_COCOA Cocoa)
mark_as_advanced(LIB_COCOA)

find_library(LIB_OPENGL OpenGL)
mark_as_advanced(LIB_OPENGL)

endif(APPLE)

add_library(napp ${LIBNAPP_SOURCES})
target_compile_definitions(napp PUBLIC ${PLATFORM_MACRO})
target_include_directories(napp PUBLIC inc extern/inc ${VULKAN_INC})

add_executable(glview WIN32 MACOSX_BUNDLE ${GLVIEW_SOURCES})
target_link_libraries(glview napp ${LIB_COCOA} ${LIB_OPENGL})

add_executable(geomake ${GEOMAKE_SOURCES})
target_include_directories(geomake PRIVATE extern/inc)

if(APPLE)
file(GLOB GLVIEW_ASSETS test/glview/*.*)
set(GLVIEW_ASSETS_DIR $<TARGET_FILE_DIR:glview>/../Resources)
add_custom_command(TARGET glview PRE_BUILD COMMAND mkdir -p ${GLVIEW_ASSETS_DIR})
add_custom_command(TARGET glview PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLVIEW_ASSETS} ${GLVIEW_ASSETS_DIR})
endif(APPLE)
