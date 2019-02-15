#include <vecmath.h>

#include <stdio.h>

#ifdef _WIN32
    #ifdef _DEBUG
	    #undef _DEBUG
	    #include <Python.h>
	    #define _DEBUG
    #else
	    #include <Python.h>
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

#define MODULE_ENTRY_POINT  PyInit_s3dconv
#define MODULE_MAX_VERTS    65536

typedef struct
{
    uint32_t index;
    Vec3f pos, norm;
} VertexEntry;

typedef struct  
{
	uint32_t One, Two, Three;
} FaceEntry;

typedef struct
{
    VertexEntry* Entry;
    uint32_t Left, Right;
} VertexMap;

int VertexEntry_Compare(const VertexEntry* a, const VertexEntry* b)
{
    float diff = 0.f;
    float aVal[] = { a->pos.x, a->pos.y, a->pos.z, a->norm.x, a->norm.y, a->norm.z };
    float bVal[] = { b->pos.x, b->pos.y, b->pos.z, b->norm.x, b->norm.y, b->norm.z };
    for (int i = 0; i < 6; i++)
    {
        float sub = aVal[i] - bVal[i];
        if (fabsf(sub) - fabsf(diff) > 0.0001f)
        {
            diff = sub;
        }
    }
    return (diff < -0.0001f) ? -1 : ( (diff > 0.0001f) ? 1 : 0 );
}

VertexMap* VertexMap_Find(VertexMap* map, const VertexEntry* ve, uint32_t* count)
{
    VertexMap* item = map;
    while (item->Entry)
    {
        const VertexEntry* other = item->Entry;
        int key = VertexEntry_Compare(ve, other);
        if (key != 0)
        {
            uint32_t idx = (key > 0) ? item->Right : item->Left;
            if (idx == 0) // if no element bound to selected branch
            {
                uint32_t newIdx = *count;
                VertexMap* newItem = &map[newIdx];
                newItem->Entry = NULL;
                newItem->Left = 0;
                newItem->Right = 0;
                *count += 1;
                if (key > 0)
                {
                    item->Right = *count;
                }
                else
                {
                    item->Left = *count;
                }
                item = newItem;
            }
            else
            {
                item = &map[idx - 1];
            }
            continue;
        }
        // found it
        break;
    }
    return item;
}

void VertexEntry_SerializeBin(FILE* fp, const VertexEntry* ve)
{
    Vec4f attr[] =
    {
        {ve->pos.x, ve->pos.y, ve->pos.z, 0.f},
        {ve->norm.x, ve->norm.y, ve->norm.z, 0.f}
    };
    fwrite(attr, sizeof(Vec4f), 2, fp);
}

void VertexEntry_SerializeTxt(FILE* fp, const VertexEntry* ve)
{
    float items[] =
    {
        ve->pos.x, ve->pos.y, ve->pos.z, 0.f,
        ve->norm.x, ve->norm.y, ve->norm.z, 0.f
    };
    for (int i = 0; i < 8; i++)
    {
        fprintf(fp, "%0.5f ", items[i]);
    }
    fprintf(fp, "\n");
}

void FaceEntry_SerializeTxt(FILE* fp, const FaceEntry* fe)
{
	fprintf(fp, "%u %u %u\n", fe->One, fe->Two, fe->Three);
}

typedef struct
{
    uint32_t Count, Stride;
} BufferLayout;

void BufferLayout_SerializeBin(FILE* fp, const BufferLayout* bl)
{
    fwrite(bl, sizeof(BufferLayout), 1, fp);
}
void BufferLayout_SerializeTxt(FILE* fp, const BufferLayout* bl)
{
    fprintf(fp, "%u %u\n", bl->Count, bl->Stride);
}

typedef struct
{
    FILE* file;
    BufferLayout VertexData, IndexData;
    VertexEntry Vertices[MODULE_MAX_VERTS];
    VertexMap VertexMapRoot[MODULE_MAX_VERTS];
	FaceEntry Faces[MODULE_MAX_VERTS];
    void(*WriteLayout)(FILE*, const BufferLayout*);
    void(*WriteVertex)(FILE*, const VertexEntry*);
	void(*WriteFace)(FILE*, const FaceEntry*);
} PyModuleState;

#define PYMODULE_GET_STATE(v) \
PyModuleState* v = (PyModuleState*)PyModule_GetState(self); \
if (!v) {PyErr_SetString(PyExc_RuntimeError, "Failed to get module state"); return NULL;}

static PyObject* PyAddVertex(PyObject *self, PyObject *args)
{
    VertexEntry entry = {0};
    PYMODULE_GET_STATE(state);
    if (!PySequence_Check(args))
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid argument: not a list");
        return NULL;
    }
    if (PySequence_Length(args) != 6)
    {
        PyErr_SetString(PyExc_RuntimeError, "Invalid argument: invalid number of elements");
        return NULL;
    }
    for (int i = 0; i < 6; i++)
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
        if (i > 2)
        {
            entry.norm.ptr[i - 3] = (float)dVal;
        }
        else
        {
            entry.pos.ptr[i] = (float)dVal;
        }
    }
    uint32_t index = state->VertexData.Count;
    uint32_t newCount = state->VertexData.Count;
    VertexMap* mapEntry = VertexMap_Find(state->VertexMapRoot, &entry, &newCount);
    if (mapEntry->Entry == NULL)
    {
        state->Vertices[index] = entry;
        state->Vertices[index].index = index;
        mapEntry->Entry = &state->Vertices[index];
        ++state->VertexData.Count;
    }
    else
    {
        index = mapEntry->Entry->index;
    }
    return PyLong_FromUnsignedLong(index);
}

static PyObject* PyAddFace(PyObject *self, PyObject *args)
{
	PYMODULE_GET_STATE(state);
	uint32_t idx = state->IndexData.Count / 3;
	if (!PySequence_Check(args))
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid argument: not a list");
		return NULL;
	}
	if (PySequence_Length(args) != 3)
	{
		PyErr_SetString(PyExc_RuntimeError, "Invalid argument: invalid number of elements");
		return NULL;
	}
	uint32_t* items[] = { &state->Faces[idx].One, &state->Faces[idx].Two, &state->Faces[idx].Three };
	for (int i = 0; i < 3; i++)
	{
		PyObject* obj = PySequence_GetItem(args, i);
		if (!PyNumber_Check(obj))
		{
			PyErr_SetString(PyExc_RuntimeError, "Invalid item: not a number");
			return NULL;
		}
		PyObject* lObj = PyNumber_Long(obj);
		if (!lObj)
		{
			PyErr_SetString(PyExc_RuntimeError, "Invalid item: not a long integer");
			return NULL;
		}
		*items[i] = PyLong_AsUnsignedLong(lObj) & UINT32_MAX;
	}
	state->IndexData.Count += 3;
	return Py_None;
}

static PyObject* PyBegin(PyObject *self, PyObject *args)
{
    PYMODULE_GET_STATE(state);
	memset(state, 0, sizeof(PyModuleState));
    const char* fileName = PyUnicode_AsUTF8(args);
    if (fileName && state && !state->file)
    {
        state->file = fopen(fileName, "w");
		if (state->file)
		{
			fprintf(state->file, "Scene3D v0.1\n");
			state->WriteLayout = &BufferLayout_SerializeTxt;
			state->WriteVertex = &VertexEntry_SerializeTxt;
			state->WriteFace = &FaceEntry_SerializeTxt;
			state->VertexData.Stride = 2;
			state->IndexData.Stride = 4;
		}
    }
    return Py_None;
}

static PyObject* PyEnd(PyObject *self, PyObject *args)
{
    PYMODULE_GET_STATE(state);
    if (state && state->file)
    {
        state->WriteLayout(state->file, &state->VertexData);
        for (uint32_t i = 0; i < state->VertexData.Count; i++)
        {
            state->WriteVertex(state->file, &state->Vertices[i]);
        }
        state->WriteLayout(state->file, &state->IndexData);
		for (uint32_t i = 0; i < (state->IndexData.Count / 3); i++)
		{
			state->WriteFace(state->file, &state->Faces[i]);
		}
        fclose(state->file);
    }
    return Py_None;
}

PyMODINIT_FUNC MODULE_ENTRY_POINT(void)
{
	static PyMethodDef methods[] =
	{
        { "begin",  PyBegin, METH_O, "Create new export file" },
        { "add_vertex",  PyAddVertex, METH_O, "Find/add new vertex entry" },
		{ "add_face",  PyAddFace, METH_O, "Find/add new face entry" },
        { "end", PyEnd, METH_NOARGS, "Close current file" },
		{ NULL, NULL, 0, NULL }
	};
	static struct PyModuleDef module =
	{
		PyModuleDef_HEAD_INIT, "s3dconv", NULL, sizeof(PyModuleState), methods
	};
    PyObject* mod = PyModule_Create(&module);
	return mod;
}
