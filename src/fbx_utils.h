#pragma once

#ifdef __cplusplus
#define FBX_UTILS_API extern "C"
#else // __cplusplus
#define FBX_UTILS_API
#endif // __cplusplus

FBX_UTILS_API void FbxLoadScene(const char* fName);
