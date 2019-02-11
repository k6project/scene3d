#include <vecmath.h>

#include <stdio.h>

#ifdef _WIN32
    #ifdef _DEBUG
	    #undef _DEBUG
	    #include <python.h>
	    #define _DEBUG
    #else
	    #include <python.h>
    #endif
#else
    #include <Python.h>
#endif

#ifdef _WIN32
    #ifndef _DEBUG
	    #pragma comment(lib, "ucrt.lib")
        #pragma comment(lib, "vcruntime.lib")
    #else
	    #pragma comment(lib, "ucrtd.lib")
	    #pragma comment(lib, "vcruntimed.lib")
    #endif
    #pragma comment(lib, "kernel32.lib")
    #pragma comment(lib, "python3.lib")
#endif

#define NumVertexComponents 6
#define PyModuleEntryPoint  PyInit_s3dconv

typedef struct
{
    uint32_t index;
    Vec3f pos, norm;
    
} PyVertexEntry;

typedef struct
{
    FILE* file;
} PyModuleState;

static PyObject* PyAbout(PyObject *self, PyObject *args)
{
	PySys_WriteStdout("Scene3D Asset Conversion Library v0.1\n");
	return Py_None;
}

static PyObject* PyGetHeaderString(PyObject *self, PyObject *args)
{
	return PyUnicode_FromString("# This is coming from C library");
}

static PyObject* PyAddVertex(PyObject *self, PyObject *args)
{
    if (!PySequence_Check(args))
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid argument: not a list");
        return NULL;
    }
    if (PySequence_Length(args) != NumVertexComponents)
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid argument: invalid number of elements");
        return NULL;
    }
    float vertex[NumVertexComponents];
    for (int i = 0; i < NumVertexComponents; i++)
    {
        PyObject* obj = PySequence_GetItem(args, i);
        if (!PyNumber_Check(obj))
        {
            PyErr_SetString(PyExc_RuntimeError, "Invalid item: not a number");
            return NULL;
        }
        PyObject* fpObj = PyNumber_Float(obj);
        if (!fpObj)
        {
            PyErr_SetString(PyExc_RuntimeError, "Invalid item: not single-precision float");
            return NULL;
        }
        double dVal = PyFloat_AsDouble(fpObj);
        vertex[i] = (float)dVal;
    }
    //TODO consume array of floats
    //return index
    return PyLong_FromLong(0);
}

static PyObject* PyBegin(PyObject *self, PyObject *args)
{
    const char* fileName = PyUnicode_AsUTF8(args);
    PyModuleState* state = (PyModuleState*)PyModule_GetState(self);
    if (fileName && state && !state->file)
    {
        state->file = fopen(fileName, "wb");
        struct { char magick[3], type; } header = { {'S', '3', 'D'}, 'G' };
        fwrite(&header, sizeof(header), 1, state->file);
    }
    return Py_None;
}

static PyObject* PyEnd(PyObject *self, PyObject *args)
{
    PyModuleState* state = (PyModuleState*)PyModule_GetState(self);
    if (state && state->file)
    {
        fclose(state->file);
    }
    return Py_None;
}

PyMODINIT_FUNC PyModuleEntryPoint(void)
{
	static PyMethodDef methods[] =
	{
		{ "about",  PyAbout, METH_NOARGS, "About this module." },
		{ "header",  PyGetHeaderString, METH_NOARGS, "Get header string." },
        
        { "begin",  PyBegin, METH_O, "Create new export file" },
        { "add_vertex",  PyAddVertex, METH_O, "Find/add new vertex entry" },
        { "end", PyEnd, METH_NOARGS, "Close current file" },
        
		{ NULL, NULL, 0, NULL }
	};
	static struct PyModuleDef module =
	{
		PyModuleDef_HEAD_INIT, "s3dconv", NULL, sizeof(PyModuleState), methods
	};
    PyObject* mod = PyModule_Create(&module);
    PyModuleState* state = (PyModuleState*)PyModule_GetState(mod);
    memset(state, 0, sizeof(PyModuleState));
	return mod;
}
