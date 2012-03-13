
// -----------------------------------------------------------------------------

/*

CalAnalysis.h

Description:
------------
This header file contains definitions for the CalStats class.

Classes:
--------
CalAnalysis - This class acts as the interface between the ROCTIter and CalStats
              classes.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.  Error checking added.
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.

*/

// -----------------------------------------------------------------------------
// Start of define macro to prevent multiple loading
// -----------------------------------------------------------------------------

#ifndef CAL_ANALYSIS_H
#define CAL_ANALYSIS_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <casa/BasicSL/String.h>

#include <iostream>
#include <sstream>
using namespace std;

#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>

#include <casa/Containers/Block.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayMath.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableProxy.h>

#include <casa/Containers/Record.h>
#include <casa/Containers/RecordField.h>

#include <casa/Utilities/GenSort.h>

#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTIter.h>

#include <calanalysis/CalAnalysis/CalStats.h>
#include <calanalysis/CalAnalysis/CalStatsDerived.h>
#include <calanalysis/CalAnalysis/CalStatsFitter.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalAnalysis class definition
// -----------------------------------------------------------------------------

/*

CalAnalysis

Description:
------------
This class acts as the interface between the ROCTIter and CalAnalysis classes.

In a nutshell:
--------------
* The constructor gets the information from the new format calibration table.
* The stats<T>() function calculates statistics (the type depends on T) and
  returns the results.
  - The user can specify the field(s), antenna(s), time range, feeds, spectral
    windows, frequencies, and the second iteration axis for the CalStats class
    (time or frequency).
  - The inputs are checked and fixed, if possible.
  - The iteration loop goes over field, antenna1, and antenna2.  If a set of
    field, antenna1 and antenna2 numbers from an iteration is consistent with
    the inputs, statistics are calculated.  This is not the most efficient way,
    but the NewCalTable class doesn't have another way to access data and the
    time for each iteration is very fast.
  - For each iteration, the dimensions of the data, data error, and flag cubes
    provided by ROCTIter are feed x frequency(spw) x row(spw,time).  This shape
    is not useful for calculating statistics with CalStats, so the parse<T>()
    function slices and dices the cubes into feed x frequency x time.
  - The parsed cubes are further refined by the select<T>() function to include
    only the feeds, frequencies, and times selected by the input parameters.
  - The resulting cubes are fed to the CalStats class and its stats<T>()
    function calculates the desired statistics which are stored in a vector
    of OUTPUT<T> instances.

Nested classes:
---------------
OUTPUT<T> - This nested class contains the outputs for the
            CalAnalysis::stats<T>() template member function.

Class public member functions:
------------------------------
CalAnalysis  - This constructor gets information from the new format calibration
               table for further processing by the stats<T>() function.
~CalAnalysis - This destructor deallocates the internal memory of an instance.
tableType    - This function returns the table type (Complex or Float).
polBasis     - This function returns the polarization basis (linear, circular,
               or scalar).

Class template public stats member functions:
---------------------------------------------
stats<T> - This member function is the main user interface for calculating the
           statistics for all iterations.  Allowed T: CalStats::NONE only
           returns the input data, CalStatsFitter::FIT calculates fit
           statistics, and CalStatsHist::HIST calculates histogram statistics.

Class template public static member functions:
----------------------------------------------
exists<T>       - This member function determines whether a value appears in a
                  vector.
uniqueNoSort<T> - This member function returns an unsorted and unique vector
                  from an input vector.

Class private member functions:
-------------------------------
feed        - This member function checks the input feed vector and returns the
              fixed feed vector.
time        - This member function checks the time range and returns the
              corresponding time vector.
spw_channel - This member functions checks the input spectral window and channel
              vectors and returns the fixed spectral window and channel vectors.
freq        - This member function creates the total frequency vector based on
              the spectral window and channel vectors.

Class template private member functions:
----------------------------------------
parse<T>  - This member function reshapes the cubes provided by class
            ROCTIter to dimensions required by class CalStats.
select<T> - This member function selects the desired feeds, frequencies, and
            times from an input cube.

Class protected member functions:
---------------------------------
CalAnalysis - This default constructor is unused by this class and unavailable
              when an instance is created.
CalAnalysis - This copy constructor is unused by this class and unavailable when
              an instance is created.
operator=   - This operator= function is unused by this class and unavailable
              when an instance is created.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version created with public member functions CalAnalysis()
              (generic), ~CalAnalysis(), tableType(), polBasis(); template
              public stats member function stats<T>(); class template public
              static functions exists<T>() and uniqueNoSort<T>(); private member
              functions feed(), time(), spw_channel(), freq(); template private
              member functions parse(), and select<T>(); and protected member
              functions CalAnalysis() (default), CalAnalysis() (copy), and
              operator=().
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.  Added the RAP enum.

*/

// -----------------------------------------------------------------------------

class CalAnalysis {

  public:

    // Real/Amplitude/Phase enums.
    typedef enum RAP {
      INIT=-1, REAL=0, AMPLITUDE, PHASE
    } RAP;

    // OUTPUT nested class (allowed T: CalStats::NONE, CalStatsFitter::FIT,
    // or CalStatsHist::HIST), used to hold the vector output of stats<T>().
    template <typename T>
    class OUTPUT {
      public:
        uInt uiFieldID;
        uInt uiAntenna1;
        uInt uiAntenna2;
        Matrix<CalStats::OUT<T> > oOut;
    };

    // Generic constructor
    CalAnalysis( const String& oTableName );

    // Destructor
    virtual ~CalAnalysis( void );

    // Get main table keywords
    String& tableType( const String& oTableName );
    String& polBasis( const String& oTableName );

    // Calculate statistics for the specified fields, antennas, time range,
    // feeds, spectral windows, and channels (allowed T: CalStats::NONE gets
    // data without calculating statistics, CalStatsFitter::FIT calculates fits,
    // and CalStatsHist::HIST calculates histogram statistics).
    template <typename T>
    Vector<OUTPUT<T> >& stats( const Vector<Int>& oFieldIn,
        const Vector<Int>& oAntenna1In, const Vector<Int>& oAntenna2In,
        const Double& dStartTimeIn, const Double& dStopTimeIn,
        const Vector<String>& oFeedIn, const Vector<Int>& oSPWIn,
        const Vector<Int>* const aoChannelIn,
        const CalStats::AXIS& eAxisIterUserID, const RAP& eRAP,
        const CalStats::ARG<T>& oArg );

    // Function to determine whether a value is present in an array
    template <typename T>
    static Bool& exists( const T& tValue, const Vector<T>& oValue );

    // Function to return unsorted unique values of a vector
    template <typename T>
    static Vector<T>& uniqueNoSort( const Vector<T>& oVector );

  private:

    NewCalTable* poNCT;
    ROCTIter* poNCTIter;

    String oTableType;
    String oPolBasis;
    
    uInt uiNumFeed;
    Vector<String> oFeed;

    uInt uiNumTime;
    Vector<Double> oTime;

    uInt uiNumSPW;
    Vector<Int> oSPW;

    uInt uiNumFreq;
    Vector<uInt> oNumFreq;
    Vector<Double> oFreq;

    // These functions check the inputs against possible values and return
    // unsorted unique values
    Bool feed( const Vector<String>& oFeedIn, Vector<String>& oFeedOut ) const;
    Bool time( const Double& dStartTimeIn, const Double& dStopTimeIn,
        Vector<Double>& oTimeOut ) const;
    Bool spw_channel( const Vector<Int>& oSPWIn,
        const Vector<Int>* const aoChannelIn, Vector<Int>& oSPWOut,
        Vector<Int>* aoChannelOut ) const;
    Bool freq( const Vector<Int>& oSPWIn, const Vector<Int>* const aoChannelIn,
        Vector<Double>& oFreqOut ) const;

    // This function sorts the input feed x frequency(spw) x row(spw,time) cube
    // (from ROCTIter) to feed x frequency x time (for CalStats)
    template <typename T>
    Cube<T>& parse( const Cube<T>& oCubeIn ) const;

    // This function takes the parsed cube and selects the feeds, frequencies,
    // and times
    template <typename T>
    Cube<T>& select( const Cube<T>& oCubeIn, const Vector<String>& oFeedOut,
        const Vector<Double>& oFreqOut, const Vector<Double>& oTimeOut ) const;

  protected:

    // Unused constructors
    CalAnalysis( void );
    CalAnalysis( const CalAnalysis& oCalAnalysis );

    // Unused operator=() function
    CalAnalysis& operator=( const CalAnalysis& oCalAnalysis );

};

// -----------------------------------------------------------------------------
// End of CalAnalysis class definition
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis::stats<T> template public fit member function
// -----------------------------------------------------------------------------

/*

CalAnalysis::stats<T>

Description:
------------
This member function is the main user interface for calculating the statistics
for all iterations.  Allowed T: CalStats::NONE only returns the input data,
CalStatsFitter::FIT calculates fit statistics, and CalStatsHist::HIST calculates
histogram statistics.

NB: This class assumes that the inputs are formulated correctly by a separate
calibration selection function or class.

NB: Scans are not used by this class.  This class assumes that scans have
already been converted to times.

NB: Iteration is used, since NewCalTable does not allow selecting specific data
directly.  It is not the most efficient way to get specific data, but given the
typical file sizes speed will not be an issue.

NB: The stats<T>() function calculates statistics (the type depends on T) and
    returns the results.
  - The user can specify the field(s), antenna(s), time range, feeds, spectral
    windows, frequencies, and the second iteration axis for the CalStats class
    (time or frequency).
  - The inputs are checked and fixed, if possible.
  - The iteration loop goes over field, antenna1, and antenna2.  If a set of
    field, antenna1 and antenna2 numbers from an iteration is consistent with
    the inputs, statistics are calculated.  This is not the most efficient way,
    but the ROCTIter class doesn't have another way to access data and the time
    for each iteration is very fast.
  - For each iteration, the dimensions of the data, data error, and flag cubes
    provided by ROCTIter are feed x frequency(spw) x row(spw,time).  This shape
    is not useful for calculating statistics with CalStats, so the parse<T>()
    function slices and dices the cubes into feed x frequency x time.
  - The parsed cubes are further refined by the select<T>() function to include
    only the feeds, frequencies, and times selected by the input parameters.
  - The resulting cubes are fed to the CalStats class and its stats<T>()
    function calculates the desired statistics which are stored in a vector
    of OUTPUT<T> instances.

Inputs:
-------
oFieldIn        - This reference to a Vector<Int> instance contains the field
                  IDs.
oAntenna1In     - This reference to a Vector<Int> instance contains the antenna
                  1 IDs.  This vector must be the same length as oAntenna2In,
                  i.e., the vectors together comprise the baselines.
oAntenna2In     - This reference to a Vector<Int> instance contains the antenna
                  2 IDs.  This vector must be the same length as oAntenna1In,
                  i.e., the vectors together comprise the baselines.
dStartTimeIn    - This reference to a Double variable contains the start time.
dStopTimeIn     - This reference to a Double variable contains the stop time.
oFeedIn         - This reference to a Vector<String> instance contains the feeds
                  ("R" and "L" for circular, "X" and "Y" for linear, "S" for
                  scalar).
oSPWIn          - This reference to a Vector<Int> instance contains the spectral
                  window IDs.
aoChannelIn     - This array of Vector<Int> instances containing the channel
                  numbers.  Each element of the array corresponds to an element
                  of oSPWIn.
eAxisIterUserID - This reference to a CalStats::AXIS enum contains either the
                  FREQUENCY or TIME iteration axes (user defined).
eRAP            - This reference to a CalAnalysis::RAP enum contains either
                  REAL, AMPLITUDE, or PHASE.
oArg<T>         - This reference to a CalStats::ARG<T> instance contains the
                  extra input parameters.

Outputs:
--------
The reference to the Vector<CalAnalysis::OUTPUT<T> > instance containing the
statistics, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.  Added a RAP enum as an input
              parameter.

*/

// -----------------------------------------------------------------------------

template <typename T>
Vector<CalAnalysis::OUTPUT<T> >& CalAnalysis::stats(
    const Vector<Int>& oFieldIn, const Vector<Int>& oAntenna1In,
    const Vector<Int>& oAntenna2In, const Double& dStartTimeIn,
    const Double& dStopTimeIn, const Vector<String>& oFeedIn,
    const Vector<Int>& oSPWIn, const Vector<Int>* const aoChannelIn,
    const CalStats::AXIS& eAxisIterUserID, const RAP& eRAP,
    const CalStats::ARG<T>& oArg ) {

  // Initialize the output vector containing statistics versus field ID, antenna
  // 1, and antenna 2

  uInt uiNumOutput = 0;

  Vector<CalAnalysis::OUTPUT<T> >* poOutput =
      new Vector<CalAnalysis::OUTPUT<T> >();


  // Check the inputs

  if ( oAntenna1In.nelements() != oAntenna2In.nelements() ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Antenna ID vectors have different lengths"
        << LogIO::POST;
    return( *poOutput );
  }

  if ( dStartTimeIn > dStopTimeIn ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Start time is larger than the stop time"
        << LogIO::POST;
    return( *poOutput );
  }

  if ( eAxisIterUserID != CalStats::FREQUENCY &&
       eAxisIterUserID != CalStats::TIME ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "User-defined iteration axis must be frequency/time"
        << LogIO::POST;
    return( *poOutput );
  }


  // Check the feeds, time range, spectral windows, and channels and create
  // the temporary vectors.  The temporary total frequency vector is also
  // created.

  Vector<String> oFeedTemp = Vector<String>();
  Bool bFeed = feed( oFeedIn, oFeedTemp );
  if ( !bFeed ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid feed ID(s)" << LogIO::POST;
    return( *poOutput );
  }

  Vector<Double> oTimeTemp = Vector<Double>();
  Bool bTime = time( dStartTimeIn, dStopTimeIn, oTimeTemp );
  if ( !bTime ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid time range" << LogIO::POST;
    return( *poOutput );
  }

  Vector<Int> oSPWTemp = Vector<Int>();
  Vector<Int>* aoChannelTemp = new Vector<Int> [oSPWIn.nelements()];
  Bool bSPW_Channel = spw_channel( oSPWIn, aoChannelIn, oSPWTemp,
      aoChannelTemp );
  if ( !bSPW_Channel ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid spectral window(s) and/or channels"
        << LogIO::POST;
    return( *poOutput );
  }

  Vector<Double> oFreqTemp = Vector<Double>();
  Bool bFreq = freq( oSPWTemp, aoChannelTemp, oFreqTemp );
  if ( !bFreq ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid frequencies" << LogIO::POST;
    return( *poOutput );
  }


  // Calculate statistics for each field, antenna1, and antenna2 that are
  // found in the input parameters

  while ( !poNCTIter->pastEnd() ) {

    if ( !exists<Int>( poNCTIter->field()[0], oFieldIn ) ||
         !exists<Int>( poNCTIter->antenna1()[0], oAntenna1In ) ||
         !exists<Int>( poNCTIter->antenna2()[0], oAntenna2In ) ) {
      poNCTIter->next();
      continue;
    }

    poOutput->resize( ++uiNumOutput, True );

    poOutput->operator[](uiNumOutput-1).uiFieldID = poNCTIter->field()[0];
    poOutput->operator[](uiNumOutput-1).uiAntenna1 = poNCTIter->antenna1()[0];
    poOutput->operator[](uiNumOutput-1).uiAntenna2 = poNCTIter->antenna2()[0];

    Cube<Complex> oParamP = parse<Complex>( poNCTIter->cparam() );
    Cube<Complex> oParam = select<Complex>( oParamP, oFeedTemp, oFreqTemp,
        oTimeTemp );
    Cube<DComplex> oParamD( oParam.shape(), 0.0 );
    convertArray<DComplex,Complex>( oParamD, oParam );

    Cube<Float> oParamErrP = parse<Float>( poNCTIter->paramErr() );
    Cube<Float> oParamErr = select<Float>( oParamErrP, oFeedTemp, oFreqTemp,
        oTimeTemp );
    Cube<Double> oParamErrD( oParamErr.shape(), 0.0 );
    convertArray<Double,Float>( oParamErrD, oParamErr );

    Cube<Bool> oFlagP = parse<Bool>( poNCTIter->flag() );
    Cube<Bool> oFlag = select<Bool>( oFlagP, oFeedTemp, oFreqTemp, oTimeTemp );

    CalStats* poCS = NULL;

    try {

      switch ((uInt) eRAP) {
        case (uInt) REAL:
          poCS = (CalStats*) new CalStatsReal( real(oParamD), oParamErrD,
              oFlag, oFeedTemp, oFreqTemp, oTimeTemp, eAxisIterUserID );
          break;
	case (uInt) AMPLITUDE:
	  poCS = (CalStats*) new CalStatsAmp( oParamD, oParamErrD, oFlag,
              oFeedTemp, oFreqTemp, oTimeTemp, eAxisIterUserID, True );
          break;
	case (uInt) PHASE:
	  poCS = (CalStats*) new CalStatsPhase( oParamD, oParamErrD, oFlag,
              oFeedTemp, oFreqTemp, oTimeTemp, eAxisIterUserID, True );
          break;
        default:
          throw( AipsError( "Invalid parameter (REAL, AMPLITUDE, or PHASE)" ) );
      }

      poOutput->operator[](uiNumOutput-1).oOut = poCS->stats<T>( oArg );
      delete poCS;

    }

    catch ( AipsError oAE ) {
      LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
      log << LogIO::WARN << oAE.getMesg()
          << ", iteration (field,antenna1,antenna2) = (" << poNCTIter->field()
          << "," << poNCTIter->antenna1() << "," << poNCTIter->antenna2()
          << "), continuing ..." << LogIO::POST;
      poOutput->operator[](uiNumOutput-1).oOut = CalStats::OUT<T>();
    }

    poNCTIter->next();

  }


  // Reset the new calibration table iterator

  poNCTIter->reset();


  // Return the reference to the Vector<CalAnalysis::OUTPUT<T> > instance

  return( *poOutput );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis::stats<T> template public fit member function
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis static template public member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::exists<T>

Description:
------------
This member function determines whether a value appears in a vector.

Inputs:
-------
tValue - This reference to a T variable contains the desired value.
oValue - This reference to a Vector<T> instance contains the values.

Outputs:
--------
The reference to the existance Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Bool& CalAnalysis::exists( const T& tValue, const Vector<T>& oValue ) {

  // Determine whether the value is present in an array and return the boolean

  Bool* pbValue = new Bool( False );

  for ( uInt v=0; v<oValue.nelements(); v++ ) {
    if ( tValue == oValue[v] ) *pbValue = True;
  }

  return( *pbValue );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::uniqueNoSort<T>

Description:
------------
This member function returns an unsorted and unique vector from an input vector.

Inputs:
-------
oVector - This reference to a Vector<T> instance contains the values.

Outputs:
--------
The reference to the unsorted and unique Vector<T> instance, returned via the
function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Vector<T>& CalAnalysis::uniqueNoSort( const Vector<T>& oVector ) {

  // Initialize the unique and unsorted vector

  Vector<T>* poVectorUnique = new Vector<T>();


  // Form the unique and unsorted vector

  for ( uInt v1=0; v1<oVector.nelements(); v1++ ) {

    Bool bDupe = False;

    for ( uInt v2=0; v2<v1; v2++ ) {
      if ( oVector[v1] == oVector[v2] ) {
        bDupe = True;
	break;
      }
    }

    if ( !bDupe ) {
      poVectorUnique->resize( poVectorUnique->nelements()+1, True );
      poVectorUnique->operator[](poVectorUnique->nelements()-1) = oVector[v1];
    }

  }


  // Return the unique and unsorted vector

  return( *poVectorUnique );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis static template public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis template private member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::parse<T>

Description:
------------
This member function reshapes the cubes provided by class ROCTIter to dimensions
required by class CalStats.

Inputs:
-------
oCubeIn - This reference to a  Cube<T> instance contains the values dimensioned
          by feed x frequency(spw) x row(spw,time).

Outputs:
--------
The reference to the Cube<T> instance dimensioned feed x frequency x time.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Cube<T>& CalAnalysis::parse( const Cube<T>& oCubeIn ) const {

  // Initialize the output cube

  Cube<T>* poCubeOut = new Cube<T>( uiNumFeed, uiNumFreq, uiNumTime, (T) 0 );


  // Put input cube data (whose dimensions are feed x freq(spw) x row(spw,time)
  // into output cube data (whose dimensions are feed x freq x time) using the
  // Slicer() class

  uInt uiFreqStart = 0;

  for ( uInt s=0; s<uiNumSPW; s++ ) {

    IPosition oInStart( 3, 0, 0, s*uiNumTime );
    IPosition oInDelta( 3, uiNumFeed, oNumFreq[s], uiNumTime );
    Slicer oIn( oInStart, oInDelta );

    if ( s > 0 ) uiFreqStart += oNumFreq[s-1];

    IPosition oOutStart( 3, 0, uiFreqStart, 0 );
    IPosition oOutDelta( 3, uiNumFeed, oNumFreq[s], uiNumTime );
    Slicer oOut( oOutStart, oOutDelta );

    poCubeOut->operator()(oOut) = oCubeIn(oIn);

  }


  // Return the output cube

  return( *poCubeOut );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::select<T>

Description:
------------
This member function selects the desired feeds, frequencies, and times from an
input cube.

NB: Cubes feed to this function should be processed by CalAnalysis::parse<T>
first.

NB: I chose the brute force method.  It is not elegant, but it will handle all
types of selections, including pathological ones.  Given the expected sizes of
new calibration tables, speed should not be an issue.

Inputs:
-------
oCubeIn  - This reference to a Cube<T> instance contains the values dimensioned
           by feed x frequency x time.
oFeedOut - This reference to a Vector<Int> instance contains the desired feeds.
oFreqOut - This reference to a Vector<Double> instance contains the desired
           frequencies.
oTimeOut - This reference to a Vector<Double> instance contains the desired
           times.

Outputs:
--------
The reference to the Cube<T> instance dimensioned feed x frequency x time,
returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Cube<T>& CalAnalysis::select( const Cube<T>& oCubeIn,
    const Vector<String>& oFeedOut, const Vector<Double>& oFreqOut,
    const Vector<Double>& oTimeOut ) const {

  // Initialize the output cube

  Cube<T>* poCubeOut = new Cube<T>( oFeedOut.nelements(), oFreqOut.nelements(),
      oTimeOut.nelements(), (T) 0 );


  // Get the desired data from the input cube and put them into the output cube

  for ( uInt p=0,pOut=0; p<oFeed.nelements(); p++ ) {

    if ( oFeedOut[pOut] != oFeed[p] ) continue;

    for ( uInt f=0,fOut=0; f<oFreq.nelements(); f++ ) {

      if ( oFreqOut[fOut] != oFreq[f] ) continue;

      for ( uInt t=0,tOut=0; t<oTime.nelements(); t++ ) {
	if ( oTimeOut[tOut] != oTime[t] ) continue;
	poCubeOut->operator()(pOut,fOut,tOut) = oCubeIn(p,f,t);
	tOut++;
      }

      fOut++;

    }

    pOut++;

  }


  // Return the output cube

  return( *poCubeOut );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis template private member functions
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------
// End of define macro to prevent multiple loading
// -----------------------------------------------------------------------------
