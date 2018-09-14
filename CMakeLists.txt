cmake_minimum_required(VERSION 3.1.3)
project(scene3d C)

set(LIBNAPP_SOURCES 
    src/napp/napp.h
    src/napp/napp_main.h
)

# MacOS

if(APPLE)

set(PLATFORM_MACRO _NAPP_MACOS_)
set(VULKAN_INC $ENV{VULKAN_SDK}/macOS/include)

set(LIBNAPP_SOURCES
    ${LIBNAPP_SOURCES}
    src/napp/napp_macos.m
)

find_library(LIB_COCOA Cocoa)
mark_as_advanced(LIB_COCOA)
set(PLATFORM_LIBS ${LIB_COCOA})

endif(APPLE)

set (VKVIEW_SOURCES
    src/vkview/main.c
)

add_library(napp ${LIBNAPP_SOURCES})
target_compile_definitions(napp PUBLIC ${PLATFORM_MACRO} VK_NO_PROTOTYPES)
target_include_directories(napp PUBLIC ${VULKAN_INC} src)

add_executable(vkview WIN32 MACOSX_BUNDLE ${VKVIEW_SOURCES})
target_link_libraries(vkview napp ${PLATFORM_LIBS})
