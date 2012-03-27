//# CalSet.cc: Implementation of Calibration parameter cache
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#include <synthesis/CalTables/CTGlobals.h>
#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTIter.h>
#include <synthesis/CalTables/RIorAParray.h>

#include <casa/Arrays.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


void smoothCT(NewCalTable ct,
	      const String& smtype,
	      const Double& smtime,
	      Vector<Int> selfields) {

  // half-width
  Double thw(smtime/2.0);

  // Workspace
  Vector<Double> times;
  Vector<Float> p,newp;
  Vector<Bool> pOK, newpOK;

  Cube<Float> fpar;
  Cube<Bool> fparok;

  Vector<Bool> mask;

  IPosition blc(3,0,0,0), fblc(3,0,0,0);
  IPosition trc(3,0,0,0), ftrc(3,0,0,0);
  IPosition vec(1,0);

  Block<String> cols(4);
  cols[0]="SPECTRAL_WINDOW_ID";
  cols[1]="FIELD_ID";
  cols[2]="ANTENNA1";
  cols[3]="ANTENNA2";
  CTIter ctiter(ct,cols);

  while (!ctiter.pastEnd()) {

    Int nSlot=ctiter.nrow();
    Int ifld=ctiter.thisField();

    // Only if more than one slot in this spw _AND_
    //  field is among those requested (if any)
    if (nSlot>1 && 
	(selfields.nelements()<1 || anyEQ(selfields,ifld))) {

      Int ispw=ctiter.thisSpw();
      
      vec(0)=nSlot;
      trc(2)=ftrc(2)=nSlot-1;

      times.assign(ctiter.time());

      newpOK.resize(nSlot);

      // Extract Float info
      fpar.assign(ctiter.casfparam("AP"));
      fparok.assign(!ctiter.flag());
      IPosition fsh(fpar.shape());

      // For each channel
      for (int ichan=0;ichan<fsh(1);++ichan) {
	blc(1)=trc(1)=fblc(1)=ftrc(1)=ichan;
	// For each param (pol)
	for (Int ipar=0;ipar<fsh(0);++ipar) {
	  blc(0)=trc(0)=ipar;
	  fblc(0)=ftrc(0)=ipar/2;
	  
	  // Reference slices of par/parOK
	  p.reference(fpar(blc,trc).reform(vec));
	  newp.assign(p);
	  pOK.reference(fparok(fblc,ftrc).reform(vec));
	  newpOK.assign(pOK);

       /*
	    cout << ispw << " "
		 << ichan << " "
		 << ipar << " "
		 << "p.shape() = " << p.shape() << " "
		 << "pOK.shape() = " << pOK.shape() << " "
		 << endl;
       */

	    
	  Vector<Bool> mask;
	  for (Int i=0;i<nSlot;++i) {
	    // Make mask
	    mask = pOK;
	    mask = (mask && ( (times >  (times(i)-thw)) && 
			      (times <= (times(i)+thw)) ) );

	    // Avoid explicit zeros, for now
	    //	    mask = (mask && amp>=FLT_MIN);


	    //cout << "    " << ifld << " " << i << " " << idx(i) << " ";
	    //for (Int j=0;j<mask.nelements();++j)
	    //  cout << mask(j);
	    //cout << endl;
	    
	    if (ntrue(mask)>0) {
	      if (smtype=="mean") {
		newp(i)=mean(p(mask));
	      }
	      else if (smtype=="median") {
		newp(i)= median(p(mask),False);
	      }
	      newpOK(i)=True;
	    }
	    else 
	      newpOK(i)=False;
	    
	  } // i
	  // keep new ok info
	  p=newp;
	  pOK=newpOK;
	} // ipar
      } // ichan

      // Put info back
      ctiter.setcparam(RIorAPArray(fpar).c());
      ctiter.setflag(!fparok);

    } // nSlot>1

    ctiter.next();
  } // ispw
	
}

} //# NAMESPACE CASA - END
