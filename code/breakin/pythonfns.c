// pythonfns.c: Load functions from Python for breakin
// Copyright (C) 2016 B. Nikolic / U. Cambridge. License: GPL3
//
// This file is not yet an offical part of CASA

#include <Python.h>

void bk_init(void)
{
  Py_Initialize();
}

size_t bk_getfn(const char* fname)
{
  PyObject *m, *pyfn, *fnaddress;
  if(!(  m= PyImport_ImportModule("breakin") ))
    goto failed;
  if(!( pyfn = PyObject_GetAttrString(m, fname)))
    goto failed;
  if(!( fnaddress = PyObject_GetAttrString(pyfn, "address")))
    goto failed;

  return PyNumber_AsSsize_t(fnaddress, NULL);
 failed:
  PyErr_Print();
  return 0;
  // Not decrementing references here since unknown when the objects
  // will be used for C layer. Therefore breakin python functions will
  // never get garbage collected. 
}

