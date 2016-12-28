// Copyright (C) 2016 B. Nikolic / U. Cambridge.
// License: GPL3
//
// This file is not yet an offical part of CASA

#include "synthkernels.h"

void bk_applyGradientToYLine(size_t iy,
			     double complex * convFunctions,
			     double complex * convWeights,
			     double pixXdir,
			     double pixYdir,
			     size_t convSize,
			     size_t ndishpair,
			     size_t nChan,
			     size_t  nPol)
{
  double complex phasor_y = cexp( ((int)iy-convSize/2) * pixYdir * I);
  for (size_t ix=0; ix<convSize; ++ix)
    {
        double complex phasor_x = cexp( ((int)ix-convSize/2) * pixXdir * I);
	for (size_t ipol=0; ipol<nPol; ++ipol)
	  {
	    for(size_t ichan=0; ichan<nChan; ++ichan)
	      {
		for (size_t iz=0; iz<ndishpair; ++iz)
		  {
		    size_t index=(((iz*nChan+ichan)*nPol+ipol)*convSize+iy)*convSize+ix;
		    convFunctions[index]= convFunctions[index]*phasor_x*phasor_y;
		    convWeights[index]= convWeights[index]* phasor_x*phasor_y;
		  }
	      }
	  }
    }
}
  


