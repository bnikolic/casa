//# CalDescColumns.h: Calibration table cal_desc column access
//# Copyright (C) 1996,1997,1998,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_CALDESCCOLUMNS_H
#define CALIBRATION_CALDESCCOLUMNS_H

#include <casa/aips.h>
#include <measures/Measures/MFrequency.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/TableMeasColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>
#include <synthesis/CalTables/CalTable.h>
#include <msvis/MSVis/MSCalEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// ROCalDescColumns: Read-only cal_desc calibration table column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "read-only", "calibration descriptor table" and "columns".
// </etymology>
//
// <synopsis>
// The ROCalDescColumns class allows read-only access to columns
// in the cal_desc calibration sub-table. 
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-only access to cal_desc calibration sub-table columns.
// </motivation>
//
// <todo asof="11/01/01">
// (i) Deal with non-standard columns.
// </todo>

class ROCalDescColumns 
{
 public:
  // Construct from a cal_desc calibration sub-table
  ROCalDescColumns (const CalTable& calTable);

  // Default destructor
  virtual ~ROCalDescColumns() {};

  // Read-only column accessors
  const casacore::ROScalarColumn<casacore::Int>& numSpw() const {return numSpw_p;};
  const casacore::ROArrayColumn<casacore::Int>& numChan() const {return numChan_p;};
  const casacore::ROScalarColumn<casacore::Int>& numReceptors() const {return numReceptors_p;};
  const casacore::ROScalarColumn<casacore::Int>& nJones() const {return nJones_p;};
  const casacore::ROArrayColumn<casacore::Int>& spwId() const {return spwId_p;};
  const casacore::ROArrayColumn<casacore::Double>& chanFreq() const {return chanFreq_p;};
  const casacore::ROArrayMeasColumn<casacore::MFrequency>& chanFreqMeas() const
    {return chanFreqMeas_p;};
  const casacore::ROScalarColumn<casacore::Int>& measFreqRef() const {return measFreqRef_p;};
  const casacore::ROArrayColumn<casacore::Double>& chanWidth() const {return chanWidth_p;}
  const casacore::ROArrayQuantColumn<casacore::Double>& chanWidthQuant() const {
    return chanWidthQuant_p;}
  const casacore::ROArrayColumn<casacore::Int>& chanRange() const {return chanRange_p;}
  const casacore::ROArrayColumn<casacore::String>& polznType() const {return polznType_p;}
  const casacore::ROScalarColumn<casacore::String>& jonesType() const {return jonesType_p;};
  const casacore::ROScalarColumn<casacore::String>& msName() const {return msName_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROCalDescColumns() {};

  // Return a CalTable cal_desc subtable as a casacore::Table reference. 
  // Utilizes friendship relationship with class CalTable.
  const casacore::Table& calDescAsTable(const CalTable& calTable) 
    {return calTable.calDescAsTable();}

  // Attach a table column accessor
  void attach (const CalTable& calTable, casacore::ROTableColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
  void attach (const CalTable& calTable, casacore::ROTableMeasColumn& tabCol,
	       MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
  void attach (const CalTable& calTable, casacore::ROArrayQuantColumn<casacore::Double>& tabCol,
	       MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);

 private:
  // Prohibit copy constructor and assignment operator 
  ROCalDescColumns (const ROCalDescColumns&);
  ROCalDescColumns& operator= (const ROCalDescColumns&);

  // Private column accessors
  casacore::ROScalarColumn<casacore::Int> numSpw_p;
  casacore::ROArrayColumn<casacore::Int> numChan_p;
  casacore::ROScalarColumn<casacore::Int> numReceptors_p;
  casacore::ROScalarColumn<casacore::Int> nJones_p;
  casacore::ROArrayColumn<casacore::Int> spwId_p;
  casacore::ROArrayColumn<casacore::Double> chanFreq_p;
  casacore::ROArrayMeasColumn<casacore::MFrequency> chanFreqMeas_p;
  casacore::ROScalarColumn<casacore::Int> measFreqRef_p;
  casacore::ROArrayColumn<casacore::Double> chanWidth_p;
  casacore::ROArrayQuantColumn<casacore::Double> chanWidthQuant_p;
  casacore::ROArrayColumn<casacore::Int> chanRange_p;
  casacore::ROArrayColumn<casacore::String> polznType_p;
  casacore::ROScalarColumn<casacore::String> jonesType_p;
  casacore::ROScalarColumn<casacore::String> msName_p;
};

// <summary> 
// CalDescColumns: Read-write cal_desc calibration table column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration descriptor table" and "columns".
// </etymology>
//
// <synopsis>
// The CalDescColumns class allows read-write access to columns
// in the cal_desc calibration sub-table. 
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate access to cal_desc calibration sub-table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class CalDescColumns 
{
 public:
  // Construct from a calibration table
  CalDescColumns (CalTable& calTable);

  // Default destructor
  virtual ~CalDescColumns() {};

  // Read-write column accessors
  casacore::ScalarColumn<casacore::Int>& numSpw() {return numSpw_p;};
  casacore::ArrayColumn<casacore::Int>& numChan() {return numChan_p;};
  casacore::ScalarColumn<casacore::Int>& numReceptors() {return numReceptors_p;};
  casacore::ScalarColumn<casacore::Int>& nJones() {return nJones_p;};
  casacore::ArrayColumn<casacore::Int>& spwId() {return spwId_p;};
  casacore::ArrayColumn<casacore::Double>& chanFreq() {return chanFreq_p;};
  casacore::ArrayMeasColumn<casacore::MFrequency>& chanFreqMeas() {return chanFreqMeas_p;};
  casacore::ScalarColumn<casacore::Int>& measFreqRef() {return measFreqRef_p;};
  casacore::ArrayColumn<casacore::Double>& chanWidth() {return chanWidth_p;}
  casacore::ArrayQuantColumn<casacore::Double>& chanWidthQuant() {return chanWidthQuant_p;}
  casacore::ArrayColumn<casacore::Int>& chanRange() {return chanRange_p;}
  casacore::ArrayColumn<casacore::String>& polznType() {return polznType_p;}
  casacore::ScalarColumn<casacore::String>& jonesType() {return jonesType_p;};
  casacore::ScalarColumn<casacore::String>& msName() {return msName_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  CalDescColumns() {};

  // Return a CalTable cal_desc subtable as a casacore::Table reference. 
  // Utilizes friendship relationship with class CalTable.
  casacore::Table& calDescAsTable(CalTable& calTable) {return calTable.calDescAsTable();}

  // Attach a table column accessor
  void attach (CalTable& calTable, casacore::ROTableColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
  void attach (CalTable& calTable, casacore::ROTableMeasColumn& tabCol,
	       MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
  void attach (CalTable& calTable, casacore::ROArrayQuantColumn<casacore::Double>& tabCol,
	       MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);

 private:
  // Prohibit copy constructor and assignment operator 
  CalDescColumns (const CalDescColumns&);
  CalDescColumns& operator= (const CalDescColumns&);

  // Private column accessors
  casacore::ScalarColumn<casacore::Int> numSpw_p;
  casacore::ArrayColumn<casacore::Int> numChan_p;
  casacore::ScalarColumn<casacore::Int> numReceptors_p;
  casacore::ScalarColumn<casacore::Int> nJones_p;
  casacore::ArrayColumn<casacore::Int> spwId_p;
  casacore::ArrayColumn<casacore::Double> chanFreq_p;
  casacore::ArrayMeasColumn<casacore::MFrequency> chanFreqMeas_p;
  casacore::ScalarColumn<casacore::Int> measFreqRef_p;
  casacore::ArrayColumn<casacore::Double> chanWidth_p;
  casacore::ArrayQuantColumn<casacore::Double> chanWidthQuant_p;
  casacore::ArrayColumn<casacore::Int> chanRange_p;
  casacore::ArrayColumn<casacore::String> polznType_p;
  casacore::ScalarColumn<casacore::String> jonesType_p;
  casacore::ScalarColumn<casacore::String> msName_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



