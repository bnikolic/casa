//# tAntennaResponses the AntennaResponses class
//# Copyright (C) 2003,2004
//# Associated Universities, Inc. Washington DC, USA.
//# Copyright by ESO (in the framework of the ALMA collaboration)
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
//# $Id: $

#include <casa/aips.h>
#include <images/Images/AntennaResponses.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasRef.h>
#include <measures/Measures/MeasTable.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Time.h>
#include <casa/System/AipsrcValue.h>
#include <casa/BasicSL/String.h>
#include <casa/iostream.h>

#include <casa/namespace.h>

int main() {
  try {

    {
      // table creation from scratch
      
      Vector<String> bName(3);
      bName(0) = "band_1";
      bName(1) = "band_2";
      bName(2) = "band_3";

      Vector<MVFrequency> minFreq(3);
      Vector<MVFrequency> maxFreq(3);
      Vector<MVFrequency> nomFreq(3);
      minFreq(0) = MVFrequency( Quantity(10., "GHz"));
      maxFreq(0) = MVFrequency( Quantity(100., "GHz"));
      nomFreq(0) = MVFrequency( Quantity(60., "GHz"));
      minFreq(1) = MVFrequency( Quantity(100., "GHz"));
      maxFreq(1) = MVFrequency( Quantity(200., "GHz"));
      nomFreq(1) = MVFrequency( Quantity(150., "GHz"));
      minFreq(2) = MVFrequency( Quantity(200., "GHz"));
      maxFreq(2) = MVFrequency( Quantity(300., "GHz"));
      nomFreq(2) = MVFrequency( Quantity(250., "GHz"));

      Vector<AntennaResponses::FuncTypes> fTyp(3, AntennaResponses::EFP);

      Vector<String> funcName(3);
      funcName(0) = "tAntennaResponses1.in";
      funcName(1) = "tAntennaResponses2.in";
      funcName(2) = "tAntennaResponses3.in";

      Vector<uInt> funcChannel(3,0);
      
      Vector<AntennaResponses::FuncTypes> fTypB(3, AntennaResponses::VP);

      Vector<String> funcNameB(3);
      funcNameB(0) = "tAntennaResponses1B.in";
      funcNameB(1) = "tAntennaResponses2B.in";
      funcNameB(2) = "tAntennaResponses3B.in";


      AntennaResponses aR; // empty table in memory

      uInt ui = 0;

      AlwaysAssert(aR.putRow(ui,"ALMA", 0, bName, minFreq, maxFreq, fTyp, funcName, funcChannel, nomFreq,
			     "DV", MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
			     MDirection(Quantity( 0., "deg"),
					Quantity(45., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( -10., "deg"), 
					Quantity(40., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( 10., "deg"),
					Quantity(50., "deg"), 
					MDirection::AZEL)
			     ), AipsError);

      ui = 1;

      AlwaysAssert(aR.putRow(ui,"ALMA", 0, bName, minFreq, maxFreq, fTypB, funcNameB, funcChannel, nomFreq,
			     "DV", MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
			     MDirection(Quantity( 0., "deg"),
					Quantity(45., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( -10., "deg"), 
					Quantity(40., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( 10., "deg"),
					Quantity(50., "deg"), 
					MDirection::AZEL)
			     ), AipsError);

      aR.create("testAntennaResponses_tmp.dat"); // write table to disk

      aR.init(); // reset to empty table

      ui = 0;

      AlwaysAssert(aR.putRow(ui,"ACA", 0, bName, minFreq, maxFreq, fTyp, funcName, funcChannel, nomFreq,
			     "DV", MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
			     MDirection(Quantity( 0., "deg"),
					Quantity(45., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( -10., "deg"), 
					Quantity(40., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( 10., "deg"),
					Quantity(50., "deg"), 
					MDirection::AZEL)
			     ), AipsError);

      ui = 2; // test setting of row number

      AlwaysAssert(aR.putRow(ui,"ACA", 0, bName, minFreq, maxFreq, fTypB, funcNameB, funcChannel, nomFreq,
			     "DV", MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
			     MDirection(Quantity( 0., "deg"),
					Quantity(45., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( -10., "deg"), 
					Quantity(40., "deg"), 
					MDirection::AZEL),
			     MDirection(Quantity( 10., "deg"),
					Quantity(50., "deg"), 
					MDirection::AZEL)
			     ), AipsError);

      AlwaysAssert(ui==1, AipsError); // ui should have been reset to 1

      aR.create("testAntennaResponsesACA_tmp.dat"); // write second table to disk

    }

    // initialisation

    AntennaResponses aR("testAntennaResponses_tmp.dat");

    AlwaysAssert(aR.isInit("testAntennaResponses_tmp.dat"), AipsError);
    AlwaysAssert(!aR.isInit("testAntennaResponsesACA_tmp.dat"), AipsError);

    AlwaysAssert(!aR.append("testAntennaResponses_tmp.dat"), AipsError);
    AlwaysAssert(aR.append("testAntennaResponsesACA_tmp.dat"), AipsError);
    AlwaysAssert(aR.isInit("testAntennaResponsesACA_tmp.dat"), AipsError);
    AlwaysAssert(!aR.append("testAntennaResponsesACA_tmp.dat"), AipsError);

    // unsuccessful access

    AlwaysAssert(aR.init("testAntennaResponses_tmp.dat"), AipsError);

    String theImageName;
    uInt theImageChannel;
    MFrequency theNomFreq;
    AntennaResponses::FuncTypes theFType;

    AntennaResponses::FuncTypes requFType = AntennaResponses::EFP;

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ACA", // wrong obs.
				  MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO),
				  requFType, "DV"),
		 AipsError);

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ALMA", 
				  MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
				  MFrequency( Quantity(600., "GHz"), MFrequency::TOPO), // wrong freq
				  requFType, "DV"),
		 AipsError);

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ALMA", 
				  MEpoch(MVEpoch(Quantity(49999., "d")), MEpoch::UTC), // wrong time
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO), 
				  requFType, "DV"), 
		 AipsError);

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ALMA", 
				  MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC), 
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO),
				  requFType, "DV",
				  MDirection(Quantity( 0., "deg"),
					     Quantity(80., "deg"), 
					     MDirection::AZEL)), // wrong center
		 AipsError);

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ALMA", 
				  MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC), 
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO),
				  requFType, "XY", // wrong antenna type
				  MDirection(Quantity( 0., "deg"),
					     Quantity(45., "deg"), 
					     MDirection::AZEL)), 
		 AipsError);

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ALMA", 
				  MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC), 
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO),
				  requFType, "DV", 
				  MDirection(Quantity( 0., "deg"),
					     Quantity(45., "deg"), 
					     MDirection::AZEL), 
				  "rec typ B"), // wrong receiver type
		 AipsError);

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ALMA", 
				  MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC), 
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO),
				  requFType, "DV", 
				  MDirection(Quantity( 0., "deg"),
					     Quantity(45., "deg"), 
					     MDirection::AZEL), 
				  "",
				  1), // wrong beam number
		 AipsError);

    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				 "ALMA", 
				  1, // wrong beam id
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO)),
		 AipsError);


    AlwaysAssert(!aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				  "ALMA", 
				  MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
				  MFrequency( Quantity(160., "GHz"), MFrequency::TOPO),
				  AntennaResponses::AIF), // wrong image type
		 AipsError);

    // successful access

    AlwaysAssert(aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				 "ALMA", 
				 MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC), 
				 MFrequency( Quantity(250., "GHz"), MFrequency::TOPO),
				 requFType, "DV", 
				 MDirection(Quantity( 0., "deg"),
					    Quantity(45., "deg"), 
					    MDirection::AZEL), 
				 "",
				 0), 
		 AipsError);

    AlwaysAssert(aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				 "ALMA", 
				 0,
				 MFrequency( Quantity(250., "GHz"), MFrequency::TOPO)), 
		 AipsError);

    AlwaysAssert(aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				 "ALMA", 
				 MEpoch(MVEpoch(Quantity(50000., "d")), MEpoch::UTC),
				 MFrequency( Quantity(160., "GHz"), MFrequency::TOPO),
				 AntennaResponses::VP),
		 AipsError);

    AlwaysAssert(aR.append("testAntennaResponsesACA_tmp.dat"), AipsError);

    AlwaysAssert(aR.getImageName(theImageName, theImageChannel, theNomFreq, theFType,
				 "ACA", 
				 0,
				 MFrequency( Quantity(250., "GHz"), MFrequency::TOPO)), 
		 AipsError);

  } 
  catch (AipsError x) {
    cerr << x.getMesg() << endl;
    cout << "FAIL" << endl;
    return 1;
  } 
  catch (...) {
    cerr << "Exception not derived from AipsError" << endl;
    cout << "FAIL" << endl;
    return 2;
  };

  cout << "OK" << endl;
  return 0;
  
}
