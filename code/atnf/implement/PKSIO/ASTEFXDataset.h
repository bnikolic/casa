//#---------------------------------------------------------------------------
//# ASTEFXDataset.h: Class for ASTE data header.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
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
//# $Id$
//#---------------------------------------------------------------------------
//# Original: 2008/10/30, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef ASTE_FX_DATASET_H
#define ASTE_FX_DATASET_H

#define ASTE_ARYMAX_FX 12

#include <atnf/PKSIO/NRODataset.h>

#include <string>

using namespace std ;

// <summary>
// Class specific for ASTE-FX data header.
// </summary>

class ASTEFXDataset : public NRODataset
{
 public:
  // constructor
  ASTEFXDataset( string name ) ;

  // destructor
  virtual ~ASTEFXDataset() ;

  // data initialization 
  virtual void initialize() ;

  // fill header from file 
  virtual int fillHeader() ;

 protected:
  // fill header information
  virtual int fillHeader( int sameEndian ) ;
} ;


#endif /* ASTE_FX_DATASET_H */
