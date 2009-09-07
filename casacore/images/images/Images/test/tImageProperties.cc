//# tPagedImage.cc:  test the PagedImage class
//# Copyright (C) 1994,1995,1998,1999,2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or(at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: tPagedImage.cc 20648 2009-06-29 07:22:00Z gervandiepen $

#include <images/Images/ImageProperties.h>
#include <images/Images/FITSImage.h>
#include <casa/OS/Path.h>
#include <casa/namespace.h>


int main(Int argc, char *argv[]) {
  try {
    // just to eliminate compiler warning about argc not being used
    if (argc > 1) {
        cerr << "WARNING: This test takes no command line options, "
            << "so the options you specified will be ignored" << endl;
    }

    Path path(argv[0]);
    String dir = path.dirName();
    FITSImage fourAxesImage(
        dir + "/fixtures/tImageProperties/ngc5921.clean.fits"
    );
    FITSImage twoAxesImage(
        dir + "/fixtures/tImageProperties/ngc5921.clean.no_freq.no_stokes.fits"
    );
   
    ImageProperties fourAxesImageProps(fourAxesImage);
    ImageProperties twoAxesImageProps(twoAxesImage);
    {
        // 3 in the casa format image, but 2 in the FITS image
        AlwaysAssert(fourAxesImageProps.spectralAxisNumber() == 2, AipsError);
        AlwaysAssert(twoAxesImageProps.spectralAxisNumber() == -1, AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.nChannels() == 8, AipsError);
        AlwaysAssert(twoAxesImageProps.nChannels() == 0, AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.isChannelNumberValid(1), AipsError);
        AlwaysAssert(! fourAxesImageProps.isChannelNumberValid(10), AipsError);
        AlwaysAssert(! twoAxesImageProps.isChannelNumberValid(0), AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.spectralCoordinateNumber() == 2, AipsError);
        AlwaysAssert(twoAxesImageProps.spectralCoordinateNumber() == -1, AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.hasSpectralAxis(), AipsError);
        AlwaysAssert(! twoAxesImageProps.hasSpectralAxis(), AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.polarizationCoordinateNumber() == 1, AipsError);
        AlwaysAssert(twoAxesImageProps.polarizationCoordinateNumber() == -1, AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.hasPolarizationAxis(), AipsError);
        AlwaysAssert(! twoAxesImageProps.hasPolarizationAxis(), AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.stokesPixelNumber("I") == 0, AipsError);
        AlwaysAssert(fourAxesImageProps.stokesPixelNumber("Q") == -1, AipsError);
        AlwaysAssert(twoAxesImageProps.stokesPixelNumber("I") == -1, AipsError);
    }

    {
        AlwaysAssert(fourAxesImageProps.nStokes() == 1, AipsError);
        AlwaysAssert(twoAxesImageProps.nStokes() == 0, AipsError);
    }
    
    {
        AlwaysAssert(fourAxesImageProps.isStokesValid("I"), AipsError);
        AlwaysAssert(! fourAxesImageProps.isStokesValid("Q"), AipsError);
        AlwaysAssert(! twoAxesImageProps.isStokesValid("I"), AipsError);
    }

    {
        // TODO test image without a direction coordinate
        AlwaysAssert(fourAxesImageProps.directionCoordinateNumber() == 0, AipsError);
        AlwaysAssert(twoAxesImageProps.directionCoordinateNumber() == 0, AipsError);
    }         

    {
        // TODO test image without a direction coordinate
        AlwaysAssert(fourAxesImageProps.hasDirectionCoordinate(), AipsError);
        AlwaysAssert(twoAxesImageProps.hasDirectionCoordinate(), AipsError);
    }         

    {
        // TODO test image without a direction coordinate
        Vector<Int> directionAxesNums = fourAxesImageProps.directionAxesNumbers();
        AlwaysAssert(directionAxesNums[0] == 0, AipsError);
        AlwaysAssert(directionAxesNums[1] == 1, AipsError);

        directionAxesNums = twoAxesImageProps.directionAxesNumbers();
        AlwaysAssert(directionAxesNums[0] == 0, AipsError);
        AlwaysAssert(directionAxesNums[1] == 1, AipsError);
    }         

    {
        // TODO test image without a direction coordinate
        Vector<Int> directionShape = fourAxesImageProps.directionShape();
        AlwaysAssert(directionShape[0] == 6, AipsError);
        AlwaysAssert(directionShape[1] == 11, AipsError);

        directionShape = twoAxesImageProps.directionShape();
        AlwaysAssert(directionShape[0] == 6, AipsError);
        AlwaysAssert(directionShape[1] == 11, AipsError);
    } 

    {
        String message;
        AlwaysAssert(fourAxesImageProps.areChannelAndStokesValid(message, 1, "I"), AipsError);
        AlwaysAssert(! fourAxesImageProps.areChannelAndStokesValid(message, 15, "I"), AipsError);
        AlwaysAssert(! fourAxesImageProps.areChannelAndStokesValid(message, 1, "Q"), AipsError);
        AlwaysAssert(! twoAxesImageProps.areChannelAndStokesValid(message, 0, "I"), AipsError);
    }

    cout<< "ok"<< endl;
  } catch (AipsError x) {
    cerr << "Exception caught: " << x.getMesg() << endl;
    return 1;
  } 
    return 0;



}
