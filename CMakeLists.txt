cmake_minimum_required(VERSION 3.1.3)
project(scene3d C)

set(LIBNAPP_SOURCES 
    src/napp/napp.h
    src/napp/napp_private.h
    src/napp/napp_init.c
)

# MacOS

if(APPLE)

set(LIBNAPP_SOURCES
    ${LIBNAPP_SOURCES}
    src/napp/napp_init_mac.m
)

endif(APPLE)

add_library(napp ${LIBNAPP_SOURCES})
