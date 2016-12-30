// Synthesis kernels
// Copyright (C) 2016 B. Nikolic / U. Cambridge.
// License: GPL3
//
// This file is not yet an offical part of CASA
//

#include <memory.h>
#include <complex.h>

#include "breakin.h"


/// Apply a gradient to one horizontal line (i.e., fixed y) for a set
/// of convolution functions for multiple polarisation, channel and
/// dish pairs
BKDECL void bk_applyGradientToYLine(size_t iy,
					float complex * convFunctions,
					float complex * convWeights,
					double pixXdir,
					double pixYdir,
					size_t convSize,
					size_t ndishpair,
					size_t nChan,
					size_t  nPol);
