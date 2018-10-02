#pragma once

#include "macros.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef struct buffer_t_* buffer_t;

NAPP_API void buffer_init(buffer_t* buffer, void* memory, int size);

NAPP_API int buffer_resize(buffer_t buffer, int new_size);

NAPP_API void* buffer_data(buffer_t buffer);

#ifdef __cplusplus

template <typename T>
inline T* buffer_data(buffer_t buffer)
{
    return static_cast<T*>(buffer_data(buffer));
}

template <typename T>
inline T& buffer_data_ref(buffer_t buffer)
{
    return *static_cast<T*>(buffer_data(buffer));
}

#endif

NAPP_API void buffer_free(buffer_t buffer);

//NAPP_API int napp_fs_load_file(const char* fname, void** rdbuff, int* max);
NAPP_API int napp_fs_load_file(const char* fname, buffer_t rdbuff);
