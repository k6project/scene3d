cmake_minimum_required(VERSION 3.1.3)
project(scene3d C CXX)

set(LIBNAPP_SOURCES
    inc/napp/macros.h
    inc/napp/appmain.h
    inc/napp/window.h
    inc/napp/callback.h
    src/napp/common.h
    src/napp/common.c
)

set(GLVIEW_SOURCES
    src/glview/main.cpp
    src/glview/glfunc.inl
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
set(PLATFORM_LIBS ${LIB_COCOA})

endif(APPLE)

add_library(napp ${LIBNAPP_SOURCES})
target_compile_definitions(napp PUBLIC ${PLATFORM_MACRO})
target_include_directories(napp PUBLIC inc extern/inc)

add_executable(glview WIN32 MACOSX_BUNDLE ${GLVIEW_SOURCES})
target_link_libraries(glview napp ${PLATFORM_LIBS})

#add_executable(vkview WIN32 MACOSX_BUNDLE ${VKVIEW_SOURCES})
#target_link_libraries(vkview napp ${PLATFORM_LIBS})

#add_executable(the_base ${THE_BASE_SOURCES})
#target_include_directories(the_base PRIVATE extern/inc)
