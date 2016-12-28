// pythonfns.h: Load functions from Python for breakin
// Copyright (C) 2016 B. Nikolic / U. Cambridge.
// License: GPL3
//
// This file is not yet an offical part of CASA
//


/// Initialise the breakin module. Must be called before any other
/// function in this module
void breakin_init(void);

/// Get C-callable pointer to a breakin function name. Breakin
/// functions in Python are searched for in the "breakin" Python
/// module and must contain an attribute "address" with their
/// C-callable address. Make them easily with Numba python package.
size_t breakin_getfn(const char* fname);
