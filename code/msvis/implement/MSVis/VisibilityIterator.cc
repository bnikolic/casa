//# VisibilityIterator.cc: Step through MeasurementEquation by visibility
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
//# $Id: VisibilityIterator.cc,v 19.15 2006/02/01 01:25:14 kgolap Exp $

#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Sort.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColDescSet.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TiledStManAccessor.h>

namespace casa { //# NAMESPACE CASA - BEGIN

ROVisibilityIterator::ROVisibilityIterator() 
 : selRows_p(0, 0) 
{}

// const of MS is cast away, but we don't actually change it.
// (just to share code between RO version and RW version of iterator)
ROVisibilityIterator::ROVisibilityIterator(const MeasurementSet &ms,
					   const Block<Int>& sortColumns,
					   Double timeInterval)
    : msIter_p(ms,sortColumns,timeInterval),selRows_p(0, 0),slurp_p(false),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
floatDataFound_p(False),lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False)
{

 
    
  initsinglems();
  


}


void ROVisibilityIterator::initsinglems(){

  //  cout << "addDefaultSortColumns = False!" << endl;
  This = (ROVisibilityIterator*)this;
  isMultiMS_p=False;
  msCounter_p=0;
  Block<Vector<Int> > blockNGroup(1);
  Block<Vector<Int> > blockStart(1);
  Block<Vector<Int> > blockWidth(1);
  Block<Vector<Int> > blockIncr(1);
  Block<Vector<Int> > blockSpw(1);
  Int nspw=msIter_p.msColumns().spectralWindow().nrow();
  blockNGroup[0].resize(nspw);
  blockNGroup[0].set(1);
  blockStart[0].resize(nspw);
  blockStart[0].set(0);  
  blockWidth[0].resize(nspw);
  blockWidth[0]=msIter_p.msColumns().spectralWindow().numChan().getColumn(); 
  blockIncr[0].resize(nspw);
  blockIncr[0].set(1);
  blockSpw[0].resize(nspw); 
  indgen(blockSpw[0]);
  selectChannel(blockNGroup, blockStart, blockWidth, blockIncr,
		blockSpw);
    

}
ROVisibilityIterator::ROVisibilityIterator(const MeasurementSet &ms,
					   const Block<Int>& sortColumns, const Bool addDefaultSort,
					   Double timeInterval)
 :msIter_p(ms,sortColumns,timeInterval, addDefaultSort),selRows_p(0, 0),slurp_p(false),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
  floatDataFound_p(False),lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False){


  initsinglems();

}


ROVisibilityIterator::ROVisibilityIterator(const Block<MeasurementSet> &mss,
					   const Block<Int>& sortColumns,
					   Double timeInterval)
: msIter_p(mss,sortColumns,timeInterval),selRows_p(0, 0),slurp_p(false),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
floatDataFound_p(False),lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False)
{
  
  initmultims(mss);

}

ROVisibilityIterator::ROVisibilityIterator(const Block<MeasurementSet> &mss,
					   const Block<Int>& sortColumns, const Bool addDefaultSort,
					   Double timeInterval)
  : msIter_p(mss,sortColumns,timeInterval,addDefaultSort),selRows_p(0, 0),slurp_p(false),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
    floatDataFound_p(False),lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False){

  initmultims(mss);

}
void ROVisibilityIterator::initmultims(const Block<MeasurementSet> &mss){

  This = (ROVisibilityIterator*)this; 
  msCounter_p=0;
  isMultiMS_p=True;
  Int numMS=mss.nelements();
  Block<Vector<Int> > blockNGroup(numMS);
  Block<Vector<Int> > blockStart(numMS);
  Block<Vector<Int> > blockWidth(numMS);
  Block<Vector<Int> > blockIncr(numMS);
  Block<Vector<Int> > blockSpw(numMS);

  for (Int k=0; k < numMS; ++k){
    ROMSSpWindowColumns msSpW(mss[k].spectralWindow());
    Int nspw=msSpW.nrow();
    blockNGroup[k].resize(nspw);
    blockNGroup[k].set(1);
    blockStart[k].resize(nspw);
    blockStart[k].set(0);  
    blockWidth[k].resize(nspw);
    blockWidth[k]=msSpW.numChan().getColumn(); 
    blockIncr[k].resize(nspw);
    blockIncr[k].set(1);
    blockSpw[k].resize(nspw); 
    indgen(blockSpw[k]);
  }
  selectChannel(blockNGroup, blockStart, blockWidth, blockIncr,
		blockSpw);

}


ROVisibilityIterator::ROVisibilityIterator(const ROVisibilityIterator& other)
    : selRows_p(other.selRows_p)
{
    operator=(other);
}

ROVisibilityIterator::~ROVisibilityIterator() 
{
}

ROVisibilityIterator& 
ROVisibilityIterator::operator=(const ROVisibilityIterator& other) 
{
  if (this==&other) return *this;
  This=(ROVisibilityIterator*)this;
  msIter_p=other.msIter_p;
  selRows_p=other.selRows_p;
  slurp_p = other.slurp_p;
  curChanGroup_p=other.curChanGroup_p;
  curNumChanGroup_p=other.curNumChanGroup_p;
  channelGroupSize_p=other.channelGroupSize_p;
  curNumRow_p=other.curNumRow_p;
  curTableNumRow_p=other.curTableNumRow_p;
  curStartRow_p=other.curStartRow_p;
  curEndRow_p=other.curEndRow_p;
  nRowBlocking_p=other.nRowBlocking_p;
  nChan_p=other.nChan_p;
  nPol_p=other.nPol_p;
  more_p=other.more_p;
  newChanGroup_p=other.newChanGroup_p;
  initialized_p=other.initialized_p;
  msIterAtOrigin_p=other.msIterAtOrigin_p;
  stateOk_p=other.stateOk_p;
  numChanGroup_p=other.numChanGroup_p;
  chanStart_p=other.chanStart_p;
  chanWidth_p=other.chanWidth_p;
  chanInc_p=other.chanInc_p;
  preselectedChanStart_p=other.preselectedChanStart_p;
  preselectednChan_p=other.preselectednChan_p;
  isMultiMS_p=other.isMultiMS_p;
  msCounter_p=other.msCounter_p;
  slicer_p=other.slicer_p;
  weightSlicer_p=other.weightSlicer_p;
  useSlicer_p=other.useSlicer_p;
  time_p.resize(other.time_p.nelements()); 
  time_p=other.time_p;
  timeInterval_p.resize(other.timeInterval_p.nelements()); 
  timeInterval_p=other.timeInterval_p;
  frequency_p.resize(other.frequency_p.nelements()); 
  frequency_p=other.frequency_p;
  freqCacheOK_p=other.freqCacheOK_p;
  flagOK_p = other.flagOK_p;
  visOK_p = other.visOK_p;
  weightSpOK_p = other.weightSpOK_p;
  flagCube_p.resize(other.flagCube_p.shape()); flagCube_p=other.flagCube_p;
  visCube_p.resize(other.visCube_p.shape()); visCube_p=other.visCube_p;
  uvwMat_p.resize(other.uvwMat_p.shape()); uvwMat_p=other.uvwMat_p;
  feedpa_p.resize(other.feedpa_p.nelements()); feedpa_p=other.feedpa_p;
  parang0_p=other.parang0_p;
  parang_p.resize(other.parang_p.nelements()); parang_p=other.parang_p;
  azel0_p=other.azel0_p;
  azel_p.resize(other.azel_p.nelements()); azel_p=other.azel_p;
  hourang_p=other.hourang_p;
  floatDataFound_p=other.floatDataFound_p;

  msd_p=other.msd_p;
  lastfeedpaUT_p=other.lastfeedpaUT_p;
  lastazelUT_p=other.lastazelUT_p;
  nAnt_p=other.nAnt_p;

  velSelection_p = other.velSelection_p;
  vPrecise_p=other.vPrecise_p;
  nVelChan_p=other.nVelChan_p;
  vStart_p=other.vStart_p;
  vInc_p=other.vInc_p;
  vDef_p=other.vDef_p;
  cFromBETA_p=other.cFromBETA_p;
  selFreq_p.resize(other.selFreq_p.nelements()); selFreq_p=other.selFreq_p;
  lsrFreq_p.resize(other.lsrFreq_p.nelements()); lsrFreq_p=other.lsrFreq_p;
  

  // column access functions
  colAntenna1.reference(other.colAntenna1);
  colAntenna2.reference(other.colAntenna2);
  colFeed1.reference(other.colFeed1);
  colFeed2.reference(other.colFeed2);
  colTime.reference(other.colTime);
  colTimeInterval.reference(other.colTimeInterval);
  colWeight.reference(other.colWeight);
  colWeightSpectrum.reference(other.colWeightSpectrum);
  colVis.reference(other.colVis);
  colFloatVis.reference(other.colFloatVis);
  colModelVis.reference(other.colModelVis);
  colCorrVis.reference(other.colCorrVis);
  colSigma.reference(other.colSigma);
  colFlag.reference(other.colFlag);
  colFlagRow.reference(other.colFlagRow);
  colScan.reference(other.colScan);
  colUVW.reference(other.colUVW);
  imwgt_p=other.imwgt_p;
  return *this;
}

void ROVisibilityIterator::setRowBlocking(Int nRow)
{
  nRowBlocking_p=nRow;
}

void ROVisibilityIterator::useImagingWeight(const VisImagingWeight& imWgt){
    imwgt_p=imWgt;
}
void ROVisibilityIterator::origin()
{
  if (!initialized_p) {
    originChunks();
  } else {
    curChanGroup_p=0;
    newChanGroup_p=True;
    curStartRow_p=0;
    freqCacheOK_p=False;
    flagOK_p = weightSpOK_p = False;
    visOK_p.resize(3); visOK_p[0]=visOK_p[1]=visOK_p[2]=False;
    setSelTable();
    attachColumns(attachTable());
    getTopoFreqs();
    updateSlicer();
    more_p=curChanGroup_p<curNumChanGroup_p;
    // invalidate any attached VisBuffer
    if (!vbStack_p.empty()) ((VisBuffer*)vbStack_p.top())->invalidate();
  }
}

void ROVisibilityIterator::originChunks()
{
  initialized_p=True;
  if (!msIterAtOrigin_p) {
    msIter_p.origin();
    msIterAtOrigin_p=True;
    
    while((!isInSelectedSPW(msIter_p.spectralWindowId())) 
	  && (msIter_p.more()))
      msIter_p++;
    
    stateOk_p=False;
    msCounter_p=msId();
    
  }
  setState();
  origin();
  setTileCache();
}

Bool ROVisibilityIterator::isInSelectedSPW(const Int& spw){
  
  for (uInt k=0; k < blockSpw_p[msId()].nelements() ; ++k){
    if(spw==blockSpw_p[msId()][k])
      return True;
  }
  return False;
}

void ROVisibilityIterator::advance()
{
  newChanGroup_p=False;
  flagOK_p = visOK_p[0] = visOK_p[1] = visOK_p[2] = weightSpOK_p = False;
  curStartRow_p=curEndRow_p+1;
  if (curStartRow_p>=curTableNumRow_p) {
    if (++curChanGroup_p >= curNumChanGroup_p) {
      curChanGroup_p--;
      more_p=False;
    } else {
      curStartRow_p=0;
      newChanGroup_p=True;
      freqCacheOK_p=False;
      updateSlicer();
    }
  }
  if (more_p) {
    setSelTable();
    getTopoFreqs();
    // invalidate any attached VisBuffer
    if (!vbStack_p.empty()) ((VisBuffer*)vbStack_p.top())->invalidate();
  }
}

ROVisibilityIterator& ROVisibilityIterator::nextChunk()
{

  if (msIter_p.more()) {
    msIter_p++;
    if((!isInSelectedSPW(msIter_p.spectralWindowId()))){
      while( (!isInSelectedSPW(msIter_p.spectralWindowId()))
	     && (msIter_p.more()))
	msIter_p++;
      stateOk_p=False;
    }
      
    if(msIter_p.newMS()){
      msCounter_p=msId();
      doChannelSelection();
    }
    msIterAtOrigin_p=False;
    stateOk_p=False;
  }
  if (msIter_p.more()) {
    setState();
    getTopoFreqs();
    if (!vbStack_p.empty()) ((VisBuffer*)vbStack_p.top())->invalidate();
  }
  more_p=msIter_p.more();
  return *this;
}

void ROVisibilityIterator::setSelTable()
{
  // work out how many rows to return 
  // for the moment we return all rows with the same value for time
  // unless row blocking is set, in which case we return more rows at once.
  if (nRowBlocking_p>0) {
    curEndRow_p = curStartRow_p + nRowBlocking_p;
    if (curEndRow_p >= curTableNumRow_p) curEndRow_p=curTableNumRow_p-1;
  } else {
    for (curEndRow_p=curStartRow_p+1; curEndRow_p<curTableNumRow_p && 
    	   time_p(curEndRow_p)==time_p(curEndRow_p-1); 
	 curEndRow_p++);
    curEndRow_p--;
  }

  curNumRow_p=curEndRow_p-curStartRow_p+1;
  selRows_p = RefRows(curStartRow_p, curEndRow_p);
  rowIds_p.resize(0);
}

void ROVisibilityIterator::getTopoFreqs()
{
  if (velSelection_p) {
    // convert selected velocities to TOPO frequencies
    // first convert observatory vel to correct frame (for this time)
    msd_p.setEpoch(msIter_p.msColumns().timeMeas()(curStartRow_p));
    if (msIter_p.newMS()) 
      msd_p.setObservatoryPosition(msIter_p.telescopePosition());
    // get obs velocity in required frame
    MRadialVelocity obsRV = msd_p.obsVel();
    // convert to doppler in required definition and get out in m/s
    Double obsVel=cFromBETA_p(obsRV.toDoppler()).getValue().get().getValue();
    // Now compute corresponding TOPO freqs
    selFreq_p.resize(nVelChan_p);
    lsrFreq_p.resize(nVelChan_p);
    Double v0 = vStart_p.getValue(), dv=vInc_p.getValue();
    if (aips_debug) cout << "obsVel="<<obsVel<<endl;
    for (Int i=0; i<nVelChan_p; i++) {
      Double vTopo = v0 + i*dv - obsVel;
      MDoppler dTopo(Quantity(vTopo,"m/s"), vDef_p);
      selFreq_p(i) = MFrequency::fromDoppler
	(dTopo,msIter_p.restFrequency().getValue()).getValue().getValue();
      // also calculate the frequencies in the requested frame for matching
      // up with the image planes 
      // (they are called lsr here, but don't need to be in that frame)
      MDoppler dLSR(Quantity(v0+i*dv,"m/s"), vDef_p);
      lsrFreq_p(i) = MFrequency::fromDoppler
	(dLSR,msIter_p.restFrequency().getValue()).getValue().getValue();
    }
  }
}

void ROVisibilityIterator::setState()
{
  if (stateOk_p) return;

  curTableNumRow_p = msIter_p.table().nrow();
  // get the times for this (major) iteration, so we can do (minor) 
  // iteration by constant time (needed for VisBuffer averaging).
  ROScalarColumn<Double> lcolTime(msIter_p.table(),MS::columnName(MS::TIME));
  time_p.resize(curTableNumRow_p); 
  lcolTime.getColumn(time_p);
  ROScalarColumn<Double> lcolTimeInterval(msIter_p.table(),
					  MS::columnName(MS::INTERVAL));
  timeInterval_p.resize(curTableNumRow_p); 
  lcolTimeInterval.getColumn(timeInterval_p);
  curStartRow_p=0;
  setSelTable();
  attachColumns(attachTable());
  // If this is a new MeasurementSet then set up the antenna locations
  if (msIter_p.newMS()) {
    This->nAnt_p = msd_p.setAntennas(msIter_p.msColumns().antenna());
    This->feedpa_p.resize(nAnt_p);
    This->feedpa_p.set(0);
    This->parang_p.resize(nAnt_p);
    This->parang_p.set(0);
    This->azel_p.resize(nAnt_p);

  }	
  if (msIter_p.newField() || msIterAtOrigin_p) { 
    msd_p.setFieldCenter(msIter_p.phaseCenter());
  }
  if ( msIter_p.newSpectralWindow() || msIterAtOrigin_p) {
    Int spw=msIter_p.spectralWindowId();
    if (floatDataFound_p) {
      nChan_p = colFloatVis.shape(0)(1);
      nPol_p = colFloatVis.shape(0)(0);
    } else {
      nChan_p = colVis.shape(0)(1);
      nPol_p = colVis.shape(0)(0);
    }
    if (Int(numChanGroup_p.nelements())<= spw || 
	numChanGroup_p[spw] == 0) {
      // no selection set yet, set default = all
      // for a reference MS this will normally be set appropriately in VisSet
      selectChannel(1,msIter_p.startChan(),nChan_p);
    }
    channelGroupSize_p=chanWidth_p[spw];
    curNumChanGroup_p=numChanGroup_p[spw];
    freqCacheOK_p=False;
  }

  stateOk_p=True;
}

void ROVisibilityIterator::updateSlicer()
{
  
  if(msIter_p.newMS()){
    numChanGroup_p.resize(0, True, False);
    doChannelSelection();
  }
  
  // set the Slicer to get the selected part of spectrum out of the table
  Int spw=msIter_p.spectralWindowId();
  //Fixed what i think was a confusion between chanWidth and chanInc
  // 2007/11/12
  Int start=chanStart_p[spw]+curChanGroup_p*chanWidth_p[spw];
  start-=msIter_p.startChan();
  AlwaysAssert(start>=0 && start+channelGroupSize_p<=nChan_p,AipsError);
  //  slicer_p=Slicer(Slice(),Slice(start,channelGroupSize_p));
  // above is slow, use IPositions instead.
  slicer_p=Slicer(IPosition(2,0,start),
		  IPosition(2,nPol_p,channelGroupSize_p), 
		  IPosition(2,1, (chanInc_p[spw]<=0)? 1 : chanInc_p[spw] ));
  weightSlicer_p=Slicer(IPosition(1,start),IPosition(1,channelGroupSize_p), 
			IPosition(1,(chanInc_p[spw]<=0)? 1 : chanInc_p[spw]));
  useSlicer_p=channelGroupSize_p<nChan_p;

  if(msIter_p.newMS()){
    setTileCache();
  }
}

void ROVisibilityIterator::setTileCache(){
  // This function sets the tile cache because of a feature in 
  // sliced data access that grows memory dramatically in some cases
  //  if(useSlicer_p){
  {

    const MeasurementSet& thems=msIter_p.ms();
    const ColumnDescSet& cds=thems.tableDesc().columnDescSet();
    /*
    ROArrayColumn<Complex> colVis;
    ROArrayColumn<Bool> colbool;
    ROArrayColumn<Float> colfloat;
    ROArrayColumn<Double> coldouble;
    */
    Vector<String> columns(8);
    // complex
    columns(0)=MS::columnName(MS::DATA);
    columns(1)=MS::columnName(MS::CORRECTED_DATA);
    columns(2)=MS::columnName(MS::MODEL_DATA);
    // boolean
    columns(3)=MS::columnName(MS::FLAG);
    // float
    columns(4)=MS::columnName(MS::WEIGHT_SPECTRUM);
    columns(5)=MS::columnName(MS::WEIGHT);
    columns(6)=MS::columnName(MS::SIGMA);
    // double
    columns(7)=MS::columnName(MS::UVW);
    //
    for (uInt k=0; k< columns.nelements(); ++k){
      if (cds.isDefined(columns(k))) {
	const ColumnDesc& cdesc=cds[columns(k)];
	String dataManType="";
	dataManType=cdesc.dataManagerType();
	//have to do something special about weight_spectrum as it tend to exist but 
	//has no valid data
	if(columns[k]==MS::columnName(MS::WEIGHT_SPECTRUM))
	  if(!existsWeightSpectrum())
	    dataManType="";
     
	if(dataManType.contains("Tiled") ){
	  try {
	    
	    ROTiledStManAccessor tacc=ROTiledStManAccessor(thems, 
							   cdesc.dataManagerGroup());
	    
	    /*
	    //This is for the data columns, weight_spectrum and flag only 
	    if((columns[k] != MS::columnName(MS::WEIGHT)) && 
	    (columns[k] != MS::columnName(MS::UVW))){
	    uInt nHyper = tacc.nhypercubes();
	    // Find smallest tile shape
	    Int lowestProduct = 0;
	    Int lowestId = 0;
	    Bool firstFound = False;
	    for (uInt id=0; id < nHyper; id++) {
	    Int product = tacc.getTileShape(id).product();
	    if (product > 0 && (!firstFound || product < lowestProduct)) {
	    lowestProduct = product;
	    lowestId = id;
	    if (!firstFound) firstFound = True;
	    }
	    }
	    Int nchantile=tacc.getTileShape(lowestId)(1);
	    if(nchantile > 0)
	    nchantile=channelGroupSize_p/nchantile+1;
	    if(nchantile<3)
	    nchantile=10;
	    
	    
	    tacc.setCacheSize (0, nchantile);
	    }
	    else
	    */
	    //One tile only for now ...seems to work faster
	    tacc.setCacheSize (0, 1);
	  } catch (AipsError x) {
	    //It failed so leave the caching as is
	    continue;
	  }
	  
	}
      }
    }
  }
  
}

/*
void ROVisibilityIterator::setTileCache(){
  // This function sets the tile cache because of a feature in 
  // sliced data access that grows memory dramatically in some cases
  //if(useSlicer_p){
  
  {   
    const MeasurementSet& thems=msIter_p.ms();
    const ColumnDescSet& cds=thems.tableDesc().columnDescSet();
    ROArrayColumn<Complex> colVis;
    ROArrayColumn<Float> colwgt;
    Vector<String> columns(3);
    columns(0)=MS::columnName(MS::DATA);
    columns(1)=MS::columnName(MS::CORRECTED_DATA);
    columns(2)=MS::columnName(MS::MODEL_DATA);
    //cout << "COL " << columns << endl;
    for (uInt k=0; k< 3; ++k){
      //cout << "IN loop k " << k << endl;
      if (thems.tableDesc().isColumn(columns(k)) ) {
	
	colVis.attach(thems,columns(k));
	String dataManType;
	dataManType = colVis.columnDesc().dataManagerType();
	//cout << "dataManType " << dataManType << endl;
	if(dataManType.contains("Tiled")){
       
	  ROTiledStManAccessor tacc(thems, 
				    colVis.columnDesc().dataManagerGroup());
	  uInt nHyper = tacc.nhypercubes();
	  // Find smallest tile shape
	  Int lowestProduct = 0;
	  Int lowestId = 0;
	  Bool firstFound = False;
	  for (uInt id=0; id < nHyper; id++) {
	    Int product = tacc.getTileShape(id).product();
	    if (product > 0 && (!firstFound || product < lowestProduct)) {
	      lowestProduct = product;
	      lowestId = id;
	      if (!firstFound) firstFound = True;
	    }
	  }
	  Int nchantile;
	  IPosition tileshape=tacc.getTileShape(lowestId);
	  IPosition axisPath(3,2,0,1);
	  //nchantile=tileshape(1);
	  tileshape(1)=channelGroupSize_p;
	  tileshape(2)=curNumRow_p;
	  //cout << "cursorshape " << tileshape << endl;
	  nchantile=tacc.calcCacheSize(0, tileshape, axisPath);
	
	//  if(nchantile > 0)
	 //   nchantile=channelGroupSize_p/nchantile*10;
	 // if(nchantile<3)
	  //  nchantile=10;
	 
	  ///////////////
	  //nchantile *=8;
	  nchantile=1;
	  //tileshape(2)=tileshape(2)*8;
	  //////////////
	  //cout << tacc.cacheSize(0) << " nchantile "<< nchantile << " max cache size " << tacc.maximumCacheSize() << endl; 
	  tacc.setCacheSize (0, tileshape, axisPath);
	  //cout << k << "  " << columns(k) << " cache size  " <<  tacc.cacheSize(0) << endl;
	  
	}
      }
    }
  }
  
}
*/

void ROVisibilityIterator::attachColumns(const Table &t)
{
  const ColumnDescSet& cds=t.tableDesc().columnDescSet();
  colAntenna1.attach(t, MS::columnName(MS::ANTENNA1));
  colAntenna2.attach(t, MS::columnName(MS::ANTENNA2));
  colFeed1.attach(t, MS::columnName(MS::FEED1));
  colFeed2.attach(t, MS::columnName(MS::FEED2));
  colTime.attach(t, MS::columnName(MS::TIME));
  colTimeInterval.attach(t, MS::columnName(MS::INTERVAL));
  if (cds.isDefined(MS::columnName(MS::DATA))) {
    colVis.attach(t, MS::columnName(MS::DATA));
  }
  if (cds.isDefined(MS::columnName(MS::FLOAT_DATA))) {
    colFloatVis.attach(t, MS::columnName(MS::FLOAT_DATA));
    floatDataFound_p=True;
  } else {
    floatDataFound_p=False;
  }
  if (cds.isDefined("MODEL_DATA")) 
    colModelVis.attach(t, "MODEL_DATA");
  if (cds.isDefined("CORRECTED_DATA"))
    colCorrVis.attach(t, "CORRECTED_DATA");
  colUVW.attach(t, MS::columnName(MS::UVW));
  colFlag.attach(t, MS::columnName(MS::FLAG));
  colFlagRow.attach(t, MS::columnName(MS::FLAG_ROW));
  colScan.attach(t, MS::columnName(MS::SCAN_NUMBER));
  colSigma.attach(t, MS::columnName(MS::SIGMA));
  colWeight.attach(t, MS::columnName(MS::WEIGHT));
  if (cds.isDefined("WEIGHT_SPECTRUM")) 
    colWeightSpectrum.attach(t, "WEIGHT_SPECTRUM");
}

ROVisibilityIterator & ROVisibilityIterator::operator++(int)
{
  if (!more_p) return *this;
  advance();
  return *this;
}

ROVisibilityIterator & ROVisibilityIterator::operator++()
{
  if (!more_p) return *this;
  advance();
  return *this;
}

void ROVisibilityIterator::update_rowIds() const
{
  if (rowIds_p.nelements() == 0) {
      rowIds_p = selRows_p.convert();
      
      Vector<uInt> msIter_rowIds(msIter_p.table().rowNumbers(msIter_p.ms()));

      for (uInt i = 0; i < rowIds_p.nelements(); i++) {
          rowIds_p(i) = msIter_rowIds(rowIds_p(i));
      }
  }
  return;
}

Vector<uInt>& ROVisibilityIterator::rowIds(Vector<uInt>& rowids) const
{
  /* Calculate the row numbers in the original MS only when needed,
     i.e. when this function is called */
  update_rowIds();
  rowids.resize(rowIds_p.nelements());
  rowids = rowIds_p;
  return rowids;
}


Vector<Int>& ROVisibilityIterator::antenna1(Vector<Int>& ant1) const
{
  ant1.resize(curNumRow_p);
  getCol(colAntenna1, ant1, MS::columnName(MS::ANTENNA1), allAntenna1);
  return ant1;
}

Vector<Int>& ROVisibilityIterator::antenna2(Vector<Int>& ant2) const
{
  ant2.resize(curNumRow_p);
  getCol(colAntenna2, ant2, MS::columnName(MS::ANTENNA2), allAntenna2);
  return ant2;
}

Vector<Int>& ROVisibilityIterator::feed1(Vector<Int>& fd1) const
{
  fd1.resize(curNumRow_p);
  getCol(colFeed1, fd1, MS::columnName(MS::FEED1), allFeed1);
  return fd1;
}

Vector<Int>& ROVisibilityIterator::feed2(Vector<Int>& fd2) const
{
  fd2.resize(curNumRow_p);
  getCol(colFeed2, fd2, MS::columnName(MS::FEED2), allFeed2);
  return fd2;
}

Vector<Int>& ROVisibilityIterator::channel(Vector<Int>& chan) const
{
  Int spw = msIter_p.spectralWindowId();
  chan.resize(channelGroupSize_p);
  Int inc=chanInc_p[spw] <= 0 ? 1 : chanInc_p[spw];
  for (Int i=0; i<channelGroupSize_p; i++) {
    chan(i)=chanStart_p[spw]+curChanGroup_p*chanWidth_p[spw]+i*inc;
  }
  return chan;
}

Vector<Int>& ROVisibilityIterator::corrType(Vector<Int>& corrTypes) const
{
  Int polId = msIter_p.polarizationId();
  msIter_p.msColumns().polarization().corrType().get(polId,corrTypes,True);
  return corrTypes;
}

Cube<Bool>& ROVisibilityIterator::flag(Cube<Bool>& flags) const
{
  if (velSelection_p) {
    if (!flagOK_p) {
      // need to do the interpolation
      getInterpolatedVisFlagWeight(Corrected);
      This->flagOK_p=This->visOK_p[Corrected]=This->weightSpOK_p=True;
    }
    flags.resize(flagCube_p.shape());  flags=flagCube_p; 
  } else {
    if (useSlicer_p) getCol(colFlag, slicer_p,flags,True);
    else getCol(colFlag, flags,True);
  }
  return flags;
}

Matrix<Bool>& ROVisibilityIterator::flag(Matrix<Bool>& flags) const
{
  if (useSlicer_p) getCol(colFlag, slicer_p,This->flagCube_p,True);
  else getCol(colFlag, This->flagCube_p,True);
  flags.resize(channelGroupSize_p,curNumRow_p);
  // need to optimize this...
  //for (Int row=0; row<curNumRow_p; row++) {
  //  for (Int chn=0; chn<channelGroupSize_p; chn++) {
  //    flags(chn,row)=flagCube(0,chn,row);
  //    for (Int pol=1; pol<nPol_p; pol++) {
  //	  flags(chn,row)|=flagCube(pol,chn,row);
  //    }
  //  }
  //}
  Bool deleteIt1;
  Bool deleteIt2;
  const Bool* pcube=This->flagCube_p.getStorage(deleteIt1);
  Bool* pflags=flags.getStorage(deleteIt2);
  for (uInt row=0; row<curNumRow_p; row++) {
    for (Int chn=0; chn<channelGroupSize_p; chn++) {
      *pflags=*pcube++;
      for (Int pol=1; pol<nPol_p; pol++, pcube++) {
	*pflags = *pcube ? *pcube : *pflags;
      }
      pflags++;
    }
  }
  flagCube_p.freeStorage(pcube, deleteIt1);
  flags.putStorage(pflags, deleteIt2);
  return flags;
}

Vector<Bool>& ROVisibilityIterator::flagRow(Vector<Bool>& rowflags) const
{
  rowflags.resize(curNumRow_p);
  getCol(colFlagRow, rowflags, MS::columnName(MS::FLAG_ROW), allRowflags);
  return rowflags;
}

Vector<Int>& ROVisibilityIterator::scan(Vector<Int>& scans) const
{
    scans.resize(curNumRow_p);
    getCol(colScan, scans, MS::columnName(MS::SCAN_NUMBER), allScans);
    return scans;
}

Vector<Double>& ROVisibilityIterator::frequency(Vector<Double>& freq) const
{
  if (velSelection_p) {
    freq.resize(nVelChan_p);
    freq=selFreq_p;
  } else {
    if (!freqCacheOK_p) {
      This->freqCacheOK_p=True;
      Int spw = msIter_p.spectralWindowId();
      This->frequency_p.resize(channelGroupSize_p);
      const Vector<Double>& chanFreq=msIter_p.frequency();
      Int start=chanStart_p[spw]-msIter_p.startChan();
      Int inc=chanInc_p[spw] <= 0 ? 1 : chanInc_p[spw];
      for (Int i=0; i<channelGroupSize_p; i++) {
	This->frequency_p(i)=chanFreq(start+curChanGroup_p*chanWidth_p[spw]+i*inc);
      }
    }
    freq.resize(channelGroupSize_p);
    freq=frequency_p;
  }
  return freq;
}

Vector<Double>& ROVisibilityIterator::lsrFrequency(Vector<Double>& freq) const
{
  if (velSelection_p) {
    freq.resize(nVelChan_p);
    freq=lsrFreq_p;
  } else {
    // if there is no vel selection, we just return the observing freqs
    frequency(freq);
  }
  return freq;
}

Vector<Double>& ROVisibilityIterator::time(Vector<Double>& t) const
{
  t.resize(curNumRow_p);

  getCol(colTime, t, MS::columnName(MS::TIME), allTimes);

  return t;
}

Vector<Double>& ROVisibilityIterator::timeInterval(Vector<Double>& t) const
{
  t.resize(curNumRow_p);
  getCol(colTimeInterval, t, MS::columnName(MS::INTERVAL), allTimeInterval);
  return t;
}

Cube<Complex>& 
ROVisibilityIterator::visibility(Cube<Complex>& vis, DataColumn whichOne) const
{
  if (velSelection_p) {
    if (!visOK_p[whichOne]) {
      getInterpolatedVisFlagWeight(whichOne);
      This->visOK_p[whichOne]=This->flagOK_p=This->weightSpOK_p=True;
    }
    vis.resize(visCube_p.shape()); vis=visCube_p;
  } else { 
    if (useSlicer_p) getDataColumn(whichOne,slicer_p,vis);
    else getDataColumn(whichOne,vis);
  }
  return vis;
}

// helper function to swap the y and z axes of a Cube
void swapyz(Cube<Complex>& out, const Cube<Complex>& in)
{
  IPosition inShape=in.shape();
  uInt nx=inShape(0),ny=inShape(2),nz=inShape(1);
  out.resize(nx,ny,nz);
  Bool deleteIn,deleteOut;
  const Complex* pin = in.getStorage(deleteIn);
  Complex* pout = out.getStorage(deleteOut);
  uInt i=0, zOffset=0;
  for (uInt iz=0; iz<nz; iz++, zOffset+=nx) {
    Int yOffset=zOffset;
    for (uInt iy=0; iy<ny; iy++, yOffset+=nx*nz) {
      for (uInt ix=0; ix<nx; ix++) pout[i++] = pin[ix+yOffset];
    }
  }
  out.putStorage(pout,deleteOut);
  in.freeStorage(pin,deleteIn);
}

// helper function to swap the y and z axes of a Cube
void swapyz(Cube<Bool>& out, const Cube<Bool>& in)
{
  IPosition inShape=in.shape();
  uInt nx=inShape(0),ny=inShape(2),nz=inShape(1);
  out.resize(nx,ny,nz);
  Bool deleteIn,deleteOut;
  const Bool* pin = in.getStorage(deleteIn);
  Bool* pout = out.getStorage(deleteOut);
  uInt i=0, zOffset=0;
  for (uInt iz=0; iz<nz; iz++, zOffset+=nx) {
    Int yOffset=zOffset;
    for (uInt iy=0; iy<ny; iy++, yOffset+=nx*nz) {
      for (uInt ix=0; ix<nx; ix++) pout[i++] = pin[ix+yOffset];
    }
  }
}

// transpose a matrix
void transpose(Matrix<Float>& out, const Matrix<Float>& in)
{
  uInt ny=in.nrow(), nx=in.ncolumn();
  out.resize(nx,ny);
  Bool deleteIn,deleteOut;
  const Float* pin = in.getStorage(deleteIn);
  Float* pout = out.getStorage(deleteOut);
  uInt i=0;
  for (uInt iy=0; iy<ny; iy++) {
    uInt yOffset=0;
    for (uInt ix=0; ix<nx; ix++, yOffset+=ny) pout[i++] = pin[iy+yOffset];
  }
  out.putStorage(pout,deleteOut);
  in.freeStorage(pin,deleteIn);
}
void ROVisibilityIterator::getInterpolatedVisFlagWeight(DataColumn whichOne)
     const
{
  // get vis, flags & weights
  // tricky.. to avoid recursion we need to set velSelection_p to False
  // temporarily.
  This->velSelection_p = False; 
  visibility(This->visCube_p, whichOne);
  flag(This->flagCube_p); 
  imagingWeight(This->imagingWeight_p);
  Vector<Double> freq; frequency(freq);
  This->velSelection_p = True;

  // now interpolate visibilities, using selFreq as the sample points
  // we should have two options: flagging output points that have
  // any flagged inputs or interpolating across flagged data.
  // Convert frequencies to float (removing offset to keep accuracy) 
  // so we can multiply them with Complex numbers to do the interpolation.
  Block<Float> xfreq(channelGroupSize_p),sfreq(nVelChan_p); 
  Int i;
  for (i=0; i<channelGroupSize_p; i++) xfreq[i]=freq(i)-freq(0);
  for (i=0; i<nVelChan_p; i++) sfreq[i]=selFreq_p(i)-freq(0);
  // we should probably be using the flags for weight interpolation as well
  // but it's not clear how to combine the 4 pol flags into one.
  // (AND the flags-> weight flagged if all flagged?)
  Cube<Complex> vis,intVis;
  swapyz(vis,visCube_p);
  Cube<Bool> flag,intFlag;
  swapyz(flag,flagCube_p);
  Matrix<Float> wt,intWt;
  transpose(wt,imagingWeight_p);
  InterpolateArray1D<Float,Complex>::InterpolationMethod method1=
    InterpolateArray1D<Float,Complex>::linear;
  InterpolateArray1D<Float,Float>::InterpolationMethod method2=
    InterpolateArray1D<Float,Float>::linear;
  if (vInterpolation_p=="nearest") {
    method1=InterpolateArray1D<Float,Complex>::nearestNeighbour;
    method2= InterpolateArray1D<Float,Float>::nearestNeighbour;
  }
  InterpolateArray1D<Float,Complex>::
    interpolate(intVis,intFlag,sfreq,xfreq,vis,flag,method1);
  InterpolateArray1D<Float,Float>::interpolate(intWt,sfreq,xfreq,wt,method2);
  swapyz(This->visCube_p,intVis);
  swapyz(This->flagCube_p,intFlag);
  transpose(This->imagingWeight_p,intWt);
}

void ROVisibilityIterator::getDataColumn(DataColumn whichOne, 
					 const Slicer& slicer,
					 Cube<Complex>& data) const
{

 
  // Return the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat;
      getCol(colFloatVis, slicer,dataFloat,True);
      data.resize(dataFloat.shape());
      convertArray(data,dataFloat);
    } else {
      getCol(colVis, slicer,data,True);
    }
    break;
  case Corrected:
    getCol(colCorrVis, slicer,data,True);
    break;
  case Model:
    getCol(colModelVis, slicer,data,True);
    break;
  }
 
}

void ROVisibilityIterator::getDataColumn(DataColumn whichOne,
					 Cube<Complex>& data) const
{
  // Return the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat;
      getCol(colFloatVis, dataFloat,True);
      data.resize(dataFloat.shape());
      convertArray(data,dataFloat);
    } else {
      getCol(colVis, data,True);
    }
    break;
  case Corrected:
    getCol(colCorrVis, data,True);
    break;
  case Model:
    getCol(colModelVis, data,True);
    break;
  }
}  

Matrix<CStokesVector>& 
ROVisibilityIterator::visibility(Matrix<CStokesVector>& vis,
				 DataColumn whichOne) const
{
  if (useSlicer_p) getDataColumn(whichOne,slicer_p,This->visCube_p);
  else getDataColumn(whichOne,This->visCube_p);
  vis.resize(channelGroupSize_p,curNumRow_p);
  Bool deleteIt;
  Complex* pcube=This->visCube_p.getStorage(deleteIt);
  if (deleteIt) cerr<<"Problem in ROVisIter::visibility - deleteIt True"<<endl;
  // Here we cope in a limited way with cases where not all 4 
  // polarizations are present: if only 2, assume XX,YY or RR,LL
  // if only 1, assume it's an estimate of Stokes I (one of RR,LL,XX,YY)
  // The cross terms are zero filled in these cases.
  switch (nPol_p) {
  case 4: {
    for (uInt row=0; row<curNumRow_p; row++) {
      for (Int chn=0; chn<channelGroupSize_p; chn++,pcube+=4) {
	vis(chn,row)=pcube;
      }
    }
    break;
  }
  case 2: {
    vis.set(Complex(0.,0.));
    for (uInt row=0; row<curNumRow_p; row++) {
      for (Int chn=0; chn<channelGroupSize_p; chn++,pcube+=2) {
	CStokesVector& v=vis(chn,row);
	v(0)=*pcube; 
	v(3)=*(pcube+1); 
      }
    }
    break;
  }
  case 1: {
    vis.set(Complex(0.,0.));
    for (uInt row=0; row<curNumRow_p; row++) {
      for (Int chn=0; chn<channelGroupSize_p; chn++,pcube++) {
	CStokesVector& v=vis(chn,row);
	v(0)=v(3)=*pcube; 
      }
    }
  } //# case 1
  } //# switch 
  return vis;
}

Vector<RigidVector<Double,3> >& 
ROVisibilityIterator::uvw(Vector<RigidVector<Double,3> >& uvwvec) const
{
    uvwvec.resize(curNumRow_p);
    getColArray<Double>(colUVW, This->uvwMat_p, MS::columnName(MS::UVW), allUVW, True);
    // get a pointer to the raw storage for quick access
    Bool deleteIt;
    Double* pmat=This->uvwMat_p.getStorage(deleteIt);
    for (uInt row=0; row<curNumRow_p; row++, pmat+=3) uvwvec(row)=pmat;
    return uvwvec;
}

Matrix<Double>& ROVisibilityIterator::uvwMat(Matrix<Double>& uvwmat) const
{
    getCol(colUVW, uvwmat,True);
    return uvwmat;
}

// Fill in parallactic angle.
const Vector<Float>& ROVisibilityIterator::feed_pa(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","feed_pa"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastfeedpaUT_p) {
    This->lastfeedpaUT_p=ut;

    // Set up the Epoch using the absolute MJD in seconds
    // get the Epoch reference from the column
    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
    //     now set the value
    mEpoch.set(MVEpoch(Quantity(ut, "s")));

    This->msd_p.setEpoch(mEpoch);

    // Calculate pa for all antennas.
    Int nAnt = msIter_p.receptorAngle().shape()(1);
    for (Int iant=0;iant<nAnt;iant++) {
      This->msd_p.setAntenna(iant);
      This->feedpa_p(iant) = This->msd_p.parAngle();
      // add angle for receptor 0
      This->feedpa_p(iant)+= msIter_p.receptorAngle()(0,iant);
      if (aips_debug) {
	if (iant==0) 
	  cout<<"Antenna "<<iant<<" at time: "<<MVTime(mEpoch.getValue())<<
	  " has PA = "<<This->feedpa_p(iant)*57.28<<endl;
      }
    }
  }
  return feedpa_p;
}


// Fill in parallactic angle.
const Float& ROVisibilityIterator::parang0(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","parang0"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastfeedpaUT_p) {
    This->lastfeedpaUT_p=ut;

    // Set up the Epoch using the absolute MJD in seconds
    // get the Epoch reference from the column
    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
    //     now set the value
    mEpoch.set(MVEpoch(Quantity(ut, "s")));

    This->msd_p.setEpoch(mEpoch);

    // Calculate pa for all antennas.
    This->msd_p.setAntenna(-1);
    This->parang0_p = This->msd_p.parAngle();
    if (aips_debug) 
      cout<<"At time: "<<MVTime(mEpoch.getValue())<<
	" PA = "<<This->parang0_p*57.28<<" deg"<<endl;
  }
  return parang0_p;
}


// Fill in parallactic angle (NO FEED PA offset!).
const Vector<Float>& ROVisibilityIterator::parang(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","parang"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastfeedpaUT_p) {
    This->lastfeedpaUT_p=ut;

    // Set up the Epoch using the absolute MJD in seconds
    // get the Epoch reference from the column
    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
    //     now set the value
    mEpoch.set(MVEpoch(Quantity(ut, "s")));

    This->msd_p.setEpoch(mEpoch);

    // Calculate pa for all antennas.
    Int nAnt = msIter_p.receptorAngle().shape()(1);
    for (Int iant=0;iant<nAnt;iant++) {
      This->msd_p.setAntenna(iant);
      This->parang_p(iant) = This->msd_p.parAngle();
      if (aips_debug) {
	if (iant==0) 
	  cout<<"Antenna "<<iant<<" at time: "<<MVTime(mEpoch.getValue())<<
	  " has PA = "<<This->parang_p(iant)*57.28<<endl;
      }
    }
  }
  return parang_p;
}

// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
const Vector<MDirection>& ROVisibilityIterator::azel(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","azel"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastazelUT_p) {
    This->lastazelUT_p=ut;

    // Set up the Epoch using the absolute MJD in seconds
    // get the Epoch reference from the column keyword
    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
    //     now set the value
    mEpoch.set(MVEpoch(Quantity(ut, "s")));

    This->msd_p.setEpoch(mEpoch);

    // Calculate az/el for all antennas.
    Int nAnt = msIter_p.receptorAngle().shape()(1);
    for (Int iant=0;iant<nAnt;iant++) {
      This->msd_p.setAntenna(iant);
      This->azel_p(iant) = This->msd_p.azel();
      if (aips_debug) {
	if (iant==0) 
	  cout<<"Antenna "<<iant<<" at time: "<<MVTime(mEpoch.getValue())<<
	  " has AzEl = "<<This->azel_p(iant).getAngle("deg")<<endl;
      }
    }
  }
  return azel_p;
}

// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
const MDirection& ROVisibilityIterator::azel0(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","azel0"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastazelUT_p) {
    This->lastazelUT_p=ut;

    // Set up the Epoch using the absolute MJD in seconds
    // get the Epoch reference from the column keyword
    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
    //     now set the value
    mEpoch.set(MVEpoch(Quantity(ut, "s")));

    This->msd_p.setEpoch(mEpoch);

    This->msd_p.setAntenna(-1);
    This->azel0_p = This->msd_p.azel();
    if (aips_debug)
      cout<<"At time: "<<MVTime(mEpoch.getValue())<<
	" AzEl = "<<This->azel0_p.getAngle("deg")<<endl;
  }
  return azel0_p;
}

// Hour angle at specified time.
const Double& ROVisibilityIterator::hourang(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","azel"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastazelUT_p) {
    This->lastazelUT_p=ut;

    // Set up the Epoch using the absolute MJD in seconds
    // get the Epoch reference from the column keyword
    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
    //     now set the value
    mEpoch.set(MVEpoch(Quantity(ut, "s")));

    This->msd_p.setEpoch(mEpoch);

    This->msd_p.setAntenna(-1);

    This->hourang_p = This->msd_p.hourAngle();

  }
  return hourang_p;

}

Vector<Float>& ROVisibilityIterator::sigma(Vector<Float>& sig) const
{
  Matrix<Float> sigmat;
  getCol(colSigma, sigmat);
  // Do a rough average of the parallel hand polarizations to get a single 
  // sigma. Should do this properly someday, or return all values
  sig.resize(sigmat.ncolumn());
  sig=sigmat.row(0);
  sig+=sigmat.row(nPol_p-1);
  sig/=2.0f;
  return sig;
}

Matrix<Float>& ROVisibilityIterator::sigmaMat(Matrix<Float>& sigmat) const
{
  sigmat.resize(nPol_p,curNumRow_p);
  getCol(colSigma, sigmat);
  return sigmat;
}

Vector<Float>& ROVisibilityIterator::weight(Vector<Float>& wt) const
{
  // Take average of parallel hand polarizations for now.
  // Later convert weight() to return full polarization dependence
  Matrix<Float> polWeight;
  getCol(colWeight, polWeight);
  wt.resize(polWeight.ncolumn());
  wt=polWeight.row(0);
  wt+=polWeight.row(nPol_p-1);
  wt/=2.0f;
  return wt;
}

Matrix<Float>& ROVisibilityIterator::weightMat(Matrix<Float>& wtmat) const
{
  wtmat.resize(nPol_p,curNumRow_p);
  getCol(colWeight, wtmat);
  return wtmat;
}


Bool ROVisibilityIterator::existsWeightSpectrum() const
{
  Bool rstat(False);

  try {
    rstat = (!colWeightSpectrum.isNull() &&
	     colWeightSpectrum.shape(0).isEqual(IPosition(2,nPol_p,channelGroupSize())));
  } catch (AipsError x) {
    rstat = False;
  }
  return rstat;
}


Cube<Float>& ROVisibilityIterator::weightSpectrum(Cube<Float>& wtsp) const
{
  if (!colWeightSpectrum.isNull()) {
    wtsp.resize(nPol_p,nChan_p,curNumRow_p);
    getCol(colWeightSpectrum, wtsp);
  } else {
    wtsp.resize(0,0,0);
  }
  return wtsp;
}

Matrix<Float>& ROVisibilityIterator::imagingWeight(Matrix<Float>& wt) const
{
    if(imwgt_p.getType() == "none")
        throw(AipsError("Programmer Error... imaging weights not set"));
    Vector<Float> weightvec;
    weight(weightvec);
    Matrix<Bool> flagmat;
    flag(flagmat);
    wt.resize(flagmat.shape());
    if(imwgt_p.getType()=="uniform"){
        Vector<Double> fvec;
        frequency(fvec);
        Matrix<Double> uvwmat;
        uvwMat(uvwmat);
        imwgt_p.weightUniform(wt, flagmat, uvwmat, fvec, weightvec);
        if(imwgt_p.doFilter())
	    imwgt_p.filter(wt, flagmat, uvwmat, fvec, weightvec);
    }
    else if(imwgt_p.getType()=="radial"){
        Vector<Double> fvec;
        frequency(fvec);
        Matrix<Double> uvwmat;
        uvwMat(uvwmat);
        imwgt_p.weightRadial(wt, flagmat, uvwmat, fvec, weightvec);
        if(imwgt_p.doFilter())
	    imwgt_p.filter(wt, flagmat, uvwmat, fvec, weightvec);
    }
    else{
	imwgt_p.weightNatural(wt, flagmat, weightvec);
	if(imwgt_p.doFilter()){
            Matrix<Double> uvwmat;
            uvwMat(uvwmat);
            Vector<Double> fvec;
            frequency(fvec);
            imwgt_p.filter(wt, flagmat, uvwmat, fvec, weightvec);

	}
    }

    return wt;
}

Int ROVisibilityIterator::nSubInterval() const
{
// Return the number of sub-intervals in the current chunk,
// i.e. the number of unique time stamps
//
  // Find all unique times in time_p
  Int retval=0;
  uInt nTimes=time_p.nelements();
  if (nTimes > 0) {
  
    Vector<Double> times(time_p); /* Do not change time_p, make a copy */
    Bool deleteIt;
    Double *tp = times.getStorage(deleteIt);

    std::sort(tp, tp + nTimes);
    
    /* Count unique times */
    retval = 1;
    for (unsigned i = 0; i < nTimes - 1; i++) {
      if (tp[i] < tp[i+1]) retval += 1;
    }
  }
  return retval;
}

ROVisibilityIterator& 
ROVisibilityIterator::selectVelocity
(Int nChan, const MVRadialVelocity& vStart, const MVRadialVelocity& vInc,
 MRadialVelocity::Types rvType, MDoppler::Types dType, Bool precise)
{
  if (!initialized_p) {
    // initialize the base iterator only (avoid recursive call to originChunks)
    if (!msIterAtOrigin_p) {
      msIter_p.origin();
      msIterAtOrigin_p=True;
      stateOk_p=False;
    }
  }    
  velSelection_p=True;
  nVelChan_p=nChan;
  vStart_p=vStart;
  vInc_p=vInc;
  msd_p.setVelocityFrame(rvType);
  vDef_p=dType;
  cFromBETA_p.set(MDoppler(MVDoppler(Quantity(0.,"m/s")),
			   MDoppler::BETA),vDef_p);
  vPrecise_p=precise;
  if (precise) {
    // set up conversion engine for full conversion
  }
  // have to reset the iterator so all caches get filled
  originChunks();
  return *this;
}


ROVisibilityIterator&  
ROVisibilityIterator::selectChannel(Int nGroup, Int start, Int width, 
				    Int increment, Int spectralWindow)
{

  if (!initialized_p) {
    // initialize the base iterator only (avoid recursive call to originChunks)
    if (!msIterAtOrigin_p) {
      msIter_p.origin();
      msIterAtOrigin_p=True;
      stateOk_p=False;
    }
  }    
  Int spw=spectralWindow;
  if (spw<0) spw = msIter_p.spectralWindowId();
  Int n = numChanGroup_p.nelements();
  if(n==0){
    blockSpw_p.resize(1, True, False);
    blockSpw_p[0].resize(1);
    blockSpw_p[0][0]=spw;
    blockNumChanGroup_p.resize(1,True,False);
    blockNumChanGroup_p[0].resize(1);
    blockNumChanGroup_p[0][0]=nGroup;
    blockChanStart_p.resize(1, True, False);
    blockChanStart_p[0].resize(1);
    blockChanStart_p[0][0]=start;
    blockChanWidth_p.resize(1, True, False);
    blockChanWidth_p[0].resize(1);
    blockChanWidth_p[0][0]=width;
    blockChanInc_p.resize(1, True, False);
    blockChanInc_p[0].resize(1);
    blockChanInc_p[0][0]=increment;
    msCounter_p=0;



  }
  else{
    Bool hasSpw=False;
    Int spwIndex=-1;
    for (uInt k = 0; k < blockSpw_p[0].nelements(); ++k){
      if(spw==blockSpw_p[0][k]){
	hasSpw=True;
	spwIndex=k;
	break;
      }
    }
    if(!hasSpw){
      Int nspw=blockSpw_p[0].nelements()+1;
      blockSpw_p[0].resize(nspw, True);
      blockSpw_p[0][nspw-1]=spw;
      blockNumChanGroup_p[0].resize(nspw,True);
      blockNumChanGroup_p[0][nspw-1]=nGroup;
      blockChanStart_p[0].resize(nspw, True);
      blockChanStart_p[0][nspw-1]=start;
      blockChanWidth_p[0].resize(nspw, True);
      blockChanWidth_p[0][nspw-1]=width;
      blockChanInc_p[0].resize(nspw, True);
      blockChanInc_p[0][nspw-1]=increment;
    }
    else{
      blockSpw_p[0][spwIndex]=spw;
      blockNumChanGroup_p[0][spwIndex]=nGroup;
      blockChanStart_p[0][spwIndex]=start;
      blockChanWidth_p[0][spwIndex]=width;
      blockChanInc_p[0][spwIndex]=increment;
    }


  }
  if (spw >= n) {
    // we need to resize the blocks
    Int newn = max(2,max(2*n,spw+1));
    numChanGroup_p.resize(newn);
    chanStart_p.resize(newn);
    chanWidth_p.resize(newn);
    chanInc_p.resize(newn);
    for (Int i = n; i<newn; i++) numChanGroup_p[i] = 0;
  }
  chanStart_p[spw] = start;
  chanWidth_p[spw] = width;

  chanInc_p[spw] = increment;
  numChanGroup_p[spw] = nGroup;
  // have to reset the iterator so all caches get filled & slicer sizes
  // get updated
  //  originChunks();
  /*
  if(msIterAtOrigin_p){
    if(!isInSelectedSPW(msIter_p.spectralWindowId())){
      while((!isInSelectedSPW(msIter_p.spectralWindowId())) 
	    && (msIter_p.more()))
	msIter_p++;
      stateOk_p=False;
      setState();
    }
  }
  */
  //leave the state where msiter is pointing
  channelGroupSize_p = chanWidth_p[msIter_p.spectralWindowId()];
  curNumChanGroup_p = numChanGroup_p[msIter_p.spectralWindowId()];

  return *this;
}

ROVisibilityIterator&  
ROVisibilityIterator::selectChannel(Block<Vector<Int> >& blockNGroup, 
				    Block<Vector<Int> >& blockStart, 
				    Block<Vector<Int> >& blockWidth, 
				    Block<Vector<Int> >& blockIncr,
				    Block<Vector<Int> >& blockSpw)
{
  /*
  No longer needed
  if(!isMultiMS_p){
    //Programmer error ...so should not reach here
    cout << "Cannot use this function if Visiter was not constructed with multi-ms" 
	 << endl;
  }
  */

  blockNumChanGroup_p.resize(0, True, False);
  blockNumChanGroup_p=blockNGroup;
  blockChanStart_p.resize(0, True, False);
  blockChanStart_p=blockStart;
  blockChanWidth_p.resize(0, True, False);
  blockChanWidth_p=blockWidth;
  blockChanInc_p.resize(0, True, False);
  blockChanInc_p=blockIncr;
  blockSpw_p.resize(0, True, False);
  blockSpw_p=blockSpw;

  if (!initialized_p) {
    // initialize the base iterator only (avoid recursive call to originChunks)
    if (!msIterAtOrigin_p) {
      msIter_p.origin();
      msIterAtOrigin_p=True;
      stateOk_p=False;
    }
  }    

  numChanGroup_p.resize(0);
  msCounter_p=0;

  doChannelSelection();
  // have to reset the iterator so all caches get filled & slicer sizes
  // get updated
  
  if(msIterAtOrigin_p){
    if(!isInSelectedSPW(msIter_p.spectralWindowId())){
      while((!isInSelectedSPW(msIter_p.spectralWindowId())) 
	    && (msIter_p.more()))
	msIter_p++;
      stateOk_p=False;
    }
    
  }
  
  originChunks();
  return *this;
}


void ROVisibilityIterator::getChannelSelection(
					       Block< Vector<Int> >& blockNGroup,
					       Block< Vector<Int> >& blockStart,
					       Block< Vector<Int> >& blockWidth,
					       Block< Vector<Int> >& blockIncr,
					       Block< Vector<Int> >& blockSpw){


  blockNGroup.resize(0, True, False);
  blockNGroup=blockNumChanGroup_p;
  blockStart.resize(0, True, False);
  blockStart=blockChanStart_p;
  blockWidth.resize(0, True, False);
  blockWidth=blockChanWidth_p;
  blockIncr.resize(0, True, False);
  blockIncr=blockChanInc_p;
  blockSpw.resize(0, True, False);
  blockSpw=blockSpw_p;



}
void  ROVisibilityIterator::doChannelSelection()
{


  
  for (uInt k=0; k < blockSpw_p[msCounter_p].nelements(); ++k){
    Int spw=blockSpw_p[msCounter_p][k];
    if (spw<0) spw = msIter_p.spectralWindowId();
    Int n = numChanGroup_p.nelements();
    if (spw >= n) {
      // we need to resize the blocks
      Int newn = max(2,max(2*n,spw+1));
      numChanGroup_p.resize(newn, True, True);
      chanStart_p.resize(newn, True, True);
      chanWidth_p.resize(newn, True, True);
      chanInc_p.resize(newn, True, True);
      for (Int i = n; i<newn; i++) numChanGroup_p[i] = 0;
    }
   
    chanStart_p[spw] = blockChanStart_p[msCounter_p][k];
    chanWidth_p[spw] = blockChanWidth_p[msCounter_p][k];
    channelGroupSize_p = blockChanWidth_p[msCounter_p][k];
    chanInc_p[spw] = blockChanInc_p[msCounter_p][k];
    numChanGroup_p[spw] = blockNumChanGroup_p[msCounter_p][k];
    curNumChanGroup_p = blockNumChanGroup_p[msCounter_p][k];
    
  }
  Int spw=msIter_p.spectralWindowId();
  Int spIndex=-1;
  for (uInt k=0; k < blockSpw_p[msCounter_p].nelements(); ++k){
    if(spw==blockSpw_p[msCounter_p][k]){
      spIndex=k;
      break;
    }
  }

 
  if(spIndex < 0)
    spIndex=0;
  //leave this at the stage where msiter is pointing
  channelGroupSize_p = blockChanWidth_p[msCounter_p][spIndex];
  curNumChanGroup_p = blockNumChanGroup_p[msCounter_p][spIndex];



}

void  ROVisibilityIterator::getSpwInFreqRange(Block<Vector<Int> >& spw, 
					      Block<Vector<Int> >& start, 
					      Block<Vector<Int> >& nchan, 
					      Double freqStart, 
					      Double freqEnd, 
					      Double freqStep){


  msIter_p.getSpwInFreqRange(spw, start, nchan, 
			     freqStart, freqEnd, freqStep);



}

void ROVisibilityIterator::allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan){

  spws.resize();
  spws=blockSpw_p[msId()];
  nvischan.resize();
  nvischan.resize(max(spws)+1);
  nvischan.set(-1);
  for (uInt k=0; k < spws.nelements(); ++k){
      nvischan[spws[k]]=chanWidth_p[spws[k]];
  }

}


void ROVisibilityIterator::lsrFrequency(const Int& spw, Vector<Double>& freq, 
					Bool& convert){

  // This method is not good for conversion between frames which are extremely
  // time dependent over the course of the observation e.g topo to lsr unless
  // the epoch is in the actual buffer


  if (velSelection_p) {
    getTopoFreqs();
    lsrFrequency(freq);
    return;
  }

  if (!freqCacheOK_p) {
    frequency(freq);   
  }

  Vector<Double> chanFreq(0);
  chanFreq=msIter_p.msColumns().spectralWindow().chanFreq()(spw);
  //      Int start=chanStart_p[spw]-msIter_p.startChan();
  //Assuming that the spectral windows selected is not a reference ms from 
  //visset ...as this will have a start chan offseted may be.

  
  Int start=chanStart_p[spw]; 
  freq.resize(chanWidth_p[spw]);  
  MFrequency::Types obsMFreqType=(MFrequency::Types)(msIter_p.msColumns().spectralWindow().measFreqRef()(spw));
  // Setting epoch to the first in this iteration
  MEpoch ep=msIter_p.msColumns().timeMeas()(0);
  MPosition obsPos=msIter_p.telescopePosition();
  MDirection dir=msIter_p.phaseCenter();
  MeasFrame frame(ep, obsPos, dir);
  MFrequency::Convert tolsr(obsMFreqType, 
  			    MFrequency::Ref(MFrequency::LSRK, frame));


  if(obsMFreqType != MFrequency::LSRK){
    convert=True;
  }


  for (Int i=0; i<chanWidth_p[spw]; i++) {
    Int inc=chanInc_p[spw] <= 0 ? 1 : chanInc_p[spw] ; 
    if(convert){
      freq[i]=tolsr(chanFreq(start+
			     (numChanGroup_p[spw]-1)*chanWidth_p[spw]+i*inc)).
	getValue().getValue();
    }
    else{
      freq[i]=chanFreq(start+
		       (numChanGroup_p[spw]-1)*chanWidth_p[spw]+i*inc);
    }
  }

}

void ROVisibilityIterator::attachVisBuffer(VisBuffer& vb)
{
  vbStack_p.push((void*)&vb);
  vb.invalidate();
}

void ROVisibilityIterator::detachVisBuffer(VisBuffer& vb)
{
  if (!vbStack_p.empty()) {
    if ((void*)vbStack_p.top() == (void*)&vb) {
      vbStack_p.pop();
      if (!vbStack_p.empty()) ((VisBuffer*)vbStack_p.top())->invalidate();
    } else {
      throw(AipsError("ROVisIter::detachVisBuffer - attempt to detach "
		      "buffer that is not the last one attached"));
    }
  }
}

Int ROVisibilityIterator::numberAnt(){
  return msColumns().antenna().nrow(); // for single (sub)array only..
}

Int ROVisibilityIterator::numberSpw(){
  return msColumns().spectralWindow().nrow();
}

Int ROVisibilityIterator::numberDDId(){
  return msColumns().dataDescription().nrow(); 
}

Int ROVisibilityIterator::numberPol(){
  return msColumns().polarization().nrow(); 
}

Int ROVisibilityIterator::numberCoh(){
  Int numcoh=0;
  for (uInt k=0; k < uInt(msIter_p.numMS()) ; ++k){
    numcoh+=msIter_p.ms(k).nrow();
  }
  return numcoh;
  
}

template<class T>
void ROVisibilityIterator::getColScalar(const ROScalarColumn<T> &column, Vector<T> &array, const String &colName, Vector<T> &all, Bool resize) const
{
    if (!slurp_p) {
        column.getColumnCells(selRows_p, array, resize);
        return;
    }

    if (all.nelements() == 0) {
        ScalarColumn<T> colAll(msIter_p.ms(), colName);
        try {
            all = colAll.getColumn();
        }
        catch (...) {
            // If out of memory, do it the slow way.
            // Sorry for the generic catch but it is undocumented, 
            // what getColumn() throws in case of OOM,
            // so this is probably the safest things to do.
            // If there are other problems the following call
            // is likely to throw too.
            column.getColumnCells(selRows_p, array, resize);
            return;
        }
    }

    update_rowIds();

    if (resize && array.shape().nelements() != curNumRow_p) {
        array.resize(curNumRow_p);
    }

    for (unsigned i = 0; i < curNumRow_p; i++) {
        array(i) = all(rowIds_p(i));
    }
    return;
}
void ROVisibilityIterator::getCol(const ROScalarColumn<Bool> &column, Vector<Bool> &array, const String &colName, Vector<Bool> &all, Bool resize) const
{
    getColScalar<Bool>(column, array, colName, all, resize);
}

void ROVisibilityIterator::getCol(const ROScalarColumn<Int> &column, Vector<Int> &array, const String &colName, Vector<Int> &all, Bool resize) const
{
    getColScalar<Int>(column, array, colName, all, resize);
}

void ROVisibilityIterator::getCol(const ROScalarColumn<Double> &column, Vector<Double> &array, const String &colName, Vector<Double> &all, Bool resize) const
{
    getColScalar<Double>(column, array, colName, all, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Bool> &column, Array<Bool> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Float> &column, Array<Float> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

template<class T>
void ROVisibilityIterator::getColArray(const ROArrayColumn<T> &column, Array<T> &array, const String &colName, Array<T> &all, Bool resize) const
{
    if (!slurp_p) {
        column.getColumnCells(selRows_p, array, resize);
        return;
    }

    if (all.nelements() == 0) {
        ArrayColumn<T> colAll(msIter_p.ms(), colName);
        try {
            all = colAll.getColumn();
        }
        catch (...) {
            column.getColumnCells(selRows_p, array, resize);
            return;
        }
    }

    update_rowIds();

    // Support only uvw column for now
    if (all.shape().nelements() != 2) {
        stringstream ss;
        ss << __func__ << ": Column " << colName << " is " << all.shape().nelements() 
           << "-dimensional, expected 2-dimensional";
        throw(AipsError(ss.str()));
    }

    if (all.shape()(0) != 3) {
        stringstream ss;
        ss << __func__ << ": Column " << colName << " has cell length " << all.shape()(0) 
           << ", expected 3";
        throw(AipsError(ss.str()));
    }

    IPosition shape(2, 3, curNumRow_p);
    if (resize && array.shape() != shape) {
        dynamic_cast<Matrix<T> &>(array).resize(3, curNumRow_p);
    }

    Matrix<T> &m1 = dynamic_cast<Matrix<T> &>(array);
    Matrix<T> &m2 = dynamic_cast<Matrix<T> &>(all);
    for (unsigned j = 0; j < curNumRow_p; j++) {
        for (unsigned i = 0; i < 3; i++) {
            m1(i, j) = m2(i, rowIds_p(j));
        }
    }
    return;
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Double> &column, Array<Double> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Complex> &column, Array<Complex> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Bool> &column, const Slicer &slicer, Array<Bool> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, slicer, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Float> &column, const Slicer &slicer, Array<Float> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, slicer, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Complex> &column, const Slicer &slicer, Array<Complex> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, slicer, array, resize);
}

const Table
ROVisibilityIterator::attachTable() const
{
    return msIter_p.table();
}

void ROVisibilityIterator::slurp() const
{
    slurp_p = true;

    return;
}

VisibilityIterator::VisibilityIterator() {}

VisibilityIterator::VisibilityIterator(MeasurementSet &MS, 
				       const Block<Int>& sortColumns, 
				       Double timeInterval)
 :ROVisibilityIterator(MS, sortColumns, timeInterval)
{
}

VisibilityIterator::VisibilityIterator(MeasurementSet &MS, 
				       const Block<Int>& sortColumns, const Bool addDefSort,
				       Double timeInterval)
 :ROVisibilityIterator(MS, sortColumns, addDefSort, timeInterval)
{
}
VisibilityIterator::VisibilityIterator(Block<MeasurementSet>& mss, 
				       const Block<Int>& sortColumns, 
				       Double timeInterval)
:ROVisibilityIterator(mss, sortColumns, timeInterval)
{
}
VisibilityIterator::VisibilityIterator(Block<MeasurementSet>& mss, 
				       const Block<Int>& sortColumns, const Bool addDefSort,
				       Double timeInterval)
 :ROVisibilityIterator(mss, sortColumns, addDefSort, timeInterval)
{
}

VisibilityIterator::VisibilityIterator(const VisibilityIterator & other)
    : ROVisibilityIterator()
{
    operator=(other);
}

VisibilityIterator::~VisibilityIterator() 
{
    if (slurp_p && dirtyRowflags) {
        ScalarColumn<Bool> colAll(msIter_p.ms(), MS::columnName(MS::FLAG_ROW));
        colAll.putColumn(allRowflags);
    }
}

VisibilityIterator& 
VisibilityIterator::operator=(const VisibilityIterator& other)
{
    if (this!=&other) {
	ROVisibilityIterator::operator=(other);
        dirtyRowflags = other.dirtyRowflags;
	RWcolFlag.reference(other.RWcolFlag);
        RWcolFlagRow.reference(other.RWcolFlagRow);
	RWcolVis.reference(other.RWcolVis);
	RWcolFloatVis.reference(other.RWcolFloatVis);
	RWcolModelVis.reference(other.RWcolModelVis);
	RWcolCorrVis.reference(other.RWcolCorrVis);
	RWcolWeight.reference(other.RWcolWeight);
        RWcolWeightSpectrum.reference(other.RWcolWeightSpectrum);
	RWcolSigma.reference(other.RWcolSigma);
    }
    return *this;
}

VisibilityIterator & VisibilityIterator::operator++(int)
{
  if (!more_p) return *this;
  advance();
  return *this;
}

VisibilityIterator & VisibilityIterator::operator++()
{
  if (!more_p) return *this;
  advance();
  return *this;
}

void VisibilityIterator::attachColumns(const Table &t)
{
  ROVisibilityIterator::attachColumns(t);

  const ColumnDescSet& cds=t.tableDesc().columnDescSet();
  if (cds.isDefined(MS::columnName(MS::DATA))) {
    RWcolVis.attach(t, MS::columnName(MS::DATA));
  }
  if (cds.isDefined(MS::columnName(MS::FLOAT_DATA))) {
    floatDataFound_p=True;
    RWcolFloatVis.attach(t, MS::columnName(MS::FLOAT_DATA));
  } else {
    floatDataFound_p=False;
  }
  if (cds.isDefined("MODEL_DATA")) 
    RWcolModelVis.attach(t, "MODEL_DATA");
  if (cds.isDefined("CORRECTED_DATA")) 
    RWcolCorrVis.attach(t, "CORRECTED_DATA");
  RWcolWeight.attach(t, MS::columnName(MS::WEIGHT));
  if (cds.isDefined("WEIGHT_SPECTRUM"))
    RWcolWeightSpectrum.attach(t, "WEIGHT_SPECTRUM");
  RWcolSigma.attach(t, MS::columnName(MS::SIGMA));
  RWcolFlag.attach(t, MS::columnName(MS::FLAG));
  RWcolFlagRow.attach(t, MS::columnName(MS::FLAG_ROW));
}

void VisibilityIterator::setFlag(const Matrix<Bool>& flag)
{
  // use same value for all polarizations
  flagCube_p.resize(nPol_p,channelGroupSize_p,curNumRow_p);
  Bool deleteIt;
  Bool* p=flagCube_p.getStorage(deleteIt);
  const Bool* pflag=flag.getStorage(deleteIt);
  if (Int(flag.nrow())!=channelGroupSize_p) {
    throw(AipsError("VisIter::setFlag(flag) - inconsistent number of channels"));
  }
  
  for (uInt row=0; row<curNumRow_p; row++) {
    for (Int chn=0; chn<channelGroupSize_p; chn++) {
      for (Int pol=0; pol<nPol_p; pol++) {
	*p++=*pflag;
      }
      pflag++;
    }
  }
  if (useSlicer_p) putCol(RWcolFlag, slicer_p,flagCube_p);
  else putCol(RWcolFlag, flagCube_p);
}

void VisibilityIterator::setFlag(const Cube<Bool>& flags)
{
  if (useSlicer_p) putCol(RWcolFlag, slicer_p,flags);
  else putCol(RWcolFlag, flags);
}

void VisibilityIterator::setFlagRow(const Vector<Bool>& rowflags)
{
    putCol(RWcolFlagRow, rowflags, MS::columnName(MS::FLAG_ROW), allRowflags);
    dirtyRowflags = true;
}

void VisibilityIterator::setVis(const Matrix<CStokesVector> & vis,
				DataColumn whichOne)
{
  // two problems: 1. channel selection -> we can only write to reference
  // MS with 'processed' channels
  //               2. polarization: there could be 1, 2 or 4 in the
  // original data, predict() always gives us 4. We save what was there
  // originally.

  //  if (!preselected_p) {
  //    throw(AipsError("VisIter::setVis(vis) - cannot change original data"));
  //  }
  if (Int(vis.nrow())!=channelGroupSize_p) {
    throw(AipsError("VisIter::setVis(vis) - inconsistent number of channels"));
  }
  // we need to reform the vis matrix to a cube before we can use
  // putColumn to a Matrix column
  visCube_p.resize(nPol_p,channelGroupSize_p,curNumRow_p);
  Bool deleteIt;
  Complex* p=visCube_p.getStorage(deleteIt);
  for (uInt row=0; row<curNumRow_p; row++) {
    for (Int chn=0; chn<channelGroupSize_p; chn++) {
      const CStokesVector& v=vis(chn,row);
      switch (nPol_p) {
      case 4: *p++=v(0); *p++=v(1); *p++=v(2); *p++=v(3); break;     
      case 2: *p++=v(0); *p++=v(3); break;
      case 1: *p++=(v(0)+v(3))/2; break;
      }
    }
  }
  if (useSlicer_p) putDataColumn(whichOne,slicer_p,visCube_p);
  else putDataColumn(whichOne,visCube_p);
}

void VisibilityIterator::setVisAndFlag(const Cube<Complex>& vis,
				       const Cube<Bool>& flag,
				       DataColumn whichOne)
{
  if (velSelection_p) {
    setInterpolatedVisFlag(vis,flag);
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,visCube_p);
    else putDataColumn(whichOne,visCube_p);
    if (useSlicer_p) putCol(RWcolFlag, slicer_p,flagCube_p);
    else putCol(RWcolFlag, flagCube_p);
  } else {
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,vis);
    else putDataColumn(whichOne,vis);
    if (useSlicer_p) putCol(RWcolFlag, slicer_p,flag);
    else putCol(RWcolFlag, flag);
  }
}

void VisibilityIterator::setVis(const Cube<Complex>& vis, DataColumn whichOne)
{
  if (velSelection_p) {
    setInterpolatedVisFlag(vis,flagCube_p);
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,visCube_p);
    else putDataColumn(whichOne,visCube_p);
  } else {
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,vis);
    else putDataColumn(whichOne,vis);
  }
}

void VisibilityIterator::setWeight(const Vector<Float>& weight)
{
  // No polarization dependence for now
  Matrix<Float> polWeight;
  getCol(colWeight, polWeight);
  for (Int i=0; i<nPol_p; i++) {
    Vector<Float> r=polWeight.row(i);
    r=weight;
  }
  putCol(RWcolWeight, polWeight);
}

void VisibilityIterator::setWeightMat(const Matrix<Float>& weightMat)
{
  putCol(RWcolWeight, weightMat);
}

void VisibilityIterator::setWeightSpectrum(const Cube<Float>& weightSpectrum)
{
  if (!colWeightSpectrum.isNull()) {
    putCol(RWcolWeightSpectrum, weightSpectrum);
  }
}

void VisibilityIterator::setSigma(const Vector<Float>& sigma)
{
  Matrix<Float> sigmat;
  getCol(colSigma, sigmat);
  for (Int i=0; i < nPol_p; i++) {
    Vector<Float> r = sigmat.row(i);
    r = sigma;
  }
  putCol(RWcolSigma, sigmat);
}

void VisibilityIterator::setInterpolatedVisFlag(const Cube<Complex>& vis, 
						const Cube<Bool>& flag)
{
  // get the frequencies to interpolate to
  velSelection_p = False; 
  Vector<Double> freq; frequency(freq);
  velSelection_p = True;

  // now interpolate visibilities, using freq as the sample points
  // we should have two options: flagging output points that have
  // any flagged inputs or interpolating across flagged data.
  // Convert frequencies to float (removing offset to keep accuracy) 
  // so we can multiply them with Complex numbers to do the interpolation.
  Block<Float> xfreq(channelGroupSize_p),sfreq(nVelChan_p); 
  Int i;
  for (i=0; i<channelGroupSize_p; i++) xfreq[i]=freq(i)-freq(0);
  for (i=0; i<nVelChan_p; i++) sfreq[i]=selFreq_p(i)-freq(0);
  // set up the Functionals for the interpolation
  Cube<Complex> swapVis,intVis;
  swapyz(swapVis,vis);
  Cube<Bool> swapFlag,intFlag;
  swapyz(swapFlag,flag);
  InterpolateArray1D<Float,Complex>::InterpolationMethod method1=
    InterpolateArray1D<Float,Complex>::linear;
  if (vInterpolation_p=="nearest") {
    method1=InterpolateArray1D<Float,Complex>::nearestNeighbour;
  }
  InterpolateArray1D<Float,Complex>::
    interpolate(intVis,intFlag,xfreq,sfreq,swapVis,swapFlag,method1);
  swapyz(visCube_p,intVis);
  swapyz(flagCube_p,intFlag);
}



void VisibilityIterator::setInterpolatedWeight(const Matrix<Float>& wt)
{
  // get the frequencies to interpolate to
  velSelection_p = False; 
  Vector<Double> freq; frequency(freq);
  velSelection_p = True;

  // now interpolate weights, using freq as the sample points
  // we should have two options: flagging output points that have
  // any flagged inputs or interpolating across flagged data.
  // Convert frequencies to float (removing offset to keep accuracy) 
  // so we can multiply them with Complex numbers to do the interpolation.
  Block<Float> xfreq(channelGroupSize_p),sfreq(nVelChan_p); 
  Int i;
  for (i=0; i<channelGroupSize_p; i++) xfreq[i]=freq(i)-freq(0);
  for (i=0; i<nVelChan_p; i++) sfreq[i]=selFreq_p(i)-freq(0);
  // set up the Functionals for the interpolation
  Matrix<Float> twt,intWt;
  transpose(twt,wt);
  InterpolateArray1D<Float,Float>::InterpolationMethod method2=
    InterpolateArray1D<Float,Float>::linear;
  if (vInterpolation_p=="nearest") {
    method2= InterpolateArray1D<Float,Float>::nearestNeighbour;
  }
  InterpolateArray1D<Float,Float>::
    interpolate(intWt,xfreq,sfreq,twt,method2);
  transpose(imagingWeight_p,intWt);
}

void VisibilityIterator::putDataColumn(DataColumn whichOne,
				       const Slicer& slicer,
				       const Cube<Complex>& data)
{
  // Set the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.


  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      putCol(RWcolFloatVis, slicer,dataFloat);
    } else {
      putCol(RWcolVis, slicer,data);
    }
    break;
  case Corrected:
    putCol(RWcolCorrVis, slicer,data);
    break;
  case Model:
    putCol(RWcolModelVis, slicer,data);
    break;
  }
}  

void VisibilityIterator::putDataColumn(DataColumn whichOne,
				       const Cube<Complex>& data)
{
  // Set the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      putCol(RWcolFloatVis, dataFloat);
    } else {
      putCol(RWcolVis, data);
    }
    break;
  case Corrected:
    putCol(RWcolCorrVis, data);
    break;
  case Model:
    putCol(RWcolModelVis, data);
    break;
  }
}  

void VisibilityIterator::putColScalar(ScalarColumn<Bool> &column, const Vector<Bool> &array, const String &colName, Vector<Bool> &all)
{
    if (!slurp_p) {
        column.putColumnCells(selRows_p, array);
        return;
    }

    if (all.nelements() == 0) {
        ScalarColumn<Bool> colAll(msIter_p.ms(), colName);
        try {
            all = colAll.getColumn();
        }
        catch (...) {
            // If out of memory, do it the slow way
            column.putColumnCells(selRows_p, array);
            return;
        }
    }

    update_rowIds();

    for (unsigned i = 0; i < curNumRow_p; i++) {
        all(rowIds_p(i)) = array(i);
    }
    return;
}

void VisibilityIterator::putCol(ScalarColumn<Bool> &column, const Vector<Bool> &array, const String &colName, Vector<Bool> &all)
{
    putColScalar(column, array, colName, all);
}

void VisibilityIterator::putCol(ArrayColumn<Bool> &column, const Array<Bool> &array)
{
    column.putColumnCells(selRows_p, array);
}

void VisibilityIterator::putCol(ArrayColumn<Float> &column, const Array<Float> &array)
{
    column.putColumnCells(selRows_p, array);
}

void VisibilityIterator::putCol(ArrayColumn<Complex> &column, const Array<Complex> &array)
{
    column.putColumnCells(selRows_p, array);
}

void VisibilityIterator::putCol(ArrayColumn<Bool> &column, const Slicer &slicer, const Array<Bool> &array)
{
    column.putColumnCells(selRows_p, slicer, array);
}

void VisibilityIterator::putCol(ArrayColumn<Float> &column, const Slicer &slicer, const Array<Float> &array)
{
    column.putColumnCells(selRows_p, slicer, array);
}

void VisibilityIterator::putCol(ArrayColumn<Complex> &column, const Slicer &slicer, const Array<Complex> &array)
{
    column.putColumnCells(selRows_p, slicer, array);
}

void VisibilityIterator::slurp() const
{
    ROVisibilityIterator::slurp();

    dirtyRowflags = false;
    return;
}

} //# NAMESPACE CASA - END

