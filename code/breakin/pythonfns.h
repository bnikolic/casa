// pythonfns.h: Load functions from Python for breakin
// Copyright (C) 2016 B. Nikolic / U. Cambridge.
// License: GPL3
//
// This file is not yet an offical part of CASA
//

#include "breakin/breakin.h"

/// Initialise the breakin module. Must be called before any other
/// function in this module
BKDECL void bk_init(void);

/// Get C-callable pointer to a breakin function name. Breakin
/// functions in Python are searched for in the "breakin" Python
/// module and must contain an attribute "address" with their
/// C-callable address. Make them easily with Numba python package.
BKDECL size_t bk_getfn(const char* fname);
