//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#include <casa/Inputs/Input.h>
#include <images/Images/ImageUtilities.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <casa/namespace.h>

Int main(Int argc, char *argv[]) {
	Input input(1);
	input.version("$ID:$");
	input.create("imagename");
	input.create("box", "");
	input.create("region", "");
	input.create("chans", CasacRegionManager::ALL);
	input.create("stokes", CasacRegionManager::ALL);
	input.create("mask", "");
	input.create("axis", "");
	input.create("function","");
	input.create("outname");
	input.create("overwrite", "F");

	input.readArguments(argc, argv);
	String imagename = input.getString("imagename");
	String box = input.getString("box");
	String region = input.getString("region");
	String chans = input.getString("chans");
	String stokes = input.getString("stokes");
	String mask = input.getString("mask");
	IPosition axes(1, input.getInt("axis"));
	String function = input.getString("function");
	String outname = input.getString("outname");
	Bool overwrite = input.getBool("overwrite");
	ImageInterface<Float> *myim = 0;
	LogIO mylog;
	ImageUtilities::openImage(myim, imagename, mylog);
	if (myim == 0) {
		mylog << "Unable to open image " << imagename << LogIO::EXCEPTION;
	}
	Record *regionPtr = 0;
	ImageTask::shCImFloat image(myim);
    ImageCollapser imCollapser(
		function, image, region, regionPtr, box,
		chans, stokes, mask, axes, outname,
        overwrite
    );

	imCollapser.collapse(False);

    return 0;
}



