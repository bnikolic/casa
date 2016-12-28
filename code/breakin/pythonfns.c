// pythonfns.c: Load functions from Python for breakin
// Copyright (C) 2016 B. Nikolic / U. Cambridge. License: GPL3
//
// This file is not yet an offical part of CASA

#include <Python.h>

void breakin_init(void)
{
  Py_Initialize();
}

size_t breakin_getfn(const char* fname)
{
  PyObject *m = PyImport_ImportModule("breakin");
  PyObject *pyfn = PyObject_GetAttrString(m, fname);
  PyObject *fnaddress = PyObject_GetAttrString(pyfn, "address");

  return PyNumber_AsSsize_t(fnaddress, NULL);

  // Not decrementing references here since unknown when the objects
  // will be used for C layer. Therefore breakin python functions will
  // never get garbage collected. 
}

