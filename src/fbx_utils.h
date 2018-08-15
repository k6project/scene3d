#pragma once

#ifdef __cplusplus
#define FBX_API extern "C"
#else // __cplusplus
#define FBX_API
#endif // __cplusplus

FBX_API void FbxLoadScene();
