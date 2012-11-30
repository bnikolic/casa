//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include "FitterGaussian.h"
#include <scimath/Functionals/Gaussian1D.h>

namespace casa {

FitterGaussian::FitterGaussian() {

}

void FitterGaussian::setPeak( double peakValue ){
	peak = peakValue;
}
void FitterGaussian::setCenter( double centerValue ){
	center = centerValue;
}
void FitterGaussian::setFWHM( double fwhmValue ){
	fwhm = fwhmValue;
}

bool FitterGaussian::doFit(){
	//Model the fit so that we can give it to the canvas for
	//drawing.
	bool fitSuccessful = true;
	yValues.resize( xValues.size());
	Gaussian1D<Float> gaussFit(peak, center, fwhm );
	for( int i = 0; i < static_cast<int>(xValues.size()); i++ ){
		yValues[i] = gaussFit.eval(&xValues[i]);
	}
	return fitSuccessful;
}

FitterGaussian::~FitterGaussian() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
