//# PlotMSCache.cc: Data cache for plotms.
//# Copyright (C) 2009
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
//# $Id: $
#include <plotms/Data/PlotMSCache.h>

#include <casa/OS/Timer.h>
#include <casa/Quanta/MVTime.h>
#include <msvis/MSVis/VisBuffer.h>
#include <plotms/Data/PlotMSVBAverager.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>

namespace casa {

const PMS::Axis PlotMSCache::METADATA[] =
    { PMS::TIME, PMS::TIME_INTERVAL, PMS::FIELD, PMS::SPW, PMS::SCAN,
      PMS::ANTENNA1, PMS::ANTENNA2, PMS::CHANNEL, PMS::CORR, PMS::FREQUENCY,
      PMS::FLAG, PMS::FLAG_ROW };
const unsigned int PlotMSCache::N_METADATA = 11;

bool PlotMSCache::axisIsMetaData(PMS::Axis axis) {
    for(unsigned int i = 0; i < N_METADATA; i++)
        if(METADATA[i] == axis) return true;
    return false;
}

const unsigned int PlotMSCache::THREAD_SEGMENT = 10;

                                           
PlotMSCache::PlotMSCache():
  nChunk_(0),
  nPoints_(),
  refTime_p(0.0),
  minX_(0),
  maxX_(0),
  minY_(0),
  maxY_(0),
  time_(),
  timeIntr_(),
  field_(),
  spw_(),
  scan_(),
  dataLoaded_(false),
  currentSet_(false)
{

    // Set up loaded axes to be initially empty, and set up data columns for
    // data-based axes.
    const vector<PMS::Axis>& axes = PMS::axes();
    for(unsigned int i = 0; i < axes.size(); i++) {
        loadedAxes_[axes[i]] = false;
        if(PMS::axisIsData(axes[i]))
            loadedAxesData_[axes[i]]= PMS::DEFAULT_DATACOLUMN;
    }    
}

PlotMSCache::~PlotMSCache() {
  // Deflate everything
  deleteCache();
}

// increase the number of chunks we can store
void PlotMSCache::increaseChunks(Int nc) {

  Int oldnChunk=nChunk_;

  if (nc==0) {   // no guidance
    if (nChunk_<1) // currently empty
      nChunk_=32;
    else
      // Double it
      nChunk_*=2;
  }
  else
    // Add requested number
    nChunk_+=nc;

  // Resize, copying existing contents
  scan_.resize(nChunk_,True);
  time_.resize(nChunk_,True);
  timeIntr_.resize(nChunk_,True);
  field_.resize(nChunk_,True);
  spw_.resize(nChunk_,True);
  chan_.resize(nChunk_,False,True);
  freq_.resize(nChunk_,False,True);
  corr_.resize(nChunk_,False,True);

  row_.resize(nChunk_,False,True);
  antenna1_.resize(nChunk_,False,True);
  antenna2_.resize(nChunk_,False,True);
  baseline_.resize(nChunk_,False,True);
  uvdist_.resize(nChunk_,False,True);
  uvdistL_.resize(nChunk_,False,True);
  u_.resize(nChunk_,False,True);
  v_.resize(nChunk_,False,True);
  w_.resize(nChunk_,False,True);

  amp_.resize(nChunk_,False,True);
  pha_.resize(nChunk_,False,True);
  real_.resize(nChunk_,False,True);
  imag_.resize(nChunk_,False,True);
  flag_.resize(nChunk_,False,True);
  flagrow_.resize(nChunk_,False,True);
  az_.resize(nChunk_,False,True);
  el_.resize(nChunk_,False,True);
  parang_.resize(nChunk_,False,True);


  plmask_.resize(nChunk_,False,True);
  
  // Construct (empty) pointed-to Vectors/Arrays
  for (Int ic=oldnChunk;ic<nChunk_;++ic) {
    row_[ic] = new Vector<uInt>();
    antenna1_[ic] = new Vector<Int>();
    antenna2_[ic] = new Vector<Int>();
    baseline_[ic] = new Vector<Int>();
    uvdist_[ic] = new Vector<Double>();
    uvdistL_[ic] = new Matrix<Double>();
    u_[ic] = new Vector<Double>();
    v_[ic] = new Vector<Double>();
    w_[ic] = new Vector<Double>();
    freq_[ic] = new Vector<Double>();
    chan_[ic] = new Vector<Int>();
    corr_[ic] = new Vector<Int>();
    amp_[ic] = new Array<Float>();
    pha_[ic] = new Array<Float>();
    real_[ic] = new Array<Float>();
    imag_[ic] = new Array<Float>();
    flag_[ic] = new Array<Bool>();
    flagrow_[ic] = new Vector<Bool>();
    az_[ic] = new Vector<Double>();
    el_[ic] = new Vector<Double>();
    parang_[ic] = new Vector<Float>();

    plmask_[ic] = new Array<Bool>();
  }

  // Zero the cumulative count
  nPoints_.resize(nChunk_);
  nPoints_=0;


}

void PlotMSCache::clear() {
    deleteCache();
    refTime_p=0.0;
}

void PlotMSCache::load(VisSet& visSet, const vector<PMS::Axis>& axes,
        const vector<PMS::DataColumn>& data, const PlotMSAveraging& averaging,
        PlotMSCacheThread* thread) {
    // TBD: 
    // o Should we have ONE PtrBlock to a list of Records, each of which
    //    we fill with desired data/info, rather than private data for
    //    every possible option?  (Keys become indices, etc., Eventual 
    //    disk paging?)
    // o Partial appends? E.g., if we have to go back and get something 
    //    that we didn't get the first time around.  Use of Records as above
    //    may make this easier, too.

      // need a way to keep track of whether:
      // 1) we already have the metadata loaded
      // 2) the underlying MS has changed, requiring a reloading of metadata


  // Check if scr cols present
  Bool scrcolOk(False);
  {
    const ColumnDescSet& cds=Table(visSet.msName()).tableDesc().columnDescSet();
    scrcolOk=cds.isDefined("CORRECTED_DATA");
  }

  cout << endl << "Caching for the new plot: " 
       << PMS::axis(axes[1]) << "("<< axes[1] << ") vs. " 
       << PMS::axis(axes[0]) << "(" << axes[0] << ")..." << endl;

  if (!scrcolOk) 
    cout << "NB: Scratch columns not present; will use DATA exclusively." << endl;

  cout << "Spectral window averaging is " << (averaging.spw() ? "ON." : "off.") << endl;

  cout << "Channel averaging is " << (averaging.channel() ? "ON" : "off");
  if (averaging.channel())
    if (averaging.channelValue()<=0.0) 
      cout << ", but with an ambiguous value of " << averaging.channelValue()
	   << ", so no channel averaging will occur.";
    else
      cout << ", with a value of " << averaging.channelValue() 
	   << ( (averaging.channelValue()>1) ? " channels" : " (i.e. full spw)" );
  cout << "." << endl;
  
  cout << "Time averaging is " << (averaging.time() ? "ON" : "off");
  if (averaging.time()) {
    cout << ", with a value of " << averaging.timeValue() << " seconds." << endl;
    cout << "  Scan averaging is " << (averaging.scan() ? "ON" : "off") << "; ";
    cout << "  Field averaging is "<< (averaging.field()? "ON" : "off");
  }
  cout << "." << endl;

  cout << "Baseline averaging is " << (averaging.baseline() ? "ON." : "off.") << endl;
  cout << "Antenna averaging is " << (averaging.antenna() ? "ON." : "off.") << endl;

  
  // Calculate which axes need to be loaded; those that have already been
    // loaded do NOT need to be reloaded (assuming that the rest of PlotMS has
    // done its job and cleared the cache if the underlying MS/selection has
    // changed).
    vector<PMS::Axis> loadAxes; vector<PMS::DataColumn> loadData;
    
    // Check meta-data.
    for(unsigned int i = 0; i < N_METADATA; i++) {
        if(!loadedAxes_[METADATA[i]]) {
            loadAxes.push_back(METADATA[i]);
            loadData.push_back(PMS::DEFAULT_DATACOLUMN);
        }
    }
        
    // Check given axes.  Should only be added to load list if: 1) not
    // already in load list, 2) not loaded, or 3) loaded but with different
    // data column (if applicable).
    bool found; PMS::Axis axis; PMS::DataColumn dc;
    for(unsigned int i = 0; i < axes.size(); i++) {
        found = false;
        axis = axes[i];
        
        // if data vector is not the same length as axes vector, assume
        // default data column
        dc = PMS::DEFAULT_DATACOLUMN;
        if(i < data.size() && scrcolOk) dc = data[i];
        
        // 1)
        for(unsigned int j = 0; !found && j < loadAxes.size(); j++)
            if(loadAxes[i] == axis) found = true;
        if(found) continue;
        
        // 2)
        if(!loadedAxes_[axis]) {
            loadAxes.push_back(axis);
            loadData.push_back(dc);
        }
        
        // 3)
        else if(PMS::axisIsData(axis) && dc != loadedAxesData_[axis]) {
            loadAxes.push_back(axis);
            loadData.push_back(dc);
        }
    }
        
    if(loadAxes.size() == 0) return; // nothing to be loaded
    
    // Load data.

    Vector<Int> nIterPerAve;
    if ( (averaging.time() && averaging.timeValue()>0.0) ||
	 averaging.baseline() ||
	 averaging.antenna() ||
	 averaging.spw() ) {

      countChunks(visSet,nIterPerAve,averaging);
      loadChunks(visSet,averaging,nIterPerAve,
		 loadAxes,loadData,thread);

    }
    else {

      countChunks(visSet);
      loadChunks(visSet,loadAxes,loadData,averaging,thread);

    }


    // Update loaded axes.
    for(unsigned int i = 0; i < loadAxes.size(); i++) {
        axis = loadAxes[i];
        loadedAxes_[axis] = true;
        if(PMS::axisIsData(axis)) loadedAxesData_[axis] = loadData[i];
    }
        
    dataLoaded_ = true;

    cout << "Finished loading." << endl;

}

void PlotMSCache::loadChunks(VisSet& vs,
			     const vector<PMS::Axis> loadAxes,
			     const vector<PMS::DataColumn> loadData,
			     const PlotMSAveraging& averaging,
			     PlotMSCacheThread* thread) {

  cout << "Loading chunks..." << endl;

  VisIter& vi(vs.iter());
  VisBuffer vb(vi);

  Int chunk = 0;
  chshapes_.resize(4,nChunk_);
  double progress;
  for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
    for(vi.origin(); vi.more(); vi++) {
      // If a thread is given, check if the user canceled.
      if(thread != NULL && thread->wasCanceled()) {
	dataLoaded_ = false;
	return;
      }
      
      // If a thread is given, update it.
      if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			    chunk % THREAD_SEGMENT == 0))
	thread->setStatus("Loading chunk " + String::toString(chunk) +
			  " / " + String::toString(nChunk_) + ".");
      
      
      // Do channel averaging, if required
      if (averaging.channel() && averaging.channelValue()>0.0) {
	  // Force read on required stuff
	  forceVBread(vb,loadAxes,loadData);
	  // Delegate actual averaging to the VisBuffer:
	  vb.channelAve(averaging.channelValue());
      }
      
      // Cache the data shapes
      chshapes_(0,chunk)=vb.nCorr();
      chshapes_(1,chunk)=vb.nChannel();
      chshapes_(2,chunk)=vb.nRow();
      chshapes_(3,chunk)=vs.numberAnt();
      
      for(unsigned int i = 0; i < loadAxes.size(); i++) {
	loadAxis(vb, chunk, loadAxes[i], loadData[i]);
      }
      chunk++;
      
      // If a thread is given, update it.
      if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			    chunk % THREAD_SEGMENT == 0)) {
	progress = ((double)chunk+1) / nChunk_;
	thread->setProgress((unsigned int)((progress * 100) + 0.5));
      }
    }
  }
  
}

void PlotMSCache::loadChunks(VisSet& vs,
			     const PlotMSAveraging& averaging,
			     const Vector<Int>& nIterPerAve,
			     const vector<PMS::Axis> loadAxes,
			     const vector<PMS::DataColumn> loadData,
			     PlotMSCacheThread* thread) {
  
  cout << "Loading chunks with averaging..." << endl;

  Bool verby(False);

  VisIter& vi(vs.iter());
  VisBuffer vb(vi);

  chshapes_.resize(4,nChunk_);
  double progress;
  vi.originChunks();
  vi.origin();
  Double time0=86400.0*floor(vb.time()(0)/86400.0);
  for (Int chunk=0;chunk<nChunk_;++chunk) {

    // If a thread is given, check if the user canceled.
    if(thread != NULL && thread->wasCanceled()) {
      dataLoaded_ = false;
      return;
    }
    
    // If a thread is given, update it.
    if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			  chunk % THREAD_SEGMENT == 0))
      thread->setStatus("Loading chunk " + String::toString(chunk) +
			" / " + String::toString(nChunk_) + ".");
      
    // Arrange to accumulate many VBs into one
    PlotMSVBAverager pmsvba(vs.numberAnt(),vi.existsWeightSpectrum());

    // Tell averager if we are averaging baselines together
    pmsvba.setBlnAveraging(averaging.baseline());
    pmsvba.setAntAveraging(averaging.antenna());

    // Sort out which data to read
    discernData(loadAxes,loadData,pmsvba);

    if (verby) cout << chunk << "----------------------------------" << endl;

    for (Int iter=0;iter<nIterPerAve(chunk);++iter) {

      // Force read on required stuff
      forceVBread(vb,loadAxes,loadData);
      
      if (verby) {
	cout << "ck=" << chunk << " vb=" << iter << " (" << nIterPerAve(chunk) << ");  " 
	     << "sc=" << vb.scan()(0) << " "
	     << "time=" << vb.time()(0)-time0 << " "
	     << "fl=" << vb.fieldId() << " "
	     << "sp=" << vb.spectralWindow() << " ";
      }

      // Do channel averaging, if required
      if (averaging.channel() && averaging.channelValue()>0.0) {
	// Delegate actual averaging to the VisBuffer:
	vb.channelAve(averaging.channelValue());
      }
      
      // Accumulate into the averager
      pmsvba.accumulate(vb);
      
      // Advance to next VB
      vi++;

      if (verby) cout << " next VB ";
      
      
      if (!vi.more() && vi.moreChunks()) {
	// go to first vb in next chunk
	if (verby) cout << "  stepping VI";
	vi.nextChunk();
	vi.origin();
      }
      if (verby) cout << endl;
    }


    // Finalize the averaging
    pmsvba.finalizeAverage();

    // The averaged VisBuffer
    VisBuffer& avb(pmsvba.aveVisBuff());

    // Cache the data shapes
    chshapes_(0,chunk)=avb.nCorr();
    chshapes_(1,chunk)=avb.nChannel();
    chshapes_(2,chunk)=avb.nRow();
    chshapes_(3,chunk)=vs.numberAnt();

    for(unsigned int i = 0; i < loadAxes.size(); i++) {
      loadAxis(avb, chunk, loadAxes[i], loadData[i]);
    }
      
    // If a thread is given, update it.
    if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			  chunk % THREAD_SEGMENT == 0)) {
      progress = ((double)chunk+1) / nChunk_;
      thread->setProgress((unsigned int)((progress * 100) + 0.5));
    }
  }
}

void PlotMSCache::forceVBread(VisBuffer& vb,
			      vector<PMS::Axis> loadAxes,
			      vector<PMS::DataColumn> loadData) {

  // pre-load requisite pieces of VisBuffer for averaging
  for(unsigned int i = 0; i < loadAxes.size(); i++) {
    switch (loadAxes[i]) {
    case PMS::AMP: 
    case PMS::PHASE: 
    case PMS::REAL: 
    case PMS::IMAG: {
      switch(loadData[i]) {
      case PMS::DATA: {
	vb.visCube();
	break;
      }
      case PMS::MODEL: {
	vb.modelVisCube();
	break;
      }
      case PMS::CORRECTED: {
	vb.correctedVisCube();
	break;
      }
      case PMS::RESIDUAL: {
	vb.correctedVisCube();
	vb.modelVisCube();
	break;
      }
      default:
	break;
      }
      break;
    }
    default:
      break;
    }
  }
  
  // Always need flags
  vb.flagRow();
  vb.flagCube();

}

void PlotMSCache::discernData(vector<PMS::Axis> loadAxes,
			      vector<PMS::DataColumn> loadData,
			      PlotMSVBAverager& vba) {

  // Turn off 
  vba.setNoData();

  // Tell the averager which data column to read
  for(unsigned int i = 0; i < loadAxes.size(); i++) {
    switch (loadAxes[i]) {
    case PMS::AMP: 
    case PMS::PHASE: 
    case PMS::REAL: 
    case PMS::IMAG: {
      switch(loadData[i]) {
      case PMS::DATA: {
	//	cout << "Arranging to load VC." << endl;
	vba.setDoVC();
	break;
      }
      case PMS::MODEL: {
	//	cout << "Arranging to load MVC." << endl;
	vba.setDoMVC();
	break;
      }
      case PMS::CORRECTED: {
	//	cout << "Arranging to load CVC." << endl;
	vba.setDoCVC();
	break;
      }
      case PMS::RESIDUAL: {
	//	cout << "Arranging to load CVC & MVC." << endl;
	vba.setDoCVC();
	vba.setDoMVC();
	break;
      }
      default:
	break;
      }
      break;
    }
    case PMS::UVDIST:
    case PMS::UVDIST_L:
    case PMS::U:
    case PMS::V:
    case PMS::W: {
      //  cout << "Arranging to load UVW
      vba.setDoUVW();
    }
    default:
      break;
    }
  }

}

      
void PlotMSCache::countChunks(VisSet& vs) {

  // This is the old way, with no averaging over chunks.

  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  
  vi.originChunks();
  vi.origin();
  refTime_p=86400.0*floor(vb.time()(0)/86400.0);

  // Count number of chunks.
  int chunk = 0;
  for(vi.originChunks(); vi.moreChunks(); vi.nextChunk())
    for (vi.origin(); vi.more(); vi++) chunk++;
  if(chunk != nChunk_) increaseChunks(chunk);
  
  //  cout << "Found " << nChunk_ << " " << chunk << " chunks." << endl;

}


void PlotMSCache::countChunks(VisSet& vs, Vector<Int>& nIterPerAve,
			      const PlotMSAveraging& averaging) {

  Bool verby(False);

  Bool combscan(averaging.scan());
  Bool combfld(averaging.field());
  Bool combspw(averaging.spw());
  
  Int nsortcol(4+Int(!combscan));  // include room for scan
  Block<Int> columns(nsortcol);
  Int i(0);
  Double iterInterval(0.0);
  if (averaging.time())
    iterInterval= averaging.timeValue();

  columns[i++]=MS::ARRAY_ID;
  if (!combscan) columns[i++]=MS::SCAN_NUMBER;  // force scan boundaries
  if (!combfld) columns[i++]=MS::FIELD_ID;      // force field boundaries
  if (!combspw) columns[i++]=MS::DATA_DESC_ID;  // force spw boundaries
  columns[i++]=MS::TIME;
  if (combspw || combfld) iterInterval=DBL_MIN;  // force per-timestamp chunks
  if (combfld) columns[i++]=MS::FIELD_ID;      // effectively ignore field boundaries
  if (combspw) columns[i++]=MS::DATA_DESC_ID;  // effectively ignore spw boundaries
  
  vs.resetVisIter(columns,iterInterval);
  
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  
  vi.originChunks();
  vi.origin();
  
  nIterPerAve.resize(100);
  nIterPerAve=0;
  
  Double time0(86400.0*floor(vb.time()(0)/86400.0));
  refTime_p=time0;
  Double time1(0.0),time(0.0);
  
  Int thisscan(-1),lastscan(-1);
  Int thisfld(-1),lastfld(-1);
  Int thisspw(-1),lastspw(-1);
  Int chunk(0);
  Int ave(-1);
  Double interval(0.0);
  if (averaging.time())
    interval= averaging.timeValue();
  Double avetime1(-1.0);

  vi.originChunks();
  vi.origin();

  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk(),chunk++) {
    Int iter(0);
    for (vi.origin(); vi.more();vi++,iter++) {

      time1=vb.time()(0);  // first time in this vb
      thisscan=vb.scan()(0);
      thisfld=vb.fieldId();
      thisspw=vb.spectralWindow();

      // New chunk means new ave interval, IF....
      if ( // (!combfld && !combspw) ||                // not combing fld nor spw, OR
	  ((time1-avetime1)>interval) ||         // (combing fld and/or spw) and solint exceeded, OR
	  ((time1-avetime1)<0.0) ||                // a negative time step occurs, OR
	  (!combscan && (thisscan!=lastscan)) ||   // not combing scans, and new scan encountered OR
	  (!combspw && (thisspw!=lastspw)) ||      // not combing spws, and new spw encountered  OR
	  (!combfld && (thisfld!=lastfld)) ||      // not combing fields, and new field encountered OR
	  (ave==-1))  {                            // this is the first interval

	if (verby) {
	  cout << "--------------------------------" << endl;
	  cout << boolalpha << interval << " " 
	       << ((time1-avetime1)>interval)  << " "
	       << ((time1-avetime1)<0.0) << " "
	       << (!combscan && (thisscan!=lastscan)) << " "
	       << (!combspw && (thisspw!=lastspw)) << " "
	       << (!combfld && (thisfld!=lastfld)) << " "
	       << (ave==-1) << " "
	       << endl;
	}

	avetime1=time1;  // for next go
	ave++;
	
	if (verby) cout << "ave = " << ave << endl;


	// increase size of nIterPerAve array, if needed
	if (nIterPerAve.nelements()<uInt(ave+1))
	  nIterPerAve.resize(nIterPerAve.nelements()+100,True);
	nIterPerAve(ave)=0;
      }
      
      // Increment chunk-per-sol count for current solution
      nIterPerAve(ave)++;
      
      if (verby) {
	cout << "          ck=" << chunk << " " << avetime1-time0 << endl;
	time=vb.time()(0);
	cout  << "                 " << "vb=" << iter << " ";
	cout << "ar=" << vb.arrayId() << " ";
	cout << "sc=" << vb.scan()(0) << " ";
	if (!combfld) cout << "fl=" << vb.fieldId() << " ";
	if (!combspw) cout << "sp=" << vb.spectralWindow() << " ";
	cout << "t=" << floor(time-time0)  << " (" << floor(time-avetime1) << ") ";
	if (combfld) cout << "fl=" << vb.fieldId() << " ";
	if (combspw) cout << "sp=" << vb.spectralWindow() << " ";
	cout << endl;
	
      }
      
      lastscan=thisscan;
      lastfld=thisfld;
      lastspw=thisspw;
    }
  }
  
  Int nAve(ave+1);
  nIterPerAve.resize(nAve,True);
  
  if (verby)  cout << "nIterPerAve = " << nIterPerAve << endl;


  if (nChunk_ != nAve) increaseChunks(nAve);
  
}


#define PMSC_DELETE(VAR)                                                \
  for(unsigned int j = 0; j < VAR.size(); j++)  			\
    if(VAR[j]) delete VAR[j];                                           \
  VAR.resize(0,True);

void PlotMSCache::release(const vector<PMS::Axis>& axes) {
    for(unsigned int i = 0; i < axes.size(); i++) {
        switch(axes[i]) {
        case PMS::SCAN: scan_.resize(0); break;
        case PMS::FIELD: field_.resize(0); break;
        case PMS::TIME: time_.resize(0); break;
        case PMS::TIME_INTERVAL: timeIntr_.resize(0); break;
        case PMS::SPW: spw_.resize(0); break;
        
        case PMS::CHANNEL: PMSC_DELETE(chan_) break;
        case PMS::FREQUENCY: PMSC_DELETE(freq_) break;
        case PMS::CORR: PMSC_DELETE(corr_) break;
        case PMS::ANTENNA1: PMSC_DELETE(antenna1_) break;
        case PMS::ANTENNA2: PMSC_DELETE(antenna2_) break;
        case PMS::BASELINE: PMSC_DELETE(baseline_) break;
        case PMS::UVDIST: PMSC_DELETE(uvdist_) break;
        case PMS::UVDIST_L: PMSC_DELETE(uvdistL_) break;
        case PMS::U: PMSC_DELETE(u_) break;
        case PMS::V: PMSC_DELETE(v_) break;
        case PMS::W: PMSC_DELETE(w_) break;
        case PMS::AMP: PMSC_DELETE(amp_) break;
        case PMS::PHASE: PMSC_DELETE(pha_) break;
        case PMS::REAL: PMSC_DELETE(real_) break;
        case PMS::IMAG: PMSC_DELETE(imag_) break;
        case PMS::FLAG: PMSC_DELETE(flag_) break;
        case PMS::FLAG_ROW: PMSC_DELETE(flagrow_) break;
        case PMS::AZIMUTH: PMSC_DELETE(az_) break;
        case PMS::ELEVATION: PMSC_DELETE(el_) break;
        case PMS::PARANG: PMSC_DELETE(parang_) break;
        case PMS::ROW: PMSC_DELETE(row_) break;
	default: break;
        }        

        loadedAxes_[axes[i]] = false;
        
        if(dataLoaded_ && axisIsMetaData(axes[i])) dataLoaded_ = false;
        
        if((dataLoaded_ || currentSet_) &&
           (currentX_ == axes[i] || currentY_ == axes[i])) {
            dataLoaded_ = false;
            currentSet_ = false;
        }
    }
    
    if(!dataLoaded_ || !currentSet_) nChunk_ = 0;
}

bool PlotMSCache::readyForPlotting() const {
    return dataLoaded_ && currentSet_;
}

void PlotMSCache::setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis) {

  // Put axes on a Vector so we can loop over them below
  Vector<Int> plaxes(2);
  plaxes(0)=xAxis;
  plaxes(1)=yAxis;

  // Set up masks that we use to realize axes relationships
  Matrix<Bool> xymask(4,2);

  xymask.set(False);
  Vector<Bool> xmask(xymask.column(0));
  Vector<Bool> ymask(xymask.column(1));
  getAxesMask(xAxis,xmask);
  getAxesMask(yAxis,ymask);
  Vector<Bool> nmask=(xmask || ymask);
  Vector<Bool> cmask=(xmask && ymask);

  //  cout << boolalpha;
  //  cout << "xmask = " << xmask << endl;
  //  cout << "ymask = " << ymask << endl;
  //  cout << "nmask = " << nmask << endl;
  //  cout << "cmask = " << cmask << endl;


  // Forbid antenna-based/baseline-based combination plots, for now
  //  (e.g., data vs. _antenna-based_ elevation)
  if (nmask(2)&&nmask(3))
    throw(AipsError("Cannot yet support antenna-based and baseline-based data in same plot."));

  icorrmax_.reference(chshapes_.row(0));
  ichanmax_.reference(chshapes_.row(1));
  ibslnmax_.reference(chshapes_.row(2));
  iantmax_.reference(chshapes_.row(3));
  
  idatamax_.resize(nChunk_);
  idatamax_=chshapes_.row(0);
  idatamax_*=chshapes_.row(1);
  idatamax_*=chshapes_.row(2);
  
  ichanbslnmax_.resize(nChunk_);
  ichanbslnmax_=chshapes_.row(1);
  ichanbslnmax_*=chshapes_.row(2);
  
  nperchan_.resize(nChunk_);
  nperchan_.set(1);
  if (nmask(0)) nperchan_*=chshapes_.row(0);
  
  nperbsln_.resize(nChunk_);
  nperbsln_.set(1);
  if (nmask(0)) nperbsln_*=chshapes_.row(0);
  if (nmask(1)) nperbsln_*=chshapes_.row(1);

  for (Int ichk=0;ichk<nChunk_;++ichk) {

    //    if (ntrue(nmask(IPosition(1,0),IPosition(1,2)))==3) 
    //      // corr, chan, baseline all present; just reference flag_
    //      plmask_[ichk]->reference(*flag_[ichk]);
    //    else {
      // create a collapsed version of the flags

      IPosition nsh(3,1,1,1),csh;
      
      for (Int iax=0;iax<3;++iax) {
	if (nmask(iax)) 
	  // non-trivial size for this axis
	  nsh(iax)=chshapes_(iax,ichk);
	else 
	  // add this axis to collapse list
	  csh.append(IPosition(1,iax));
      }

      plmask_[ichk]->resize(nsh);
      (*plmask_[ichk]) = operator>(partialNFalse(*flag_[ichk],csh).reform(nsh),uInt(0));


      //    }
  }


  // Count up the total number of points we will plot
  //   (keep a cumualtive running total)

  Int cumulativeN(0);
  for (Int ic=0;ic<nChunk_;++ic) {
    Int chN=1;
    for (Int ii=0;ii<4;++ii) 
      if (nmask(ii))
	chN*=chshapes_.column(ic)(ii);
    cumulativeN+=chN;
    nPoints_(ic)=cumulativeN;
  }

  //  cout << "nPoints_         = " << nPoints_ << endl;

  //  cout << "nChunk    = " << nChunk() << endl;
  //  cout << "nPoints() = " << nPoints() << endl;


  currentX_ = xAxis; currentY_ = yAxis;
  currentSet_ = true;

  computeRanges();

}


void PlotMSCache::getAxesMask(PMS::Axis axis,Vector<Bool>& axismask) {

  // Nominally all False
  axismask.set(False);

  switch(axis) {
  case PMS::AMP:
  case PMS::PHASE:
  case PMS::REAL:
  case PMS::IMAG:
  case PMS::FLAG:
    axismask(Slice(0,3,1))=True;
    break;
  case PMS::CHANNEL:
  case PMS::FREQUENCY:
    axismask(1)=True;
    break;
  case PMS::CORR:
    axismask(0)=True;
    break;
  case PMS::ROW:
  case PMS::ANTENNA1:
  case PMS::ANTENNA2:
  case PMS::BASELINE:
  case PMS::UVDIST:
  case PMS::U:
  case PMS::V:
  case PMS::W:
  case PMS::FLAG_ROW:
    axismask(2)=True;
    break;
  case PMS::UVDIST_L:
    axismask(1)=True;
    axismask(2)=True;
    break;
  case PMS::AZIMUTH:
  case PMS::ELEVATION:
  case PMS::PARANG:
    axismask(3)=True;
    break;
  case PMS::TIME:
  case PMS::TIME_INTERVAL:
  case PMS::SCAN:
  case PMS::SPW:
  case PMS::FIELD:
  default:
    break;
  }

}




Double PlotMSCache::getX(Int i) {

  // Find correct chunk and index offset
  setChunk(i);

  return get(currentX_);
		
}

Double PlotMSCache::getY(Int i) {

  // Find correct chunk and index offset
  setChunk(i);

  return get(currentY_);
  
}

void PlotMSCache::getXY(Int i,Double& x,Double& y) {

  // Find correct chunk and index offset
  setChunk(i);

  x= get(currentX_);
  y= get(currentY_);

}


Bool PlotMSCache::getFlagMask(Int i) {

  // Find correct chunk and index offset
  setChunk(i);
  
  return !(*(plmask_[currChunk_]->data()+irel_));
};

Double PlotMSCache::get(PMS::Axis axis) {

  // Call axis-specific gets
  switch(axis) {
    // Degenerate ones
  case PMS::SCAN:
    return getScan();
    break;
  case PMS::FIELD:
    return getField();
    break;
  case PMS::TIME:
    return getTime();
    break;
  case PMS::TIME_INTERVAL:
    return getTimeIntr();
    break;
  case PMS::SPW:
    return getSpw();
    break;

    // Partial shapes
  case PMS::FREQUENCY:
    return getFreq();
    break;
  case PMS::CHANNEL:
    return getChan();
    break;
  case PMS::CORR:
    return getCorr();
    break;
  case PMS::ANTENNA1:
    return getAnt1();
    break;
  case PMS::ANTENNA2:
    return getAnt2();
    break;
  case PMS::BASELINE:
    return getBsln();
    break;

  case PMS::UVDIST:
    return getUVDist();
    break;
  case PMS::U:
    return getU();
    break;
  case PMS::V:
    return getV();
    break;
  case PMS::W:
    return getW();
    break;
  case PMS::UVDIST_L:
    return getUVDistL();
    break;

    // Data
  case PMS::AMP:
    return getAmp();
    break;
  case PMS::PHASE:
    return getPha();
    break;
  case PMS::REAL:
    return getReal();
    break;
  case PMS::IMAG:
    return getImag();
    break;
  case PMS::FLAG:
    return getFlag();
    break;

  case PMS::FLAG_ROW:
    return getFlagRow();
    break;

  case PMS::AZIMUTH:
    return getAz();
    break;
  case PMS::ELEVATION:
    return getEl();
    break;
  case PMS::PARANG:
    return getParAng();
    break;
  case PMS::ROW:
    return getRow();
    break;
  default:
    throw(AipsError("Help!"));
  }

}


void PlotMSCache::getRanges(Double& minX, Double& maxX, Double& minY,
        Double& maxY) {
    if(!currentSet_) return;
    minX = minX_; maxX = maxX_;
    minY = minY_; maxY = maxY_;
}


PlotLogMessage* PlotMSCache::locateNearest(Double x, Double y) {
  
  throw(AipsError("locateNearest is disabled."));

  // WARNING: Hardwired to for amp vs freq plot
  /*
  Double mindist(DBL_MAX);
  Int minic(0),minidx(0);

  Vector<Double> freqdist;
  Double fdist;
  Float adist,dist;

  for (Int ic=0;ic<nChunk_;++ic) {
    Int npts=amp_[ic]->nelements();
    if (npts>0) {

      freqdist.resize(0);
      freqdist=(*freq_[ic]);
      freqdist-=x;

      Float *a = amp_[ic]->data();
      for (Int i=0;i<npts;++i,++a) {
	adist= *a - Float(y);
	adist/=Float(maxY_-minY_);  // Unitless!
	adist*=adist;
	
	//	fdist=*(freq_[ic]->data()+(i/xdiv_)%xmod_) - Double(x);
	fdist=freqdist((i/xdiv_)%xmod_);
	fdist/=(maxX_-minX_);       // Unitless!
	fdist*=fdist;

	dist=adist+Float(fdist);

	if (dist<mindist) {
	  minic=ic;
	  minidx=i;
	  mindist=dist;
	}
      }

    }
  }

  stringstream ss;
  reportMeta(minic,minidx,ss);
  return new PlotLogGeneric(PlotMS::CLASS_NAME, PlotMS::LOG_LOCATE, ss.str());

  */

}

PlotLogMessage* PlotMSCache::locateRange(Double xmin,Double xmax,Double ymin,Double ymax) {

  Timer locatetimer;

  locatetimer.mark();

  Double thisx;
  Double thisy;
  stringstream ss;

  Int nFound(0);
  for (Int i=0;i<nPoints();++i) {
    
    getXY(i,thisx,thisy);;

    if (thisx > xmin && thisx < xmax)
      if (thisy > ymin && thisy < ymax) {
	++nFound;
	reportMeta(thisx,thisy,ss);
	ss << '\n';
      }
  }
  
  ss << "Found " << nFound << " points among " << nPoints() << " in "
     << locatetimer.all_usec()/1.0e6 << "s.";

  /*
  // get rid of last newline
  long pos = ss.tellp();
  if(pos > 0) {
      ss.seekp(pos - 1);
      ss.write(" ", 1);
  }
  */
  
  return new PlotLogGeneric(PlotMS::CLASS_NAME, PlotMS::LOG_LOCATE, ss.str());
}

vector<pair<PMS::Axis, unsigned int> > PlotMSCache::loadedAxes() const {    
    // have to const-cast loaded axes because the [] operator is not const,
    // even though we're not changing it.
    map<PMS::Axis, bool>& la = const_cast<map<PMS::Axis, bool>& >(loadedAxes_);
    
    vector<pair<PMS::Axis, unsigned int> > v;
    const vector<PMS::Axis>& axes = PMS::axes();
    for(unsigned int i = 0; i < axes.size(); i++)
        if(la[axes[i]])
            v.push_back(pair<PMS::Axis, unsigned int>(
                        axes[i], nPointsForAxis(axes[i])));
    
    return v;
}

void PlotMSCache::reportMeta(Double x, Double y,stringstream& ss) {



  ss << "Scan=" << getScan() << " ";
  ss << "Field=" << getField() << " ";
  ss << "Time=" << MVTime(getTime()/C::day).string(MVTime::YMD,7) << " ";
  ss << "BL=";

  Int ant1=Int(getAnt1());
  if (ant1<0)
    ss << "*-";
  else
    ss << ant1 << "-";

  Int ant2=Int(getAnt2());
  if (ant2<0)
    ss << "* ";
  else
    ss << ant2 << " ";

  Int spw=Int(getSpw());
  ss << "Spw=";
  if (spw<0)
    ss << "* ";
  else
    ss << spw << " ";

  ss << "Chan=" << getChan() << " ";
  ss << "Freq=" << getFreq() << " ";
  ss << "Corr=" << getCorr() << " ";
  ss << "X=" << x << " ";
  ss << "Y="  << y << " ";
  ss << "(" << (currChunk_ > 0 ? (nPoints_(currChunk_-1)+irel_) : irel_) << "/";
  ss << currChunk_ << "/" << irel_ << ")";

}

void PlotMSCache::setChunk(Int i) {

  if (i==0) currChunk_=0;  // probably insufficient as a general reset!

  // Bump up if needed
  if (i > (nPoints_(currChunk_)-1)) ++currChunk_; 

  irel_=i;
  if (currChunk_>0) 
    irel_-=nPoints_(currChunk_-1);

}

void PlotMSCache::deleteCache() {
    // Release all axes.
    release(PMS::axes());
}

void PlotMSCache::loadAxis(const VisBuffer& vb, Int vbnum, PMS::Axis axis,
			   PMS::DataColumn data) {    

    switch(axis) {

    case PMS::SCAN: // assumes scan unique in VB
        scan_(vbnum) = vb.scan()(0); 
	break;
        
    case PMS::FIELD:
        field_(vbnum) = vb.fieldId();
	break;
        
    case PMS::TIME: // assumes time unique in VB
        time_(vbnum) = vb.time()(0); 
	break;
        
    case PMS::TIME_INTERVAL: // assumes timeInterval unique in VB
        timeIntr_(vbnum) = vb.timeInterval()(0); 
	break;
        
    case PMS::SPW:
        spw_(vbnum) = vb.spectralWindow(); 
	break;

    case PMS::CHANNEL:
        *chan_[vbnum] = vb.channel(); 
	break;
        
    case PMS::FREQUENCY:
        *freq_[vbnum] = vb.frequency()/1.0e9; 
	break;
        
    case PMS::CORR:
      *corr_[vbnum] = vb.corrType();
      break;
        
    case PMS::ANTENNA1:
        *antenna1_[vbnum] = vb.antenna1(); 
	break;
    case PMS::ANTENNA2:
        *antenna2_[vbnum] = vb.antenna2(); 
	break;
    case PMS::BASELINE: {
      Vector<Int> a1(vb.antenna1());
      Vector<Int> a2(vb.antenna2());
      baseline_[vbnum]->resize(vb.nRow());
      Vector<Int> bl(*baseline_[vbnum]);
      for (Int irow=0;irow<vb.nRow();++irow)
	bl(irow)=chshapes_(3,0) * a1(irow) - (a1(irow) * (a1(irow) - 1)) / 2 + a2(irow) - a1(irow);
	//	bl(irow)=40 * a1(irow) - (a1(irow) * (a1(irow) - 1)) / 2 + a2(irow) - a1(irow);
      //	bl(irow)=40 * a2(irow) - (a2(irow) * (a2(irow) - 1)) / 2 + a1(irow) - a2(irow);
      break;
    }
    case PMS::UVDIST: {
      Vector<Double> u(vb.uvwMat().row(0));
      Vector<Double> v(vb.uvwMat().row(1));
      *uvdist_[vbnum] = sqrt(u*u+v*v);
      break;
    }
    case PMS::U:
      *u_[vbnum] = vb.uvwMat().row(0);
      break;
    case PMS::V:
      *v_[vbnum] = vb.uvwMat().row(1);
      break;
    case PMS::W:
      *w_[vbnum] = vb.uvwMat().row(2);
      break;
    case PMS::UVDIST_L: {
      Vector<Double> u(vb.uvwMat().row(0));
      Vector<Double> v(vb.uvwMat().row(1));
      Vector<Double> uvdistM = sqrt(u*u+v*v);
      uvdistM /=C::c;
      uvdistL_[vbnum]->resize(vb.nChannel(),vb.nRow());
      Vector<Double> uvrow;
      for (Int irow=0;irow<vb.nRow();++irow) {
	uvrow.reference(uvdistL_[vbnum]->column(irow));
	uvrow.set(uvdistM(irow));
	uvrow*=vb.frequency();
      }
      break;
    }

    case PMS::AMP: {
      switch(data) {
      case PMS::DATA: {
	*amp_[vbnum] = amplitude(vb.visCube());
	break;
      }
      case PMS::MODEL: {
	*amp_[vbnum] = amplitude(vb.modelVisCube());
	break;
      }
      case PMS::CORRECTED: {
	*amp_[vbnum] = amplitude(vb.correctedVisCube());
	break;
      }
      case PMS::RESIDUAL: {
	*amp_[vbnum] = amplitude(vb.correctedVisCube()-vb.modelVisCube());
	break;
      }
      }
      break;
    }
    case PMS::PHASE: {
      switch(data) {
      case PMS::DATA: {
	*pha_[vbnum] = phase(vb.visCube())*(180.0/C::pi);
	break;
      }
      case PMS::MODEL: {
	*pha_[vbnum] = phase(vb.modelVisCube())*(180.0/C::pi);
	break;
      }
      case PMS::CORRECTED: {
	*pha_[vbnum] = phase(vb.correctedVisCube())*(180.0/C::pi);
	break;
      }
      case PMS::RESIDUAL: {
	*pha_[vbnum] = phase(vb.correctedVisCube()-vb.modelVisCube())*(180.0/C::pi);
	break;
      }
      }
      break;
    }
    case PMS::REAL: {
      switch(data) {
      case PMS::DATA: {
	*real_[vbnum] = real(vb.visCube());
	break;
      }
      case PMS::MODEL: {
	*real_[vbnum] = real(vb.modelVisCube());
	break;
      }
      case PMS::CORRECTED: {
	*real_[vbnum] = real(vb.correctedVisCube());
	break;
      }
      case PMS::RESIDUAL: {
	*real_[vbnum] = real(vb.correctedVisCube())-real(vb.modelVisCube());
	break;
      }
      }
      break;
    }
    case PMS::IMAG: {
      switch(data) {
      case PMS::DATA: {
	*imag_[vbnum] = imag(vb.visCube());
	break;
      }
      case PMS::MODEL: {
	*imag_[vbnum] = imag(vb.modelVisCube());
	break;
      }
      case PMS::CORRECTED: {
	*imag_[vbnum] = imag(vb.correctedVisCube());
	break;
      }
      case PMS::RESIDUAL: {
	*imag_[vbnum] = imag(vb.correctedVisCube())-imag(vb.modelVisCube());
	break;
      }
      }
      break;
    }
                
    case PMS::FLAG:
      *flag_[vbnum] = vb.flagCube();
      break;
    case PMS::FLAG_ROW:
      *flagrow_[vbnum] = vb.flagRow();
      break;

    case PMS::AZIMUTH:
    case PMS::ELEVATION: {
      Matrix<Double> azel;
      vb.azelMat(vb.time()(0),azel);
      *az_[vbnum] = azel.row(0);
      *el_[vbnum] = azel.row(1);
      break;
    }
    case PMS::PARANG:
      *parang_[vbnum] = vb.feed_pa(vb.time()(0))*(180.0/C::pi);
      break;

    case PMS::ROW:
      *row_[vbnum] = vb.rowIds();
      break;

      /*        
    case PMS::WEIGHT:
    case PMS::VEL_RADIO:
    case PMS::VEL_OPTICAL:
    case PMS::VEL_RELATIVISTIC:
    case PMS::AZIMUTH:
    case PMS::ELEVATION:
    case PMS::BASELINE:
    case PMS::HOURANGLE:
    case PMS::PARALLACTICANGLE:
      */

    default:
      throw(AipsError("Axis choice NYI"));
      break;
    }
}

unsigned int PlotMSCache::nPointsForAxis(PMS::Axis axis) const {
    switch(axis) {    
    case PMS::FREQUENCY: 
    case PMS::CHANNEL: 
    case PMS::AMP: 
    case PMS::PHASE: 
    case PMS::ANTENNA1:
    case PMS::ANTENNA2: 
    case PMS::BASELINE: 
    case PMS::UVDIST:
    case PMS::UVDIST_L:
    case PMS::U:
    case PMS::V:
    case PMS::W:
    case PMS::FLAG:
    case PMS::AZIMUTH: 
    case PMS::ELEVATION: 
    case PMS::PARANG: 
    case PMS::ROW:
    case PMS::FLAG_ROW: 
      {

        unsigned int n = 0;
        for(unsigned int i = 0; i < freq_.size(); i++) {
            if(axis == PMS::FREQUENCY)     n += freq_[i]->size();
            else if(axis == PMS::CHANNEL)  n += chan_[i]->size();
            else if(axis == PMS::AMP)      n += amp_[i]->size();
            else if(axis == PMS::PHASE)    n += pha_[i]->size();
            else if(axis == PMS::ROW)      n += row_[i]->size();
            else if(axis == PMS::ANTENNA1) n += antenna1_[i]->size();
            else if(axis == PMS::ANTENNA2) n += antenna2_[i]->size();
            else if(axis == PMS::BASELINE) n += antenna2_[i]->size();
            else if(axis == PMS::UVDIST)   n += uvdist_[i]->size();
            else if(axis == PMS::UVDIST_L) n += uvdistL_[i]->size();
            else if(axis == PMS::U)        n += u_[i]->size();
            else if(axis == PMS::V)        n += v_[i]->size();
            else if(axis == PMS::W)        n += w_[i]->size();
            else if(axis == PMS::FLAG)     n += flag_[i]->size();
            else if(axis == PMS::AZIMUTH)  n += az_[i]->size();
            else if(axis == PMS::ELEVATION)n += el_[i]->size();
            else if(axis == PMS::PARANG)   n += parang_[i]->size();
	    else if(axis == PMS::FLAG_ROW) n += flagrow_[i]->size();
        }
        return n;
    }     
    
    case PMS::TIME:          return time_.size();
    case PMS::TIME_INTERVAL: return timeIntr_.size();    
    case PMS::FIELD:         return field_.size();      
    case PMS::SCAN:          return scan_.size();     
    case PMS::SPW:           return spw_.size();     
        
    default: return 0;
    }
}

void PlotMSCache::computeRanges() {

  cout << "Computing ranges..." << flush;

    Vector<Int> plaxes(2);
    plaxes(0)=currentX_;
    plaxes(1)=currentY_;


    Vector<Double> limits(4);
    limits(0)=limits(2)=DBL_MAX;
    limits(1)=limits(3)=-DBL_MAX;

    Int totalN(0);
    for (Int ic=0;ic<nChunk_;++ic) {

      Int thisN=ntrue(*plmask_[ic]);
      if (thisN >0) {
	totalN+=thisN;
	for (Int ix=0;ix<2;++ix) {

	  // Arrange collapsed-on-axis mask
	  Array<Bool> collmask;
	  switch(plaxes(ix)) {
	  case PMS::FREQUENCY: 
	  case PMS::CHANNEL: {
	    // collapse on corr, row
	    collmask=operator>(partialNTrue(*plmask_[ic],IPosition(2,0,2)),uInt(0));
	    break;
	  }
	  case PMS::CORR: {
	    // collapse on chan, row
	    collmask=operator>(partialNTrue(*plmask_[ic],IPosition(2,1,2)),uInt(0));
	    break;
	  }
	  case PMS::ROW:
	  case PMS::ANTENNA1:
	  case PMS::ANTENNA2:
	  case PMS::BASELINE:
	  case PMS::UVDIST:
	  case PMS::U:
	  case PMS::V:
	  case PMS::W: {
	    // collapse on corr,chan
	    collmask=operator>(partialNTrue(*plmask_[ic],IPosition(2,0,1)),uInt(0));
	    break;
	  }
	  case PMS::UVDIST_L: {
	    // collapse on corr
	    collmask=operator>(partialNTrue(*plmask_[ic],IPosition(1,0)),uInt(0));
	    break;
	  }
	  case PMS::AMP: 
	  case PMS::PHASE:
	  case PMS::REAL:
	  case PMS::IMAG:{
	    // reference plmask_
	    collmask.reference(*plmask_[ic]);
	    break;
	  }
	  case PMS::AZIMUTH:
	  case PMS::ELEVATION:
	  case PMS::PARANG:
	    // TBD
	    break;
	  default:
	    break;
	  }

	  // Now calculate masked limits

	  if (True) {

	  switch(plaxes(ix)) {
	  case PMS::SCAN:
	    limits(2*ix)=min(limits(2*ix),Double(scan_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(scan_(ic)));
	    break;
	  case PMS::FIELD:
	    limits(2*ix)=min(limits(2*ix),Double(field_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(field_(ic)));
	    break;
	  case PMS::TIME:
	    limits(2*ix)=min(limits(2*ix),time_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),time_(ic));
	    break;
	  case PMS::TIME_INTERVAL:
	    limits(2*ix)=min(limits(2*ix),timeIntr_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),timeIntr_(ic));
	    break;
	  case PMS::SPW:
	    limits(2*ix)=min(limits(2*ix),Double(spw_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(spw_(ic)));
	    break;
	  case PMS::FREQUENCY:
	    if (freq_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*freq_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*freq_[ic])(collmask)));
	    }
	    break;
	  case PMS::CHANNEL:
	    if (chan_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*chan_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*chan_[ic])(collmask))));
	    }
	    break;
	  case PMS::CORR:
	    if (corr_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*corr_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*corr_[ic])(collmask))));
	    }
	    break;
	  case PMS::ROW:
	    if (row_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*row_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*row_[ic])(collmask))));
	    }
	    break;
	  case PMS::ANTENNA1:
	    if (antenna1_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*antenna1_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*antenna1_[ic])(collmask))));
	    }
	    break;
	  case PMS::ANTENNA2:
	    if (antenna2_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*antenna2_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*antenna2_[ic])(collmask))));
	    }
	    break;
	  case PMS::BASELINE:
	    if (baseline_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*baseline_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*baseline_[ic])(collmask))));
	    }
	    break;
	    
	  case PMS::UVDIST:
	    if (uvdist_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*uvdist_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*uvdist_[ic])(collmask))));
	    }
	    break;
	  case PMS::U:
	    if (u_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*u_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*u_[ic])(collmask))));
	    }
	    break;
	  case PMS::V:
	    if (v_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*v_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*v_[ic])(collmask))));
	    }
	    break;
	  case PMS::W:
	    if (w_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*w_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*w_[ic])(collmask))));
	    }
	    break;
	  case PMS::UVDIST_L:
	    if (uvdistL_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*uvdistL_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*uvdistL_[ic])(collmask))));
	    }
	    break;
	    
	  case PMS::AMP: {
	    if (amp_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*amp_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*amp_[ic])(collmask)));
	    }
	    break;
	  }
	  case PMS::PHASE:
	    if (pha_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*pha_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*pha_[ic])(collmask)));
	    }
	    break;
	  case PMS::REAL:
	    if (real_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*real_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*real_[ic])(collmask)));
	    }
	    break;
	  case PMS::IMAG:
	    if (imag_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*imag_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*imag_[ic])(collmask)));
	    }
	    break;
	  case PMS::FLAG:
	  case PMS::FLAG_ROW:
	    if (flag_[ic]->nelements()>0) {
	      limits(2*ix)=-0.5;
	      limits(2*ix+1)=1.5;
	    }
	    break;
	  case PMS::AZIMUTH:
	    if (az_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*az_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*az_[ic]));
	    }
	    break;
	  case PMS::ELEVATION:
	    if (el_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*el_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*el_[ic]));
	    }
	    break;
	  case PMS::PARANG:
	    if (parang_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*parang_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*parang_[ic]));
	    }
	    break;
	  default:
	    throw(AipsError("Help PlotMSCache::computeRanges"));
	  }

	  }
	  else {

	  // Now calculate masked limits
	  switch(plaxes(ix)) {
	  case PMS::SCAN:
	    limits(2*ix)=min(limits(2*ix),Double(scan_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(scan_(ic)));
	    break;
	  case PMS::FIELD:
	    limits(2*ix)=min(limits(2*ix),Double(field_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(field_(ic)));
	    break;
	  case PMS::TIME:
	    limits(2*ix)=min(limits(2*ix),time_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),time_(ic));
	    break;
	  case PMS::TIME_INTERVAL:
	    limits(2*ix)=min(limits(2*ix),timeIntr_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),timeIntr_(ic));
	    break;
	  case PMS::SPW:
	    limits(2*ix)=min(limits(2*ix),Double(spw_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(spw_(ic)));
	    break;
	  case PMS::FREQUENCY:
	    if (freq_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*freq_[ic])));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*freq_[ic])));
	    }
	    break;
	  case PMS::CHANNEL:
	    if (chan_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*chan_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*chan_[ic]))));
	    }
	    break;
	  case PMS::CORR:
	    if (corr_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*corr_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*corr_[ic]))));
	    }
	    break;
	  case PMS::ROW:
	    if (row_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*row_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*row_[ic]))));
	    }
	    break;
	  case PMS::ANTENNA1:
	    if (antenna1_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*antenna1_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*antenna1_[ic]))));
	    }
	    break;
	  case PMS::ANTENNA2:
	    if (antenna2_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*antenna2_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*antenna2_[ic]))));
	    }
	    break;
	  case PMS::BASELINE:
	    if (baseline_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*baseline_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*baseline_[ic]))));
	    }
	    break;
	    
	  case PMS::UVDIST:
	    if (uvdist_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*uvdist_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*uvdist_[ic]))));
	    }
	    break;
	  case PMS::U:
	    if (u_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*u_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*u_[ic]))));
	    }
	    break;
	  case PMS::V:
	    if (v_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*v_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*v_[ic]))));
	    }
	    break;
	  case PMS::W:
	    if (w_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*w_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*w_[ic]))));
	    }
	    break;
	  case PMS::UVDIST_L:
	    if (uvdistL_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*uvdistL_[ic]))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*uvdistL_[ic]))));
	    }
	    break;
	    
	  case PMS::AMP: {
	    if (amp_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*amp_[ic])));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*amp_[ic])));
	    }
	    break;
	  }
	  case PMS::PHASE:
	    if (pha_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*pha_[ic])));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*pha_[ic])));
	    }
	    break;
	  case PMS::REAL:
	    if (real_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*real_[ic])));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*real_[ic])));
	    }
	    break;
	  case PMS::IMAG:
	    if (imag_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*imag_[ic])));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*imag_[ic])));
	    }
	    break;
	  case PMS::FLAG:
	  case PMS::FLAG_ROW:
	    if (flag_[ic]->nelements()>0) {
	      limits(2*ix)=-0.5;
	      limits(2*ix+1)=1.5;
	    }
	    break;
	  case PMS::AZIMUTH:
	    if (az_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*az_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*az_[ic]));
	    }
	    break;
	  case PMS::ELEVATION:
	    if (el_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*el_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*el_[ic]));
	    }
	    break;
	  case PMS::PARANG:
	    if (parang_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*parang_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*parang_[ic]));
	    }
	    break;
	  default:
	    throw(AipsError("Help PlotMSCache::computeRanges"));
	  }

	  }

	}
      }
    }

    minX_=limits(0);
    maxX_=limits(1);
    minY_=limits(2);
    maxY_=limits(3);

    cout << ": dX=" << minX_ << "-" << maxX_ << " dY=" << minY_ << "-" << maxY_ << endl;
    cout << "Npoints = " << totalN << endl;

}

}
