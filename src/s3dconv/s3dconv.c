#ifdef _DEBUG
	#undef _DEBUG
	#include <python.h>
	#define _DEBUG
#else
	#include <python.h>
#endif

#ifndef _DEBUG
	#pragma comment(lib, "ucrt.lib")
#pragma comment(lib, "vcruntime.lib")
#else
	#pragma comment(lib, "ucrtd.lib")
	#pragma comment(lib, "vcruntimed.lib")
#endif
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "python3.lib")

const char* GetLibraryName()
{
	static const char* name = "Scene3D Asset Conversion Library";
	return name;
}

static PyObject* Python_about(PyObject *self, PyObject *args)
{
	PySys_WriteStdout("Scene3D Asset Conversion Library v0.1\n");
	return Py_None; //PyUnicode_FromString(GetLibraryName());
}

PyMODINIT_FUNC PyInit_s3dconv(void)
{
	static PyMethodDef methods[] =
	{
		{ "about",  Python_about, METH_NOARGS, "About this module." },
		{ NULL, NULL, 0, NULL }
	};
	static struct PyModuleDef module =
	{
		PyModuleDef_HEAD_INIT, "s3dconv", /*doc*/NULL, /*state*/-1, methods
	};
	return PyModule_Create(&module);
}
