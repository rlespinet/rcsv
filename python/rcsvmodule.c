#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <Python.h>
#include <numpy/arrayobject.h>
#include "rcsv.h"


static char module_docstring[] = "Read and write CSVs at the speed of light !";

static char read_docstring [] = "Read CSV into a numpy array";

static PyObject *rcsv_read_bind(PyObject *self, PyObject *args);


static PyMethodDef rcsv_methods[] = {
    {"read", rcsv_read_bind, METH_VARARGS, read_docstring},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef rcsv_module = {
    PyModuleDef_HEAD_INIT,
    "rcsv",                     /* name of module */
    module_docstring,           /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    rcsv_methods
};

PyMODINIT_FUNC PyInit_rcsv(void) {
    import_array();

    return PyModule_Create(&rcsv_module);
}

static void set_python_error(int code) {
    switch (code) {
    case ERR_OUT_OF_MEMORY:
        PyErr_SetString(PyExc_ValueError,
                        "Out Of Memory. Please buy more RAM.");
        break;
    case ERR_INTERNAL_ERROR:
        PyErr_SetString(PyExc_ValueError,
                        "Internal error. Please contact the author of this program.");
        break;
    case ERR_PTHREAD_ERROR:
        PyErr_SetString(PyExc_ValueError,
                        "Pthread error. Please contact the author of this program.");
        break;
    case ERR_FILE_ERROR:
        PyErr_SetString(PyExc_ValueError,
                        "File error. Failed to open the file, make sure that the path is right "
                        "and that you have read access.");
        break;
    }
}

static PyObject *rcsv_read_bind(PyObject *self, PyObject *args) {
    const char *path;
    PyObject* array = NULL;

    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;


    float *data = NULL;
    int rows = 0, cols = 0;
    int code = rcsv_read(&rows, &cols, &data, path);
    if (code != SUCCESS) {
        set_python_error(code);
        Py_RETURN_NONE;
    }

    npy_intp dims[] = {rows, cols};
    array = PyArray_SimpleNewFromData(2, dims, NPY_FLOAT, (void*) data);

    return array;
}
