#pragma once

#ifdef _MSC_VER
#include <wchar.h>
typedef wchar_t TChar;
#define STR(s) L##s
#else
typedef char TChar;
#define STR(s) s
#endif

#define VK_INIT(v,t) do{memset(&v,0,sizeof(v));v.sType=t;}while(0)
#define TEST_RV(c,r,m) do{if(!(c)){appPrintf(STR(m)STR("\n"));return r;}}while(0)
#define TEST_R(c,m) do{if(!(c)){appPrintf(STR(m)STR("\n"));return;}}while(0)
#define QTEST_RV(c,r) do{if(!(c)){return r;}}while(0)
#define QTEST_R(c) do{if(!(c)){return;}}while(0)
