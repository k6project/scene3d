#include "common.h"

static void* g_context;

static napp_callback_t g_callback[NAPP_CALLBACKS];

static void napp_dummy(void* arg)
{
}

void napp_init_callbacks()
{
	for (int i = 0; i < NAPP_CALLBACKS; i++)
	{
		if (!g_callback[i])
			g_callback[i] = &napp_dummy;
	}
}

void napp_set_context(void* context)
{
	g_context = context;
}

void napp_set_callback(unsigned int index, napp_callback_t proc)
{
	if (index < NAPP_CALLBACKS)
	{
		g_callback[index] = proc;
	}
}

void napp_invoke_cb(unsigned int index)
{
	g_callback[index](g_context);
}
