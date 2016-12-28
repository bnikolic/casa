// Synthesis kernels
// Copyright (C) 2016 B. Nikolic / U. Cambridge.
// License: GPL3
//
// This file is not yet an offical part of CASA
//

#include <memory.h>
#include <complex.h>

/// Apply a gradient to a set of convolution functions all with same
/// y-subsampling value but iterate over x subsampling, polarisation,
/// channel and dish pairs
void bk_applyGradientToYLine(size_t iy,
			     double complex * convFunctions,
			     double complex * convWeights,
			     double pixXdir,
			     double pixYdir,
			     size_t convSize,
			     size_t ndishpair,
			     size_t nChan,
			     size_t  nPol);
