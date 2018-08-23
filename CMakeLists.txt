cmake_minimum_required(VERSION 3.1.3)
project(scene3d C)

set(LIBNAPP_SOURCES 
    src/napp/napp.h
    src/napp/napp_private.h
    src/napp/napp_init.c
)

add_library(napp ${LIBNAPP_SOURCES})
