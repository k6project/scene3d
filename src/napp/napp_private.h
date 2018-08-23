#pragma once

#ifndef NAPP_PRIVATE
#error This is a private libNApp file
#endif

#include "napp.h"

typedef struct NAppState
{
	void* Handle;
} NAppState;

extern NAppState GState;
