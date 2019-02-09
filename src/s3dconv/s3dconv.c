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

PyMODINIT_FUNC PyModuleEntryPoint(void)
{
	static PyMethodDef methods[] =
	{
		{ "about",  PyAbout, METH_NOARGS, "About this module." },
		{ "header",  PyGetHeaderString, METH_NOARGS, "Get header string." },
        { "add_vertex",  PyAddVertex, METH_O, "Add new vertex/normal." },
		{ NULL, NULL, 0, NULL }
	};
	static struct PyModuleDef module =
	{
		PyModuleDef_HEAD_INIT, "s3dconv", /*doc*/NULL, /*state*/-1, methods
	};
	return PyModule_Create(&module);
}
