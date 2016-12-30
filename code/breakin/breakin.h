// Copyright (C) 2016 B. Nikolic / U. Cambridge.
// License: GPL3
//
// This file is not yet an offical part of CASA
//
//
// BreakIn: Break into the algorithmic kernels of CASA from Python by
// using Numba & friends. Maybe go a bit faster too.
//
// This module is pure C. All exported functions have prefix bk
#ifndef __CASA_BREAKIN__H
#define __CASA_BREAKIN__H

#ifdef __cplusplus
# define BKDECL extern "C"
#else
# define BKDECL
#endif

#include "breakin/pythonfns.h"

/// Use the "C" functions defined in this directory 
#define BKFNLOCAL(F) bk_ ## F

/// Call back out into python to get the function named F
#define BKFNPY(F)  (* ( void (*)(...) )(bk_getfn( #F ))) 

/// Change this define to switch between the Python breakin or plain C
/// functions
#define BKFN(F) BKFNPY(F)


#endif
