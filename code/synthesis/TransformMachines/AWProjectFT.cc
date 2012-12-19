// -*- C++ -*-
//# AWProjectFT.cc: Implementation of AWProjectFT class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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

#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/OS/HostInfo.h>
#include <casa/sstream.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/ImageInterface.h>

#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <synthesis/TransformMachines/AWProjectFT.h>
#include <synthesis/TransformMachines/AWConvFunc.h>
#include <synthesis/TransformMachines/CFStore2.h>
#include <synthesis/MeasurementComponents/ExpCache.h>
#include <synthesis/MeasurementComponents/CExp.h>
#include <synthesis/TransformMachines/AWVisResampler.h>
#include <synthesis/TransformMachines/VBStore.h>

#include <scimath/Mathematics/FFTServer.h>
#include <scimath/Mathematics/MathFunc.h>
#include <measures/Measures/MeasTable.h>
#include <casa/iostream.h>
#include <casa/OS/Timer.h>

#define CONVSIZE (1024*2)
#define OVERSAMPLING 2
#define USETABLES 0           // If equal to 1, use tabulated exp() and
			      // complex exp() functions.
#define MAXPOINTINGERROR 250.0 // Max. pointing error in arcsec used to
// determine the resolution of the
// tabulated exp() function.
#define DORES True


namespace casa { //# NAMESPACE CASA - BEGIN
  
#define NEED_UNDERSCORES
  extern "C" 
  {
    //
    // The Gridding Convolution Function (GCF) used by the underlying
    // gridder written in FORTRAN.
    //
    // The arguments must all be pointers and the value of the GCF at
    // the given (u,v) point is returned in the weight variable.  Making
    // this a function which returns a complex value (namely the weight)
    // has problems when called in FORTRAN - I (SB) don't understand
    // why.
    //
#if defined(NEED_UNDERSCORES)
#define nwcppeij nwcppeij_
#endif
    //
    //---------------------------------------------------------------
    //
    IlluminationConvFunc awEij;
    void awcppeij(Double *griduvw, Double *area,
		 Double *raoff1, Double *decoff1,
		 Double *raoff2, Double *decoff2, 
		 Int *doGrad,
		 Complex *weight,
		 Complex *dweight1,
		 Complex *dweight2,
		 Double *currentCFPA)
    {
      Complex w,d1,d2;
      awEij.getValue(griduvw, raoff1, raoff2, decoff1, decoff2,
		    area,doGrad,w,d1,d2,*currentCFPA);
      *weight   = w;
      *dweight1 = d1;
      *dweight2 = d2;
    }
  }
  //
  //---------------------------------------------------------------
  //
  AWProjectFT::AWProjectFT()
    : FTMachine(), padding_p(1.0), nWPlanes_p(1),
      imageCache(0), cachesize(0), tilesize(16),
      gridder(0), isTiled(False), arrayLattice(0), lattice(0), 
      maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
      pointingToImage(0), usezero_p(False),
      // convFunc_p(), convWeights_p(),
      epJ_p(),
      doPBCorrection(True), conjBeams_p(True),/*cfCache_p(cfcache),*/ paChangeDetector(),
      rotateAperture_p(True),
      Second("s"),Radian("rad"),Day("d"), pbNormalized_p(False), paNdxProcessed_p(),
      visResampler_p(), sensitivityPatternQualifier_p(-1),sensitivityPatternQualifierStr_p(""),
      rotatedConvFunc_p(),cfs2_p(), cfwts2_p(), runTime(0.0)
  {
    convSize=0;
    tangentSpecified_p=False;
    lastIndex_p=0;
    paChangeDetector.reset();
    pbLimit_p=5e-2;
    //
    // Get various parameters from the visibilities.  
    //
    doPointing=1; 

    maxConvSupport=-1;  
    //
    // Set up the Conv. Func. disk cache manager object.
    //
    // if (!cfCache_p.null()) delete &cfCache_p;
    // cfCache_p=cfcache;
    convSampling=OVERSAMPLING;
    convSize=CONVSIZE;
    Long hostRAM = (HostInfo::memoryTotal(true)*1024); // In bytes
    hostRAM = hostRAM/(sizeof(Float)*2); // In complex pixels
    if (cachesize > hostRAM) cachesize=hostRAM;
    sigma=1.0;
    canComputeResiduals_p=DORES;
    cfs2_p = new CFStore2;
    cfwts2_p = new CFStore2;
    pop_p->init();
    //    rotatedConvFunc_p.data=new Array<Complex>();    
  }
  //
  //---------------------------------------------------------------
  //
  AWProjectFT::AWProjectFT(Int nWPlanes, Long icachesize, 
			   CountedPtr<CFCache>& cfcache,
			   CountedPtr<ConvolutionFunction>& cf,
			   CountedPtr<VisibilityResamplerBase>& visResampler,
			   Bool applyPointingOffset,
			   Bool doPBCorr,
			   Int itilesize, 
			   Float pbLimit,
			   Bool usezero,
			   Bool conjBeams)
    : FTMachine(cfcache,cf), padding_p(1.0), nWPlanes_p(nWPlanes),
      imageCache(0), cachesize(icachesize), tilesize(itilesize),
      gridder(0), isTiled(False), arrayLattice(0), lattice(0), 
      maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
      pointingToImage(0), usezero_p(usezero),
      // convFunc_p(), convWeights_p(),
      epJ_p(),
      doPBCorrection(doPBCorr), conjBeams_p(conjBeams),/*cfCache_p(cfcache),*/ paChangeDetector(),
      rotateAperture_p(True),
      Second("s"),Radian("rad"),Day("d"), pbNormalized_p(False),
      visResampler_p(visResampler), sensitivityPatternQualifier_p(-1),sensitivityPatternQualifierStr_p(""),
      rotatedConvFunc_p(), runTime(0.0)
  {
    convSize=0;
    tangentSpecified_p=False;
    lastIndex_p=0;
    paChangeDetector.reset();
    pbLimit_p=pbLimit;
    //
    // Get various parameters from the visibilities.  
    //
    if (applyPointingOffset) doPointing=1; else doPointing=0;

    maxConvSupport=-1;  
    //
    // Set up the Conv. Func. disk cache manager object.
    //
    // if (!cfCache_p.null()) delete &cfCache_p;
    // cfCache_p=cfcache;
    convSampling=OVERSAMPLING;
    convSize=CONVSIZE;
    Long hostRAM = (HostInfo::memoryTotal(true)*1024); // In bytes
    hostRAM = hostRAM/(sizeof(Float)*2); // In complex pixels
    if (cachesize > hostRAM) cachesize=hostRAM;
    sigma=1.0;
    canComputeResiduals_p=DORES;
    cfs2_p = new CFStore2;
    cfwts2_p = new CFStore2;
    pop_p->init();
    //    rotatedConvFunc_p.data=new Array<Complex>();
  }
  //
  //---------------------------------------------------------------
  //
  AWProjectFT::AWProjectFT(const RecordInterface& stateRec)
    : FTMachine(),Second("s"),Radian("rad"),Day("d"),visResampler_p(), cfs2_p(), cfwts2_p()
  {
    LogIO log_l(LogOrigin("AWProjectFT", "AWProjectFT[R&D]"));
    //
    // Construct from the input state record
    //
    String error;
    
    if (!fromRecord(stateRec)) {
      log_l << "Failed to create " << name() << " object." << LogIO::EXCEPTION;
    };
    maxConvSupport=-1;
    convSampling=OVERSAMPLING;
    visResampler_p->init(useDoubleGrid_p);
    convSize=CONVSIZE;
    canComputeResiduals_p=DORES;
    cfs2_p = new CFStore2;
    cfwts2_p = new CFStore2;
    pop_p->init();
  }
  //
  //----------------------------------------------------------------------
  //
  AWProjectFT::AWProjectFT(const AWProjectFT& other):FTMachine()
  {
    operator=(other);
  }
  //
  //---------------------------------------------------------------
  //
  // This is nasty, we should use CountedPointers here.
  AWProjectFT::~AWProjectFT() 
  {
      if(imageCache) delete imageCache; imageCache=0;
      if(gridder) delete gridder; gridder=0;
  }
  //
  //---------------------------------------------------------------
  //
  AWProjectFT& AWProjectFT::operator=(const AWProjectFT& other)
  {
    if(this!=&other) 
      {
	//Do the base parameters
	FTMachine::operator=(other);

	
	padding_p=other.padding_p;
	
	nWPlanes_p=other.nWPlanes_p;
	imageCache=other.imageCache;
	cachesize=other.cachesize;
	tilesize=other.tilesize;
	cfRefFreq_p = other.cfRefFreq_p;
	if(other.gridder==0) gridder=0;
	else
	  {
	    uvScale.resize();
	    uvOffset.resize();
	    uvScale=other.uvScale;
	    uvOffset=other.uvOffset;
	    gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
							   uvScale, uvOffset,
							   "SF");
	  }

	isTiled=other.isTiled;
	lattice=0;
	arrayLattice=0;

	maxAbsData=other.maxAbsData;
	centerLoc=other.centerLoc;
	offsetLoc=other.offsetLoc;
	pointingToImage=other.pointingToImage;
	usezero_p=other.usezero_p;

	
	padding_p=other.padding_p;
	nWPlanes_p=other.nWPlanes_p;
	imageCache=other.imageCache;
	cachesize=other.cachesize;
	tilesize=other.tilesize;
	isTiled=other.isTiled;
	maxAbsData=other.maxAbsData;
	centerLoc=other.centerLoc;
	offsetLoc=other.offsetLoc;
	pointingToImage=other.pointingToImage;
	usezero_p=other.usezero_p;
	doPBCorrection = other.doPBCorrection;
	maxConvSupport= other.maxConvSupport;

	epJ_p=other.epJ_p;
	convSize=other.convSize;
	lastIndex_p=other.lastIndex_p;
	paChangeDetector=other.paChangeDetector;
	pbLimit_p=other.pbLimit_p;
	//
	// Get various parameters from the visibilities.  
	//
	doPointing=other.doPointing;

	maxConvSupport=other.maxConvSupport;
	//
	// Set up the Conv. Func. disk cache manager object.
	//
	cfCache_p=other.cfCache_p;
	convSampling=other.convSampling;
	convSize=other.convSize;
	cachesize=other.cachesize;
    
	currentCFPA=other.currentCFPA;
	lastPAUsedForWtImg = other.lastPAUsedForWtImg;
	avgPB_p = other.avgPB_p;
	avgPBSq_p = other.avgPBSq_p;
	convFuncCtor_p = other.convFuncCtor_p;
	pbNormalized_p = other.pbNormalized_p;
	sensitivityPatternQualifier_p = other.sensitivityPatternQualifier_p;
	sensitivityPatternQualifierStr_p = other.sensitivityPatternQualifierStr_p;
	visResampler_p=other.visResampler_p; // Copy the counted pointer
	//	visResampler_p=other.visResampler_p->clone();
	*visResampler_p = *other.visResampler_p; // Call the appropriate operator=()
	rotatedConvFunc_p = other.rotatedConvFunc_p;
	cfs2_p = other.cfs2_p;
	cfwts2_p = other.cfwts2_p;
	paNdxProcessed_p = other.paNdxProcessed_p;
	runTime= other.runTime;
	imRefFreq_p = other.imRefFreq_p;
	conjBeams_p = other.conjBeams_p;
      };
    return *this;
  };
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::init() 
  {
    LogIO log_l(LogOrigin("AWProjectFT", "init[R&D]"));

    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    
    if(image->shape().product()>cachesize) 
      isTiled=True;
    else 
      isTiled=False;
    
    sumWeight.resize(npol, nchan);
    sumCFWeight.resize(npol, nchan);
    
    wConvSize=max(1, nWPlanes_p);
    
    CoordinateSystem cs=image->coordinates();
    uvScale.resize(3);
    uvScale=0.0;
    uvScale(0)=Float(nx)*cs.increment()(0); 
    uvScale(1)=Float(ny)*cs.increment()(1); 
    uvScale(2)=Float(wConvSize)*abs(cs.increment()(0));
    
    Int index= cs.findCoordinate(Coordinate::SPECTRAL);
    SpectralCoordinate spCS = cs.spectralCoordinate(index);
    imRefFreq_p = spCS.referenceValue()(0);
    
    uvOffset.resize(3);
    uvOffset(0)=nx/2;
    uvOffset(1)=ny/2;
    uvOffset(2)=0;
    
    if(gridder) delete gridder; gridder=0;
    gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						   uvScale, uvOffset,
						   "SF");
    
    // Set up image cache needed for gridding. 
    if(imageCache) delete imageCache;   imageCache=0;
    
    // The tile size should be large enough that the
    // extended convolution function can fit easily
    if(isTiled) 
      {
	Float tileOverlap=0.5;
	tilesize=min(256,tilesize);
	IPosition tileShape=IPosition(4,tilesize,tilesize,npol,nchan);
	Vector<Float> tileOverlapVec(4);
	tileOverlapVec=0.0;
	tileOverlapVec(0)=tileOverlap;
	tileOverlapVec(1)=tileOverlap;
	if (sizeof(long) < 4)  // 32-bit machine
	  {
	    Int tmpCacheVal=static_cast<Int>(cachesize);
	    imageCache=new LatticeCache <Complex> (*image, tmpCacheVal, tileShape, 
						   tileOverlapVec,
						   (tileOverlap>0.0));
	  }
	else  // 64-bit machine
	  {
	    Long tmpCacheVal=cachesize;
	    imageCache=new LatticeCache <Complex> (*image, tmpCacheVal, tileShape, 
						   tileOverlapVec,
						   (tileOverlap>0.0));
	  }
      }
    
#if(USETABLES)
    Double StepSize;
    Int N=500000;
    StepSize = abs((((2*nx)/uvScale(0))/(sigma) + 
		    MAXPOINTINGERROR*1.745329E-02*(sigma)/3600.0))/N;
    if (!awEij.isReady())
      {
	log_l << "Making lookup table for exp function with a resolution of " 
	      << StepSize << " radians.  "
	      << "Memory used: " << sizeof(Float)*N/(1024.0*1024.0)<< " MB." 
	      << LogIO::NORMAL 
	      <<LogIO::POST;
	
	awEij.setSigma(sigma);
	awEij.initExpTable(N,StepSize);
	//    ExpTab.build(N,StepSize);
	
	log_l << "Making lookup table for complex exp function with a resolution of " 
	      << 2*M_PI/N << " radians.  "
	      << "Memory used: " << 2*sizeof(Float)*N/(1024.0*1024.0) << " MB." 
	      << LogIO::NORMAL
	      << LogIO::POST;
	awEij.initCExpTable(N);
	//    CExpTab.build(N);
      }
#endif
    //    vpSJ->reset();
    paChangeDetector.reset();
    makingPSF = False;
    
    //cerr << "Current runTime = " << runTime << endl;
  }
  //
  //---------------------------------------------------------------
  //
  MDirection::Convert AWProjectFT::makeCoordinateMachine(const VisBuffer& vb,
							 const MDirection::Types& From,
							 const MDirection::Types& To,
							 MEpoch& last)
  {
    LogIO log_l(LogOrigin("AWProjectFT","makeCoordinateMachine[R&D]"));
    Double time = getCurrentTimeStamp(vb);
    
    MEpoch epoch(Quantity(time,Second),MEpoch::TAI);
    //  epoch = MEpoch(Quantity(time,Second),MEpoch::TAI);
    //
    // ...now make an object to hold the observatory position info...
    //
    MPosition pos;
    String ObsName=vb.msColumns().observation().telescopeName()(vb.arrayId());
    
    if (!MeasTable::Observatory(pos,ObsName))
      log_l << "Observatory position for "+ ObsName + " not found"
	    << LogIO::EXCEPTION;
    //
    // ...now make a Frame object out of the observatory position and
    // time objects...
    //
    MeasFrame frame(epoch,pos);
    //
    // ...finally make the convert machine.
    //
    MDirection::Convert mac(MDirection::Ref(From,frame),
			    MDirection::Ref(To,frame));
    
    MEpoch::Convert toLAST = MEpoch::Convert(MEpoch::Ref(MEpoch::TAI,frame),
					     MEpoch::Ref(MEpoch::LAST,frame));
    last = toLAST(epoch);
    
    return mac;
  }
  //
  //---------------------------------------------------------------
  //
  int AWProjectFT::findPointingOffsets(const VisBuffer& vb, 
					Array<Float> &l_off,
					Array<Float> &m_off,
					Bool Evaluate)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "findPointingOffsets[R&D]"));
    Int NAnt = 0;
    MEpoch LAST;
    Double thisTime = getCurrentTimeStamp(vb);
    //    Array<Float> pointingOffsets = epJ->nearest(thisTime);
    if (epJ_p.null()) return 0;
    Array<Float> pointingOffsets; epJ_p->nearest(thisTime,pointingOffsets);
    NAnt=pointingOffsets.shape()(2);
    l_off.resize(IPosition(3,1,1,NAnt)); // Poln x NChan x NAnt 
    m_off.resize(IPosition(3,1,1,NAnt)); // Poln x NChan x NAnt 
    // Can't figure out how to do the damn slicing of [Pol,NChan,NAnt,1] array
    // into [Pol,NChan,NAnt] array
    //
    //    l_off = pointingOffsets(Slicer(IPosition(4,0,0,0,0),IPosition(4,1,1,NAnt+1,0)));
    //    m_off = pointingOffsets(Slicer(IPosition(4,1,0,0,0),IPosition(4,1,1,NAnt+1,0)));
    IPosition tndx(3,0,0,0), sndx(4,0,0,0,0);
    for(tndx(2)=0;tndx(2)<NAnt; tndx(2)++,sndx(2)++)
      //    for(Int j=0;j<NAnt;j++)
      {
	// l_off(IPosition(3,0,0,j)) = pointingOffsets(IPosition(4,0,0,j,0));
	// m_off(IPosition(3,0,0,j)) = pointingOffsets(IPosition(4,1,0,j,0));

	sndx(0)=0; l_off(tndx) = pointingOffsets(sndx);
	sndx(0)=2; m_off(tndx) = pointingOffsets(sndx);
      }
    return NAnt;
    if (!Evaluate) return NAnt;
    
    //  cout << "AzEl Offsets: " << pointingOffsets << endl;
    //
    // Make a Coordinate Conversion Machine to go from (Az,El) to
    // (HA,Dec).
    //
    MDirection::Convert toAzEl = makeCoordinateMachine(vb,MDirection::HADEC,
						       MDirection::AZEL,
						       LAST);
    MDirection::Convert toHADec = makeCoordinateMachine(vb,MDirection::AZEL,
							MDirection::HADEC,
							LAST);
    //
    // ...and now hope that it all works and works correctly!!!
    //
    Quantity dAz(0,Radian),dEl(0,Radian);
    //
    // An array of shape [2,1,1]!
    //
    Array<Double> phaseDir = vb.msColumns().field().phaseDir().getColumn();
    Double RA0   = phaseDir(IPosition(3,0,0,0));
    Double Dec0  = phaseDir(IPosition(3,1,0,0));
    //  
    // Compute reference (HA,Dec)
    //
    Double LST   = LAST.get(Day).getValue();
    Double SDec0 = sin(Dec0), CDec0=cos(Dec0);
    LST -= floor(LST); // Extract the fractional day
    LST *= 2*C::pi;// Convert to Raidan
    
    Double HA0;
    HA0 = LST - RA0;
    Quantity QHA0(HA0,Radian), QDEC0(Dec0,Radian);
    //
    // Convert reference (HA,Dec) to reference (Az,El)
    //
    MDirection PhaseCenter(QHA0, QDEC0,MDirection::Ref(MDirection::HADEC));
    MDirection AzEl0 = toAzEl(PhaseCenter);
    
    MDirection tmpHADec = toHADec(AzEl0);
    
    Double Az0_Rad = AzEl0.getAngle(Radian).getValue()(0);
    Double El0_Rad = AzEl0.getAngle(Radian).getValue()(1);
    
    //
    // Convert the antenna pointing offsets from (Az,El)-->(RA,Dec)-->(l,m) 
    //
    
    for(IPosition n(3,0,0,0);n(2)<=NAnt;n(2)++)
      {
	//
	// From (Az,El) -> (HA,Dec)
	//
	// Add (Az,El) offsets to the reference (Az,El)
	//
	dAz.setValue(l_off(n)+Az0_Rad);  dEl.setValue(m_off(n)+El0_Rad);
	//      dAz.setValue(0.0+Az0_Rad);  dEl.setValue(0.0+El0_Rad);
	MDirection AzEl(dAz,dEl,MDirection::Ref(MDirection::AZEL));
	//
	// Convert offsetted (Az,El) to (HA,Dec) and then to (RA,Dec)
	//
	MDirection HADec = toHADec(AzEl);
	Double HA,Dec,RA, dRA;
	HA  = HADec.getAngle(Radian).getValue()(0);
	Dec = HADec.getAngle(Radian).getValue()(1);
	RA  = LST - HA;
	dRA = RA - RA0;
	//
	// Convert offsetted (RA,Dec) -> (l,m)
	//
	l_off(n)  = sin(dRA)*cos(Dec);
	m_off(n) = sin(Dec)*CDec0-cos(Dec)*SDec0*cos(dRA);
	//      cout << "FindOff: " << n(2) << " " << l_offsets(n) << " " << m_offsets(n) << endl;
	
	//       cout << l_off(n) << " " << m_off(n) << " "
	// 	   << " " << HA << " " << Dec
	// 	   << " " << LST << " " << RA0 << " " << Dec0 
	// 	   << " " << RA << " " << Dec
	// 	   << endl;
      }
    
    return NAnt+1;
  }
  //
  //---------------------------------------------------------------
  //
  int AWProjectFT::findPointingOffsets(const VisBuffer& vb, 
					 Cube<Float>& pointingOffsets,
					Array<Float> &l_off,
					Array<Float> &m_off,
					Bool Evaluate)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "findPointingOffsets[R&D]"));
    Int NAnt = 0;
    Float tmp;
    // TBD: adapt the following to VisCal mechanism:
    MEpoch LAST;
    
    NAnt=pointingOffsets.shape()(2);
    l_off.resize(IPosition(3,2,1,NAnt));
    m_off.resize(IPosition(3,2,1,NAnt));
    IPosition ndx(3,0,0,0),ndx1(3,0,0,0);
    for(ndx(2)=0;ndx(2)<NAnt;ndx(2)++)
      {
	ndx1=ndx;
	ndx(0)=0;ndx1(0)=0;	tmp=l_off(ndx)  = pointingOffsets(ndx1);//Axis_0,Pol_0,Ant_i
	ndx(0)=1;ndx1(0)=1;	tmp=l_off(ndx)  = pointingOffsets(ndx1);//Axis_0,Pol_1,Ant_i
	ndx(0)=0;ndx1(0)=2;	tmp=m_off(ndx)  = pointingOffsets(ndx1);//Axis_1,Pol_0,Ant_i
	ndx(0)=1;ndx1(0)=3;	tmp=m_off(ndx)  = pointingOffsets(ndx1);//Axis_1,Pol_1,Ant_i
      }

//     l_off  = pointingOffsets(IPosition(3,0,0,0),IPosition(3,0,0,NAnt));
//     m_off = pointingOffsets(IPosition(3,1,0,0),IPosition(3,1,0,NAnt));
    /*
    IPosition shp(pointingOffsets.shape());
    IPosition shp1(l_off.shape()),shp2(m_off.shape());
    for(Int ii=0;ii<NAnt;ii++)
      {
	IPosition ndx(3,0,0,0);
	ndx(2)=ii;
	cout << "Pointing Offsets: " << ii << " " 
	     << l_off(ndx)*57.295*60.0 << " " 
	     << m_off(ndx)*57.295*60.0 << endl;
      }
    */
    return NAnt;
    if (!Evaluate) return NAnt;
    
    //
    // Make a Coordinate Conversion Machine to go from (Az,El) to
    // (HA,Dec).
    //
    MDirection::Convert toAzEl = makeCoordinateMachine(vb,MDirection::HADEC,
						       MDirection::AZEL,
						       LAST);
    MDirection::Convert toHADec = makeCoordinateMachine(vb,MDirection::AZEL,
							MDirection::HADEC,
							LAST);
    //
    // ...and now hope that it all works and works correctly!!!
    //
    Quantity dAz(0,Radian),dEl(0,Radian);
    //
    // An array of shape [2,1,1]!
    //
    Array<Double> phaseDir = vb.msColumns().field().phaseDir().getColumn();
    Double RA0   = phaseDir(IPosition(3,0,0,0));
    Double Dec0  = phaseDir(IPosition(3,1,0,0));
    //  
    // Compute reference (HA,Dec)
    //
    Double LST   = LAST.get(Day).getValue();
    Double SDec0 = sin(Dec0), CDec0=cos(Dec0);
    LST -= floor(LST); // Extract the fractional day
    LST *= 2*C::pi;// Convert to Raidan
    
    Double HA0;
    HA0 = LST - RA0;
    Quantity QHA0(HA0,Radian), QDEC0(Dec0,Radian);
    //
    // Convert reference (HA,Dec) to reference (Az,El)
    //
    MDirection PhaseCenter(QHA0, QDEC0,MDirection::Ref(MDirection::HADEC));
    MDirection AzEl0 = toAzEl(PhaseCenter);
    
    MDirection tmpHADec = toHADec(AzEl0);
    
    Double Az0_Rad = AzEl0.getAngle(Radian).getValue()(0);
    Double El0_Rad = AzEl0.getAngle(Radian).getValue()(1);
    
    //
    // Convert the antenna pointing offsets from (Az,El)-->(RA,Dec)-->(l,m) 
    //
    
    for(IPosition n(3,0,0,0);n(2)<=NAnt;n(2)++)
      {
	//
	// From (Az,El) -> (HA,Dec)
	//
	// Add (Az,El) offsets to the reference (Az,El)
	//
	dAz.setValue(l_off(n)+Az0_Rad);  dEl.setValue(m_off(n)+El0_Rad);
	//      dAz.setValue(0.0+Az0_Rad);  dEl.setValue(0.0+El0_Rad);
	MDirection AzEl(dAz,dEl,MDirection::Ref(MDirection::AZEL));
	//
	// Convert offsetted (Az,El) to (HA,Dec) and then to (RA,Dec)
	//
	MDirection HADec = toHADec(AzEl);
	Double HA,Dec,RA, dRA;
	HA  = HADec.getAngle(Radian).getValue()(0);
	Dec = HADec.getAngle(Radian).getValue()(1);
	RA  = LST - HA;
	dRA = RA - RA0;
	//
	// Convert offsetted (RA,Dec) -> (l,m)
	//
	l_off(n)  = sin(dRA)*cos(Dec);
	m_off(n) = sin(Dec)*CDec0-cos(Dec)*SDec0*cos(dRA);
	//      cout << "FindOff: " << n(2) << " " << l_offsets(n) << " " << m_offsets(n) << endl;
	
	//       cout << l_off(n) << " " << m_off(n) << " "
	// 	   << " " << HA << " " << Dec
	// 	   << " " << LST << " " << RA0 << " " << Dec0 
	// 	   << " " << RA << " " << Dec
	// 	   << endl;
      }
    
    return NAnt+1;
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::makeSensitivityImage(const VisBuffer& vb, 
					 const ImageInterface<Complex>& imageTemplate,
					 ImageInterface<Float>& sensitivityImage)
  {
    if (convFuncCtor_p->makeAverageResponse(vb, imageTemplate, sensitivityImage))
      cfCache_p->flush(sensitivityImage,sensitivityPatternQualifierStr_p); 
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::normalizeAvgPB(ImageInterface<Complex>& inImage,
				   ImageInterface<Float>& outImage)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "normalizeAvgPB[R&D]"));
    if (pbNormalized_p) return;
    IPosition inShape(inImage.shape()),ndx(4,0,0,0,0);
    Vector<Complex> peak(inShape(2));
    
    outImage.resize(inShape);
    outImage.setCoordinateInfo(inImage.coordinates());

    Bool isRefIn, isRefOut;
    Array<Complex> inBuf;
    Array<Float> outBuf;

    isRefIn  = inImage.get(inBuf);
    isRefOut = outImage.get(outBuf);
    log_l << "Normalizing the average PBs to unity"
	  << LogIO::NORMAL << LogIO::POST;
    //
    // Normalize each plane of the inImage separately to unity.
    //
    Complex inMax = max(inBuf);
    if (abs(inMax)-1.0 > 1E-3)
      {
	for(ndx(3)=0;ndx(3)<inShape(3);ndx(3)++)
	  for(ndx(2)=0;ndx(2)<inShape(2);ndx(2)++)
	    {
	      peak(ndx(2)) = 0;
	      for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
		  if (abs(inBuf(ndx)) > peak(ndx(2)))
		    peak(ndx(2)) = inBuf(ndx);
	      
	      for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
		  //		      avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
		  inBuf(ndx) /= peak(ndx(2));
	    }
	if (isRefIn) inImage.put(inBuf);
      }

    ndx=0;
    for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
      for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
	{
	  IPosition plane1(ndx);
	  plane1=ndx;
	  plane1(2)=1; // The other poln. plane
	  //	  avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
	  outBuf(ndx) = sqrt(real(inBuf(ndx) * inBuf(plane1)));
	}
    //
    // Rather convoluted way of copying Pol. plane-0 to Pol. plane-1!!!
    //
    for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
      for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
	{
	  IPosition plane1(ndx);
	  plane1=ndx;
	  plane1(2)=1; // The other poln. plane
	  outBuf(plane1) = real(outBuf(ndx));
	}

    pbNormalized_p = True;
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::normalizeAvgPB()
  {
    LogIO log_l(LogOrigin("AWProjectFT", "normalizeAvgPB[R&D]"));
    if (pbNormalized_p) return;
    Bool isRefF;
    Array<Float> avgPBBuf;
    isRefF=avgPB_p->get(avgPBBuf);
    //    Float pbMax = max(avgPBBuf);
      {
	pbPeaks.resize(avgPB_p->shape()(2),True);
	// if (makingPSF) pbPeaks = 1.0;
	// else pbPeaks /= (Float)noOfPASteps;
	pbPeaks = 1.0;
	log_l << "Normalizing the average PBs to " << 1.0
	      << LogIO::NORMAL << LogIO::POST;
	
	IPosition avgPBShape(avgPB_p->shape()),ndx(4,0,0,0,0);
	Vector<Float> peak(avgPBShape(2));
	
	
	Float pbMax = max(avgPBBuf);
	if (fabs(pbMax-1.0) > 1E-3)
	  {
	    //	    avgPBBuf = avgPBBuf/noOfPASteps;
	    for(ndx(3)=0;ndx(3)<avgPBShape(3);ndx(3)++)
	      for(ndx(2)=0;ndx(2)<avgPBShape(2);ndx(2)++)
		{
		  peak(ndx(2)) = 0;
		  for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
		    for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
		      if (abs(avgPBBuf(ndx)) > peak(ndx(2)))
			peak(ndx(2)) = avgPBBuf(ndx);
	      
		  for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
		    for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
		      //		      avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
		      avgPBBuf(ndx) /= peak(ndx(2));
		}
	    if (isRefF) avgPB_p->put(avgPBBuf);
	  }

	ndx=0;
	for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
	  for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
	    {
	      IPosition plane1(ndx);
	      plane1=ndx;
	      plane1(2)=1; // The other poln. plane
	      avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
	      //	      avgPBBuf(ndx) = (avgPBBuf(ndx) * avgPBBuf(plane1));
	    }
	for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
	  for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
	    {
	      IPosition plane1(ndx);
	      plane1=ndx;
	      plane1(2)=1; // The other poln. plane
	      avgPBBuf(plane1) = avgPBBuf(ndx);
	    }
      }
      pbNormalized_p = True;
  }
  //
  //---------------------------------------------------------------
  void AWProjectFT::makeCFPolMap(const VisBuffer& vb, const Vector<Int>& locCfStokes,
				 Vector<Int>& polM)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "findPointingOffsets[R&D]"));
    Vector<Int> msStokes = vb.corrType();
    Int nPol = msStokes.nelements();
    polM.resize(polMap.shape());
    polM = -1;

    for(Int i=0;i<nPol;i++)
      for(uInt j=0;j<locCfStokes.nelements();j++)
	if (locCfStokes(j) == msStokes(i))
	    {polM(i) = j;break;}
  }
  //
  //---------------------------------------------------------------
  //
  // Given a polMap (mapping of which Visibility polarization is
  // gridded onto which grid plane), make a map of the conjugate
  // planes of the grid E.g, for Stokes-I and -V imaging, the two
  // planes of the uv-grid are [LL,RR].  For input VisBuffer
  // visibilites in order [RR,RL,LR,LL], polMap = [1,-1,-1,0].  The
  // conjugate map will be [0,-1,-1,1].
  //
  void AWProjectFT::makeConjPolMap(const VisBuffer& vb, 
				     const Vector<Int> cfPolMap, 
				     Vector<Int>& conjPolMap)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "makConjPolMap[R&D]"));
    //
    // All the Natak (Drama) below with slicers etc. is to extract the
    // Poln. info. for the first IF only (not much "information
    // hiding" for the code to slice arrays in a general fashion).
    //
    // Extract the shape of the array to be sliced.
    //
    Array<Int> stokesForAllIFs = vb.msColumns().polarization().corrType().getColumn();
    IPosition stokesShape(stokesForAllIFs.shape());
    IPosition firstIFStart(stokesShape),firstIFLength(stokesShape);
    //
    // Set up the start and length IPositions to extract only the
    // first column of the array.  The following is required since the
    // array could have only one column as well.
    //
    firstIFStart(0)=0;firstIFLength(0)=stokesShape(0);
    for(uInt i=1;i<stokesShape.nelements();i++) {firstIFStart(i)=0;firstIFLength(i)=1;}
    //
    // Construct the slicer and produce the slice.  .nonDegenerate
    // required to ensure the result of slice is a pure vector.
    //
    Vector<Int> visStokes = stokesForAllIFs(Slicer(firstIFStart,firstIFLength)).nonDegenerate();

    conjPolMap = cfPolMap;
    
    Int i,j,N = cfPolMap.nelements();
    for(i=0;i<N;i++)
      if (cfPolMap[i] > -1)
	if      (visStokes[i] == Stokes::RR) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::LL) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
	else if (visStokes[i] == Stokes::LL) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::RR) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
	else if (visStokes[i] == Stokes::LR) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::RL) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
	else if (visStokes[i] == Stokes::RL) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::LR) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
  }
  //
  //---------------------------------------------------------------
  // Convert the CFs in the supplied CFStore to
  // A(nu)<Convolution>A(nu_*)
  //
  void AWProjectFT::makeWBCFWt(CFStore2& cfs, const Double imRefFreq)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "makeWBCFWt[R&D]"));
    log_l << "Converting WTCFs to wide-band versions" << LogIO::POST;

    Vector<Int> ant1List, ant2List;
    Vector<Quantity> paList;
    ant1List = cfs.getAnt1List();
    ant2List = cfs.getAnt2List();
    paList   = cfs.getPAList();

    if (paNdxProcessed_p.nelements() == 0) {paNdxProcessed_p.resize(1); paNdxProcessed_p[0]=False;}
    CountedPtr<CFBuffer> cfb_l, cfb_clone;
    Quantity dPA(360.0,"deg");
    for (uInt pa=0;pa<paList.nelements();pa++)
      for (uint a1=0;a1<ant1List.nelements(); a1++)
	for (uint a2=0;a2<ant2List.nelements(); a2++)
	  {
	    if (paNdxProcessed_p.nelements() < pa) {paNdxProcessed_p.resize(pa+1,True); paNdxProcessed_p[pa]=False;}
	    if (!paNdxProcessed_p[pa])
	      {
		paNdxProcessed_p[pa]=True;
		Vector<Double> wVals, fVals; PolMapType mVals, mNdx, conjMVals, conjMNdx;
		Double fIncr, wIncr;
		cfb_l = cfs.getCFBuffer(paList[pa], dPA, ant1List(a1), ant2List(a2));
		cfb_l->getCoordList(fVals,wVals,mNdx, mVals, conjMNdx, conjMVals, fIncr, wIncr);
		
		// cerr << paList << " " << endl
		//      << wVals << " " << endl
		//      << fVals << " " << endl
		//      << conjMVals << " " << endl;
		
		cfb_clone=cfb_l->clone();
		cfb_clone->getCoordList(fVals,wVals,mNdx, mVals, conjMNdx, conjMVals, fIncr, wIncr);
		// cerr << "Clone: " 
		//      << paList << " " << endl
		//      << wVals << " " << endl
		//      << fVals << " " << endl
		//      << conjMVals << " " << endl;

		CountedPtr<Array<Complex> > cfc_l, conjCFC_l, result_l;
		//
		// Damn!  Convolver does not work for complex convolution!
		//		Convolver<Complex> convolver;

		for (Int nw=0;nw<(Int)wVals.nelements(); nw++)
		  for (Int nf=0;nf<(Int)fVals.nelements(); nf++)
		    for (Int ipol=0;ipol<(Int)conjMVals.nelements();ipol++)
		      for (Int mRow=0;mRow<(Int)conjMVals[ipol].nelements(); mRow++)
			{
			  Double f, cf;
			  f=fVals[nf];
			  cf=sqrt(2*imRefFreq*imRefFreq - f*f);
			  Int conjNF = cfb_l->nearestFreqNdx(cf);
			  // cerr << "###PTRs: " 
			  //      << &(*(cfb_l->getCFCellPtr(nf,nw,conjMNdx[ipol][mRow]))) << " "
			  //      << &(*(cfb_clone->getCFCellPtr(nf,nw,conjMNdx[ipol][mRow]))) << " "
			  //      << &(*(cfb_clone->getCFCellPtr(conjNF,nw,conjMNdx[ipol][mRow])))
			  //      << endl;
			  result_l  = cfb_l->getCFCellPtr(nf,nw,conjMNdx[ipol][mRow])->getStorage();
			  cfc_l     = cfb_clone->getCFCellPtr(nf,nw,conjMNdx[ipol][mRow])->getStorage();
			  conjCFC_l = cfb_clone->getCFCellPtr(conjNF,nw,conjMNdx[ipol][mRow])->getStorage();

			  // convolver.setPsf(*cfc_l);
			  // convolver.linearConv(*result_l, *conjCFC_l);
			  SynthesisUtils::libreConvolver(*result_l,*conjCFC_l);


			  // {
			  //   CountedPtr<CFCell> cfc_t, conjCFC_t;
			  //   cfc_t = cfb_l->getCFCellPtr(nf,nw,conjMNdx[ipol][mRow]);
			  //   conjCFC_t = cfb_l->getCFCellPtr(conjNF,nw,conjMNdx[ipol][mRow]);
			  //   cout << "-------------------------------" << " " << f << " " << cf << " " 
			  // 	 << nf << " " << nw << " " << conjMNdx[ipol][mRow] << " " 
			  // 	 << conjNF << " " << nw << " " << conjMNdx[ipol][mRow] << " " 
			  // 	 <<endl;
			  //   cfc_t->show("CFC: ",cout);
			  //   conjCFC_t->show("conjCFC: ",cout);
			  //   cout << "-------------------------------" << endl;
			  // }
			}
	      }
	  }
  }
  //
  // Locate a convlution function.  It will be either in the cache
  // (mem. or disk cache) or will be computed and cached for possible
  // later use.
  //
  void AWProjectFT::findConvFunction(const ImageInterface<Complex>& image,
				     const VisBuffer& vb)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "findConvFunction[R&D]"));
    if (!paChangeDetector.changed(vb,0)) return;
    Int cfSource=CFDefs::NOTCACHED;
    CoordinateSystem ftcoords;
    // Think of a generic call to get the key-values.  And a
    // overloadable method (or an externally supplied one?) to convert
    // the values to key-ids.  That will ensure that AWProjectFT
    // remains the A-Projection algorithm implementation configurable
    // by the behaviour of the supplied objects.
    Float pa=getVBPA(vb);
    //UUU// ok();
    visResampler_p->setMaps(chanMap, polMap); //UUU Added here.
    visResampler_p->setFreqMaps(expandedSpwFreqSel_p,expandedSpwConjFreqSel_p);

    lastPAUsedForWtImg = currentCFPA = pa;

    Vector<Double> pointingOffset(convFuncCtor_p->findPointingOffset(image,vb));
    cfSource = visResampler_p->makeVBRow2CFMap(*cfs2_p,*convFuncCtor_p, vb,
					       paChangeDetector.getParAngleTolerance(),
					       chanMap,polMap,pointingOffset);
    if (cfSource == CFDefs::NOTCACHED)
      {
	PolMapType polMat, polIndexMat, conjPolMat, conjPolIndexMat;
	Vector<Int> visPolMap(vb.corrType());
	polMat = pop_p->makePolMat(visPolMap,polMap);
	polIndexMat = pop_p->makePol2CFMat(visPolMap,polMap);

	// polMat = pop_p->makePolMat(visPolMap,polMap);
	// polIndexMat = pop_p->makePol2CFMat(visPolMap,polMap);

	conjPolMat = pop_p->makeConjPolMat(visPolMap,polMap);
	conjPolIndexMat = pop_p->makeConjPol2CFMat(visPolMap,polMap);
	
	convFuncCtor_p->setPolMap(polMap);
	convFuncCtor_p->setSpwSelection(spwChanSelFlag_p);
	convFuncCtor_p->setSpwFreqSelection(spwFreqSel_p);

	cerr << "Freq. selection: " << expandedSpwFreqSel_p << endl << expandedSpwConjFreqSel_p << endl;

	convFuncCtor_p->makeConvFunction(image,vb,wConvSize, 
					 pop_p, pa, uvScale, uvOffset,
					 *cfs2_p, *cfwts2_p);
	//	log_l << "Converting WTCFs to wide-band versions" << LogIO::POST;
	//	makeWBCFWt(*cfwts2_p, imRefFreq_p);

	// cfSource = visResampler_p->makeVBRow2CFMap(*cfs2_p,*convFuncCtor_p, vb,
	// 					   paChangeDetector.getParAngleTolerance(),
	// 					   chanMap,polMap);
      }
    //
    // Load the average PB (sensitivity pattern) from the cache.  If
    // not found in the cache, make one and cache it.
    //
    if (cfCache_p->loadAvgPB(avgPB_p,sensitivityPatternQualifierStr_p) == CFDefs::NOTCACHED)
	makeSensitivityImage(vb,image,*avgPB_p);

    verifyShapes(avgPB_p->shape(), image.shape());

    if (paChangeDetector.changed(vb,0)) paChangeDetector.update(vb,0);
    //
    // Write some useful info. to the logger.
    //
    if (cfSource != CFDefs::MEMCACHE)
      {
	//
	// Compute the aggregate memory used by the cached convolution
	// functions.
	//
	// Int maxMemoryMB=HostInfo::memoryTotal(true)/1024;
	// String unit(" KB");
	// Float memoryKB=0;
	// memoryKB=(Float)cfCache_p->size();
	
	// memoryKB = Int(memoryKB/1024.0+0.5);
	// if (memoryKB > 1024) {memoryKB /=1024; unit=" MB";}
	
	// log_l << "Memory used in gridding functions = "
	//       << (Int)(memoryKB+0.5) << unit << " out of a maximum of "
	//       << maxMemoryMB << " MB" << LogIO::POST;
	//
	// Show the list of support sizes along the w-axis for the current PA.
	//

	{	
	  // log_l << "Convolution support:" <<endl;
	  // IPosition cfsShape = cfs2_p->getStorage().shape();
	  // for (Int ib=0;ib<cfsShape(0); ib++)
	  //   for(int it=0;it<cfsShape(1); it++)
	  //     {
	  // 	IPosition cfbShape = cfs2_p->getStorage()(ib,it)->getStorage().shape();
	  // 	for(int ip=0;ip<cfbShape(2); ip++)
	  // 	  for(Int ich=0;ich<cfbShape(0);ich++)
	  // 	    {
	  // 	      for(Int iw=0;iw<cfbShape(1); iw++)
	  // 		{		      
	  // 		  log_l << "   CFB[" << ib << "," << it << "] " 
	  // 			<< "CFC[" << ich << "," << iw << "," << ip << "]: " 
	  // 			<< cfs2_p->getStorage()(ib,it)->getStorage()(ich,iw,ip)->xSupport_p
	  // 			<< " pixels in Fourier plane" << LogIO::POST;
	  // 		}
	  // 	    }
	  //     }

	  cfs2_p->makePersistent(cfCache_p->getCacheDir().c_str());
	  cfwts2_p->makePersistent(cfCache_p->getCacheDir().c_str(),"WT");
	  Double memUsed=cfs2_p->memUsage();
	  String unit(" KB");
	  memUsed = (Int)(memUsed/1024.0+0.5);
	  if (memUsed > 1024) {memUsed /=1024; unit=" MB";}
	  log_l << "Convolution function memory footprint:" 
		<< (Int)(memUsed) << unit << " out of a maximum of "
		<< HostInfo::memoryTotal(true)/1024 << " MB" << LogIO::POST;
	  
	}
      }
    // cfs2_p->makePersistent("test.cf");
    // cfwts2_p->makePersistent("test.wtcf");
  }
  //
  //------------------------------------------------------------------------------
  // Vectorized initializeToVis.  See design related comments in the
  // .h file.
  //
  //
  // The functional goals here are:
  //
  //  1. If doPBCorrection==True (i.e. the input sky-image is flat-sky
  //  image), divide the image by the PB
  //
  //  2. Convert from Stokes to Feed (correlation) frame
  //
  //  3. FFT the image to produce gridded vis.
  //
  //  4. And since the same image buffer is used to accumulate the
  //  model, multiply the sky-image back with PB
  //
  // The call to non-vectorized version of initializeToVis() which is
  // pure virtual and hence the local version is called, is only to do
  // the FFT.  FFT is *NOT* in place.
  //
  // These operations effecitvely maintains the model image as PB*Sky
  // and supplies flat-sky model for prediction.  This can probably be
  // achieve with fewer operations and same memory buffers....but that
  // for later (SB)
  void AWProjectFT::initializeToVis(Block<CountedPtr<ImageInterface<Complex> > > & compImageVec,
				    PtrBlock<SubImage<Float> *> & modelImageVec, 
				    PtrBlock<SubImage<Float> *>& weightImageVec, 
				    PtrBlock<SubImage<Float> *>& /*fluxScaleVec*/, 
				    Block<Matrix<Float> >& weightsVec,
				    const VisBuffer& vb)
  {
    LogIO log_p(LogOrigin("AWProjectFT","initToVis"));
    //
    // Setting the image below is crucial since init() and
    // initMaps(vb) below expect this to be set.
    //
    image=&(*compImageVec[0]);

    log_p << "Total flux in model image (before avgPB normalization): " 
	  << sum((*(modelImageVec[0])).get()) << LogIO::POST;
    if(doPBCorrection) 
      {
	// Make the sensitivity Image if applicable
	init();
	initMaps(vb);
	findConvFunction(*(compImageVec[0]), vb); // Pure virtual -- call local version

	// Get the sensitivity Image
	Matrix<Float> tempWts;
	tempWts.resize();
	getWeightImage(*(weightImageVec[0]), tempWts);  // Pure virtual -- call local version

	// Normalize the model image by the sensitivity image only.
	// No local implementation -- call FTMachine version

	// Divide by avgPB
	//
	normalizeImage( *(modelImageVec[0]) , weightsVec[0],  *(weightImageVec[0]), 
			False, (Float)pbLimit_p, (Int)1);


	//
	// Divide by sqrt(avgPB)
	//
	// normalizeImage( *(modelImageVec[0]) , weightsVec[0],  *(weightImageVec[0]), 
	// 		False, (Float)pbLimit_p, (Int)4);
      }
    log_p << "Total flux in model image (after avgPB normalization): " 
	  << sum((*(modelImageVec[0])).get()) << LogIO::POST;
    
    // Convert from Stokes planes to Correlation planes
    // No local implementation -- call FTMachine version
    stokesToCorrelation(*(modelImageVec[0]), *(compImageVec[0]));

    // Call initializeToVis
    initializeToVis(*(compImageVec[0]), vb); // Pure virtual
    
    // Multiply the flat-sky model by the PB.
    // No local implementation -- call FTMachine version
    if(doPBCorrection)
      // Multiply by avgPB
      normalizeImage( *(modelImageVec[0]) , weightsVec[0], *(weightImageVec[0]) , False, (Float)pbLimit_p, (Int)3);
      
      // Multiply by sqrt(avgPB)
      //normalizeImage( *(modelImageVec[0]) , weightsVec[0], *(weightImageVec[0]) , False, (Float)pbLimit_p, (Int)5);
  }
  //
  //------------------------------------------------------------------------------
  //
  void AWProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
				    const VisBuffer& vb)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "initializeToVis[R&D]"));
    image=&iimage;
    
    ok();
    
    init();
    makingPSF = False;
    initMaps(vb);
    //visResampler_p->setMaps(chanMap, polMap);
    
    findConvFunction(*image, vb);
    //UUU    if (!cfCache_p->avgPBReady() && doPBCorrection)
    //UUU  log_l << "Sensitivity pattern not found." << LogIO::EXCEPTION;
    //UUU//    if (!cfCache_p->avgPBReady(sensitivityPatternQualifierStr_p) && doPBCorrection)
    //UUU//  log_l << "Sensitivity pattern for term " << sensitivityPatternQualifierStr_p << " not found." << LogIO::EXCEPTION;

    //  
    // Initialize the maps for polarization and channel. These maps
    // translate visibility indices into image indices
    //

    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    
    if(image->shape().product()>cachesize) isTiled=True;
    else isTiled=False;
    //
    // If we are memory-based then read the image in and create an
    // ArrayLattice otherwise just use the PagedImage
    //

    isTiled=False;

    if(isTiled){
    	lattice=CountedPtr<Lattice<Complex> > (image, False);
    }
    else 
      {
	IPosition gridShape(4, nx, ny, npol, nchan);
	griddedData.resize(gridShape);
	griddedData=Complex(0.0);
	
	IPosition stride(4, 1);
	IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
		      (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
	IPosition trc(blc+image->shape()-stride);
	
	IPosition start(4, 0);
	griddedData(blc, trc) = image->getSlice(start, image->shape());
	
	arrayLattice = new ArrayLattice<Complex>(griddedData);
	lattice=arrayLattice;
      }

    //AlwaysAssert(lattice, AipsError);
    
    log_l << LogIO::DEBUGGING << "Starting FFT of image" << LogIO::POST;
    
    Vector<Float> sincConv(nx);
    Float centerX=nx/2;
    for (Int ix=0;ix<nx;ix++) 
      {
	Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
	if(ix==centerX) sincConv(ix)=1.0;
	else            sincConv(ix)=sin(x)/x;
      }
    
    //    Vector<Complex> correction(nx);
    //
    // Do the Grid-correction
    //
    //    if(0) //UUU
    if(cfCache_p->avgPBReady()) //SB
    {
      //      normalizeAvgPB();
      
      IPosition cursorShape(4, nx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
	  
      verifyShapes(avgPB_p->shape(), image->shape());
      Array<Float> avgBuf; avgPB_p->get(avgBuf);
      if (max(avgBuf) < 1e-04)
	log_l << "Normalization by PB requested but either PB not "
	      <<"found in the cache or is ill-formed."
	      << LogIO::WARN << LogIO::POST;
	  

      LatticeStepper lpb(avgPB_p->shape(),cursorShape,axisPath);
      LatticeIterator<Float> lipb(*avgPB_p, lpb);

      Vector<Complex> griddedVis;
      //
      // Grid correct in anticipation of the convolution by the
      // convFunc.  Each polarization plane is corrected by the
      // appropraite primary beam.
      //
      for(lix.reset(),lipb.reset();!lix.atEnd();lix++,lipb++) 
	{
	  Int iy=lix.position()(1);
	  //	  gridder->correctX1D(correction,iy);
	  griddedVis = lix.rwVectorCursor();
	  
	  Vector<Float> PBCorrection(lipb.rwVectorCursor().shape());
	  PBCorrection = lipb.rwVectorCursor();
	  for(int ix=0;ix<nx;ix++) 
	    {
	      if (doPBCorrection)
		{
		  PBCorrection(ix) = pbFunc(PBCorrection(ix),pbLimit_p)*(sincConv(ix)*sincConv(iy));
		  lix.rwVectorCursor()(ix) /= (PBCorrection(ix));
		}
	      else 
		lix.rwVectorCursor()(ix) /= (1.0/(sincConv(ix)*sincConv(iy)));
	    }
	}
    }
    //
    // Now do the FFT2D in place
    //

    LatticeFFT::cfft2d(*lattice);

    log_l << LogIO::DEBUGGING << "Finished FFT" << LogIO::POST;
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
				     const VisBuffer& vb,
				     Array<Complex>& griddedVis,
				     Vector<Double>& uvscale)
  {
    initializeToVis(iimage, vb);
    griddedVis.assign(griddedData); //using the copy for storage
    uvscale.assign(uvScale);
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::finalizeToVis()
  {
    LogIO log_l(LogOrigin("AWProjectFT", "finalizeToVis[R&D]"));
    
    //cerr << "Run time = " << runTime << endl;
    //runTime=0.0;

    if(isTiled) 
      {
	AlwaysAssert(imageCache, AipsError);
	AlwaysAssert(image, AipsError);
	ostringstream o;
	imageCache->flush();
	imageCache->showCacheStatistics(o);
	log_l << o.str() << LogIO::POST;
      }
    if(pointingToImage) delete pointingToImage; pointingToImage=0;
  }
  //
  //---------------------------------------------------------------
  //
  // Initialize the FFT to the Sky. Here we have to setup and
  // initialize the grid.
  //
  void AWProjectFT::initializeToSky(ImageInterface<Complex>& iimage,
				     Matrix<Float>& weight,
				     const VisBuffer& vb)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "initializeToSky[R&D]"));
    
    // image always points to the image
    image=&iimage;
    
    init();
    initMaps(vb);
    visResampler_p->setMaps(chanMap, polMap);
    visResampler_p->setFreqMaps(expandedSpwFreqSel_p,expandedSpwConjFreqSel_p);
    
    // Initialize the maps for polarization and channel. These maps
    // translate visibility indices into image indices
    
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    
    if(image->shape().product()>cachesize) isTiled=True;
    else                                   isTiled=False;
    
    sumWeight=0.0;
    sumCFWeight = 0.0;
    weight.resize(sumWeight.shape());
    weight=0.0;
    //
    // Initialize for in memory or to disk gridding. lattice will
    // point to the appropriate Lattice, either the ArrayLattice for
    // in memory gridding or to the image for to disk gridding.
    //
    if(isTiled) 
      {
	imageCache->flush();
	image->set(Complex(0.0));
	lattice=CountedPtr<Lattice<Complex> > (image, False);
      }
    else 
      {
	IPosition gridShape(4, nx, ny, npol, nchan);
	griddedData.resize(gridShape);
	griddedData=Complex(0.0);
	arrayLattice = new ArrayLattice<Complex>(griddedData);
	lattice=arrayLattice;
	// if(useDoubleGrid_p) 
	//   {
	//     griddedData.resize();
	//     griddedData2.resize(gridShape);
	//     griddedData2=DComplex(0.0);
	//   }
      }
    // if(useDoubleGrid_p) 
    //   visResampler_p->initializeToSky(griddedData2, sumWeight);
    // else
      visResampler_p->initializeToSky(griddedData, sumWeight);
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::finalizeToSky()
  {
    //
    // Now we flush the cache and report statistics For memory based,
    // we don't write anything out yet.
    //
    LogIO log_l(LogOrigin("AWProjectFT", "findPointingOffsets[R&D]"));

    if(isTiled) 
      {
	AlwaysAssert(image, AipsError);
	AlwaysAssert(imageCache, AipsError);
	imageCache->flush();
	ostringstream o;
	imageCache->showCacheStatistics(o);
	log_l << o.str() << LogIO::POST;
      }
    if(pointingToImage) delete pointingToImage; pointingToImage=0;

    paChangeDetector.reset();
    cfCache_p->flush();
    // if(useDoubleGrid_p) 
    //   visResampler_p->finalizeToSky(griddedData2, sumWeight);
    // else
      visResampler_p->finalizeToSky(griddedData, sumWeight);
  }
  //
  //---------------------------------------------------------------
  //
  Array<Complex>* AWProjectFT::getDataPointer(const IPosition& centerLoc2D,
					       Bool readonly) 
  {
    Array<Complex>* result;
    // Is tiled: get tiles and set up offsets
    centerLoc(0)=centerLoc2D(0);
    centerLoc(1)=centerLoc2D(1);
    result=&imageCache->tile(offsetLoc, centerLoc, readonly);
    gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
    return result;
  }
  
  // The following file has the runFORTRAN* stuff. Moving it to a
  // separate file to reduce clutter and ultimately delete it.
#include "AWProjectFT.FORTRANSTUFF"
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::put(const VisBuffer& vb, Int row, Bool dopsf,
			FTMachine::Type type)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "put[R&D]"));
    // Take care of translation of Bools to Integer
    makingPSF=dopsf;
    
    findConvFunction(*image, vb);
    Nant_p     = vb.msColumns().antenna().nrow();

    const Matrix<Float> *imagingweight;
    imagingweight=&(vb.imagingWeight());

    Cube<Complex> data;
    //Fortran gridder need the flag as ints 
    Cube<Int> flags;
    Matrix<Float> elWeight;
    interpolateFrequencyTogrid(vb, *imagingweight,data, flags, elWeight, type);

    Int NAnt;
    if (doPointing) NAnt = findPointingOffsets(vb,l_offsets,m_offsets,True);
    
    //
    // If row is -1 then we pass through all rows
    //
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
      } 
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
      }
    //    
    // Get the uvws in a form that Fortran can use and do that
    // necessary phase rotation. On a Pentium Pro 200 MHz when null,
    // this step takes about 50us per uvw point. This is just barely
    // noticeable for Stokes I continuum and irrelevant for other
    // cases.
    //
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING to correct for an image inversion problem
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // // This is the convention for dphase
    // dphase*=-1.0;

    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);  
    
    //Here we redo the match or use previous match
    
    //Channel matching for the actual spectral window of buffer
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }

    VBStore vbs;
    setupVBStore(vbs,vb, elWeight,data,uvw,flags, dphase);

    resampleDataToGrid(griddedData, vbs, vb, dopsf);//, *imagingweight, *data, uvw,flags,dphase,dopsf);
    
  //Double or single precision gridding.
  // if(useDoubleGrid_p) 
  //   visResampler_p->DataToGrid(griddedData2, vbs, sumWeight, dopsf);
  // else
  //    visResampler_p->DataToGrid(griddedData, vbs, sumWeight, dopsf); 


  }
  //
  //-------------------------------------------------------------------------
  // Gridding
  void AWProjectFT::resampleDataToGrid(Array<Complex>& griddedData_l, VBStore& vbs, 
				       const VisBuffer& /*vb*/, Bool& dopsf)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "resampleDataToGrid[R&D]"));
    visResampler_p->DataToGrid(griddedData_l, vbs, sumWeight, dopsf); 
  }
  //
  //-------------------------------------------------------------------------
  // Gridding
  void AWProjectFT::resampleDataToGrid(Array<DComplex>& griddedData_l, VBStore& vbs, 
				       const VisBuffer& /*vb*/, Bool& dopsf)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "resampleDataToGridD[R&D]"));
    visResampler_p->DataToGrid(griddedData_l, vbs, sumWeight, dopsf); 
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::get(VisBuffer& vb, Int row)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "get[R&D]"));
    // If row is -1 then we pass through all rows

    //------COMMON FROM HERE
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
	vb.modelVisCube()=Complex(0.0,0.0);
      }
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
	vb.modelVisCube().xyPlane(row)=Complex(0.0,0.0);
      }
    
    findConvFunction(*image, vb);
    
    Nant_p     = vb.msColumns().antenna().nrow();
    Int NAnt=0;
    if (doPointing)   NAnt = findPointingOffsets(vb,l_offsets,m_offsets,True);
    
    // Get the uvws in a form that Fortran can use
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING to correct for an image inversion problem
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    //    dphase*=-1.0;
    
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);
    
    //Here we redo the match or use previous match
    //
    //Channel matching for the actual spectral window of buffer
    //
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    //No point in reading data if its not matching in frequency
    if(max(chanMap)==-1) return;
    //-----COMMONG TILL HERE
    
    // Cube<Int> flags(vb.flagCube().shape());
    // flags=0;
    // flags(vb.flagCube())=True;

    Cube<Complex> data;
    Cube<Int> flags;
    getInterpolateArrays(vb, data, flags);

    VBStore vbs;
    //    setupVBStore(vbs,vb, vb.imagingWeight(),vb.modelVisCube(),uvw,flags, dphase);
    setupVBStore(vbs,vb, vb.imagingWeight(),data,uvw,flags, dphase);

      // visResampler_p->setParams(uvScale,uvOffset,dphase);
      // visResampler_p->setMaps(chanMap, polMap);
    resampleGridToData(vbs, griddedData, vb);//, uvw, flags, dphase);

      // De-gridding
      //      visResampler_p->GridToData(vbs, griddedData);


    interpolateFrequencyFromgrid(vb, data, FTMachine::MODEL);
  }
  //
  //-------------------------------------------------------------------------
  // De-gridding
  void AWProjectFT::resampleGridToData(VBStore& vbs, Array<Complex>& griddedData_l,
				       const VisBuffer& /*vb*/)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "resampleGridToData[R&D]"));
    //    Timer tim;
    //    tim.mark();
    visResampler_p->GridToData(vbs, griddedData_l);
    //    runTime+=tim.user();
  }
  //
  //---------------------------------------------------------------
  //
  // Finalize the FFT to the Sky. Here we actually do the FFT and
  // return the resulting image
  ImageInterface<Complex>& AWProjectFT::getImage(Matrix<Float>& weights,
						  Bool fftNormalization) 
  {
    LogIO log_l(LogOrigin("AWProjectFT", "getImage[R&D]"));
    //
    // There are three objects held by the FTMachine objects: (1)
    // *image, (2) *lattice, and (3) griddedData.
    //
    // (1) is the Dirty Image (ImageInterface<Float>)
    //
    // (2) is the Lattice of the Dirty Image (Lattice<Float>)
    //
    // (3) appears to be a reference to (2).  Since FFT of *lattice is
    // done in place, griddedData (and *lattice) have the gridded data
    // in them before, and the Dirty Image data after the FFT.
    //
    // Question: Why three objects for the same precise information?
    // --SB (Dec. 2010).
    AlwaysAssert(image, AipsError);
    

    weights.resize(sumWeight.shape());
    convertArray(weights, sumWeight);

    //  
    // If the weights are all zero then we cannot normalize otherwise
    // we don't care.
    //
    if(max(weights)==0.0) 
      log_l // UUU //<< LogIO::SEVERE
	    << "No useful data in " << name() << ".  Weights all zero"
	    << LogIO::POST;
    // UUU else
      {
	const IPosition latticeShape = lattice->shape();
	log_l << LogIO::DEBUGGING
		<< "Starting FFT and scaling of image" << LogIO::POST;
	//    
	// x and y transforms (lattice has the gridded vis.  Make the
	// dirty images)
	//
	// if (useDoubleGrid_p)
	//   {
	//     ArrayLattice<DComplex> darrayLattice(griddedData2);
	//     LatticeFFT::cfft2d(darrayLattice,False);
	//     griddedData.resize(griddedData2.shape());
	//     convertArray(griddedData, griddedData2);
	
	//     //Don't need the double-prec grid anymore...
	//     griddedData2.resize();
	//     arrayLattice = new ArrayLattice<Complex>(griddedData);
	//     lattice=arrayLattice;
	//   }
	// else
	  {
	    arrayLattice = new ArrayLattice<Complex>(griddedData);
	    lattice=arrayLattice;
	    LatticeFFT::cfft2d(*lattice,False);
	  }
	//
	// Now normalize the dirty image.
	//
	// Since *lattice is not copied to *image till the end of this
	// method, normalizeImage also needs to work with Lattices
	// (rather than ImageInterface).
	//
	// //normalizeImage(*lattice,sumWeight,*avgPB_p,fftNormalization);
        //	normalizeImage(*lattice,sumWeight,*avgPB_p, *avgPBSq_p, fftNormalization);

	// nx ny normalization from GridFT...
	{
	  Int inx = lattice->shape()(0);
	  Int iny = lattice->shape()(1);
	  Vector<Complex> correction(inx);
	  correction=Complex(1.0, 0.0);
	  // Do the Grid-correction
	  IPosition cursorShape(4, inx, 1, 1, 1);
	  IPosition axisPath(4, 0, 1, 2, 3);
	  LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
	  LatticeIterator<Complex> lix(*lattice, lsx);
	  for(lix.reset();!lix.atEnd();lix++) 
	    {
	      Int pol=lix.position()(2);
	      Int chan=lix.position()(3);
	      {
		if(fftNormalization) 
		  {
		    if(weights(pol,chan)!=0.0)
		      {
			Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
			lix.rwCursor()*=rnorm;
		      }
		    else
		      lix.woCursor()=0.0;
		  }
		else 
		  {
		    Complex rnorm(Float(inx)*Float(iny));
		    lix.rwCursor()*=rnorm;
		  }
	      }
	    }
	}
	if(!isTiled) 
	  {
	    //
	    // Check the section from the image BEFORE converting to a lattice 
	    //
	    IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
			  (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
	    IPosition stride(4, 1);
	    IPosition trc(blc+image->shape()-stride);
	    //
	    // Do the copy
	    //
	    image->put(griddedData(blc, trc));
	    griddedData.resize(IPosition(1,0));
	  }
      }

    // {
    //   // TempImage<Complex> tt(lattice->shape(), image->coordinates());
    //   // tt.put(lattice->get());
    //   storeImg(String("uvgrid.im"), *image);
    // }

    return *image;
  }
  //
  //---------------------------------------------------------------
  //
  // Get weight image
  void AWProjectFT::getWeightImage(ImageInterface<Float>& weightImage,
				   Matrix<Float>& weights) 
  {
    LogIO log_l(LogOrigin("AWProjectFT", "getWeightImage[R&D]"));
    
    weights.resize(sumWeight.shape());
    convertArray(weights,sumWeight);
    
    const IPosition latticeShape = weightImage.shape();
    const IPosition avgpbShape = avgPB_p->shape();
    //    cout << "AWP::getWeightImage : weightimage shape : " << latticeShape << "  and avgpb shape : " << avgpbShape << " " << sumWeight << endl;
    
    Int nx=latticeShape(0);
    Int ny=latticeShape(1);
    
    IPosition loc(2, 0);
    IPosition cursorShape(4, nx, ny, 1, 1);
    IPosition axisPath(4, 0, 1, 2, 3);
    LatticeStepper lsx(latticeShape, cursorShape, axisPath);
    LatticeIterator<Float> lix(weightImage, lsx);
    LatticeIterator<Float> liy(*avgPB_p,lsx);
    for(lix.reset();!lix.atEnd();lix++) 
      {
	//Int pol=lix.position()(2);
	//Int chan=lix.position()(3);
	//lix.rwCursor()=weights(pol,chan);
	lix.rwCursor()=liy.rwCursor();
      }
  }
  //
  //---------------------------------------------------------------
  //
  Bool AWProjectFT::toRecord(RecordInterface& outRec, Bool withImage) 
  {
    LogIO log_l(LogOrigin("AWProjectFT", "toRecord[R&D]"));
    
    // Save the current AWProjectFT object to an output state record
    Bool retval = True;
    Double cacheVal=(Double) cachesize;
    outRec.define("cache", cacheVal);
    outRec.define("tile", tilesize);
    
    Vector<Double> phaseValue(2);
    String phaseUnit;
    phaseValue=mTangent_p.getAngle().getValue();
    phaseUnit= mTangent_p.getAngle().getUnit();
    outRec.define("phasevalue", phaseValue);
    outRec.define("phaseunit", phaseUnit);
    
    Vector<Double> dirValue(3);
    String dirUnit;
    dirValue=mLocation_p.get("m").getValue();
    dirUnit=mLocation_p.get("m").getUnit();
    outRec.define("dirvalue", dirValue);
    outRec.define("dirunit", dirUnit);
    
    outRec.define("padding", padding_p);
    outRec.define("maxdataval", maxAbsData);
    
    Vector<Int> center_loc(4), offset_loc(4);
    for (Int k=0; k<4 ; k++)
      {
	center_loc(k)=centerLoc(k);
	offset_loc(k)=offsetLoc(k);
      }
    outRec.define("centerloc", center_loc);
    outRec.define("offsetloc", offset_loc);
    outRec.define("sumofweights", sumWeight);
    outRec.define("sumofcfweights", sumCFWeight);
    if(withImage && image)
      { 
	ImageInterface<Complex>& tempimage(*image);
	Record imageContainer;
	String error;
	retval = (retval || tempimage.toRecord(error, imageContainer));
	outRec.defineRecord("image", imageContainer);
      }
    return retval;
  }
  //
  //---------------------------------------------------------------
  //
  Bool AWProjectFT::fromRecord(const RecordInterface& inRec)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "fromRecord[R&D]"));

    Bool retval = True;
    imageCache=0; lattice=0; arrayLattice=0;
    Double cacheVal;
    inRec.get("cache", cacheVal);
    cachesize=(Long) cacheVal;
    inRec.get("tile", tilesize);
    
    Vector<Double> phaseValue(2);
    inRec.get("phasevalue",phaseValue);
    String phaseUnit;
    inRec.get("phaseunit",phaseUnit);
    Quantity val1(phaseValue(0), phaseUnit);
    Quantity val2(phaseValue(1), phaseUnit); 
    MDirection phasecenter(val1, val2);
    
    mTangent_p=phasecenter;
    // This should be passed down too but the tangent plane is 
    // expected to be specified in all meaningful cases.
    tangentSpecified_p=True;  
    Vector<Double> dirValue(3);
    String dirUnit;
    inRec.get("dirvalue", dirValue);
    inRec.get("dirunit", dirUnit);
    MVPosition dummyMVPos(dirValue(0), dirValue(1), dirValue(2));
    MPosition mLocation(dummyMVPos, MPosition::ITRF);
    mLocation_p=mLocation;
    
    inRec.get("padding", padding_p);
    inRec.get("maxdataval", maxAbsData);
    
    Vector<Int> center_loc(4), offset_loc(4);
    inRec.get("centerloc", center_loc);
    inRec.get("offsetloc", offset_loc);
    uInt ndim4 = 4;
    centerLoc=IPosition(ndim4, center_loc(0), center_loc(1), center_loc(2), 
			center_loc(3));
    offsetLoc=IPosition(ndim4, offset_loc(0), offset_loc(1), offset_loc(2), 
			offset_loc(3));
    inRec.get("sumofweights", sumWeight);
    inRec.get("sumofcfweights", sumCFWeight);
    if(inRec.nfields() > 12 )
      {
	Record imageAsRec=inRec.asRecord("image");
	if(!image) image= new TempImage<Complex>(); 
	
	String error;
	retval = (retval || image->fromRecord(error, imageAsRec));    
	
	// Might be changing the shape of sumWeight
	init(); 
	
	if(isTiled) 
    	  lattice=CountedPtr<Lattice<Complex> > (image, False);
	else 
	  {
	    //
	    // Make the grid the correct shape and turn it into an
	    // array lattice Check the section from the image BEFORE
	    // converting to a lattice
	    //
	    IPosition gridShape(4, nx, ny, npol, nchan);
	    griddedData.resize(gridShape);
	    griddedData=Complex(0.0);
	    IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
			  (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
	    IPosition start(4, 0);
	    IPosition stride(4, 1);
	    IPosition trc(blc+image->shape()-stride);
	    griddedData(blc, trc) = image->getSlice(start, image->shape());
	    
	    arrayLattice = new ArrayLattice<Complex>(griddedData);
	    lattice=arrayLattice;
	  }
	
	AlwaysAssert(image, AipsError);
      };
    return retval;
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::ok() 
  {
    AlwaysAssert(image, AipsError);
  }
  //----------------------------------------------------------------------
  //
  // Make a straightforward 2D interferometry honest-to-god
  // image. This returns a complex image, without conversion to
  // Stokes. The polarization representation is that required for the
  // visibilities.
  //
  void AWProjectFT::makeImage(FTMachine::Type type, 
			      VisSet& vs,
			      ImageInterface<Complex>& theImage,
			      Matrix<Float>& weight) 
  {
    LogIO log_l(LogOrigin("AWProjectFT", "makeImage[R&D]"));
    
    if(type==FTMachine::COVERAGE) 
      log_l << "Type COVERAGE not defined for Fourier transforms"
	    << LogIO::EXCEPTION;
    
    
    // Initialize the gradients
    ROVisIter& vi(vs.iter());
    
    // Loop over all visibilities and pixels
    VisBuffer vb(vi);
    
    // Initialize put (i.e. transform to Sky) for this model
    vi.origin();
    
    if(vb.polFrame()==MSIter::Linear) 
      StokesImageUtil::changeCStokesRep(theImage, StokesImageUtil::LINEAR);
    else 
      StokesImageUtil::changeCStokesRep(theImage, StokesImageUtil::CIRCULAR);
    
    initializeToSky(theImage,weight,vb);

    //    
    // Loop over the visibilities, putting VisBuffers
    //
    paChangeDetector.reset();

    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	  {
	    if (type==FTMachine::PSF) makingPSF=True;
	    findConvFunction(theImage,vb);
	    
	    switch(type) 
	      {
	      case FTMachine::RESIDUAL:
		vb.visCube()=vb.correctedVisCube();
		vb.visCube()-=vb.modelVisCube();
		put(vb, -1, False);
		break;
	      case FTMachine::MODEL:
		vb.visCube()=vb.modelVisCube();
		put(vb, -1, False);
		break;
	      case FTMachine::CORRECTED:
		vb.visCube()=vb.correctedVisCube();
		put(vb, -1, False);
		break;
	      case FTMachine::PSF:
		vb.visCube()=Complex(1.0,0.0);
		makingPSF = True;
		put(vb, -1, True);
		break;
	      case FTMachine::OBSERVED:
	      default:
		put(vb, -1, False);
		break;
	      }
	  }
      }
    finalizeToSky();
    // Normalize by dividing out weights, etc.
    getImage(weight, True);
  }
  //
  //-------------------------------------------------------------------------
  //  
  void AWProjectFT::setPAIncrement(const Quantity& paIncrement)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "setPAIncrement[R&D]"));
    paChangeDetector.setTolerance(paIncrement);
    rotateAperture_p = True;
    if (paIncrement.getValue("rad") < 0)
      rotateAperture_p = False;
    log_l << LogIO::NORMAL <<"Setting PA increment to " 
	  << paIncrement.getValue("deg") << " deg" << endl;
    cfCache_p->setPAChangeDetector(paChangeDetector);
  }
  //
  //-------------------------------------------------------------------------
  //  
  Bool AWProjectFT::verifyShapes(IPosition pbShape, IPosition skyShape)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "verifyShapes[R&D]"));

    if ((pbShape(0) != skyShape(0)) && // X-axis
	(pbShape(1) != skyShape(1)) && // Y-axis
	(pbShape(2) != skyShape(2)))   // Poln-axis
      {
	log_l << "Sky and/or polarization shape of the avgPB and the sky model do not match."
	      << LogIO::EXCEPTION;
	return False;
      }
    return True;
    
  }
  //
  //-------------------------------------------------------------------------
  //  
  void AWProjectFT::setupVBStore(VBStore& vbs,
				 const VisBuffer& vb, 
				 const Matrix<Float>& imagingweight,
				 const Cube<Complex>& visData,
				 const Matrix<Double>& uvw,
				 const Cube<Int>& flagCube,
				 const Vector<Double>& dphase)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "setupVBStore[R&D]"));

    //    Vector<Int> ConjCFMap, CFMap;

    vbs.vb_p = &vb;
    makeCFPolMap(vb,cfStokes_p,CFMap_p);
    makeConjPolMap(vb,CFMap_p,ConjCFMap_p);

    visResampler_p->setParams(uvScale,uvOffset,dphase);
    visResampler_p->setMaps(chanMap, polMap);
    visResampler_p->setCFMaps(CFMap_p, ConjCFMap_p);
    visResampler_p->setFreqMaps(expandedSpwFreqSel_p,expandedSpwConjFreqSel_p);
    //
    // Set up VBStore object to point to the relavent info. of the VB.
    //
    vbs.imRefFreq_p = imRefFreq_p;
    vbs.nRow_p = vb.nRow();
    vbs.beginRow_p = 0;
    vbs.endRow_p = vbs.nRow_p;
    vbs.spwID_p = vb.spectralWindow();

    vbs.antenna1_p.reference(vb.antenna1());
    vbs.antenna2_p.reference(vb.antenna2());
    vbs.paQuant_p = Quantity(getPA(vb),"rad");
    vbs.corrType_p.reference(vb.corrType());
    vbs.uvw_p.reference(uvw);
    vbs.imagingWeight_p.reference(imagingweight);
    vbs.visCube_p.reference(visData);
    //    vbs.freq_p.reference(interpVisFreq_p);
    vbs.freq_p.reference(vb.frequency());
    //    vbs.rowFlag_p.assign(vb.flagRow());  
    vbs.rowFlag_p.reference(vb.flagRow());
    if(!usezero_p) 
      for (Int rownr=0; rownr<vbs.nRow_p; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) vbs.rowFlag_p(rownr)=True;

    // Really nice way of converting a Cube<Int> to Cube<Bool>.
    // However the VBS objects should ultimately be references
    // directly to bool cubes.
    //  vbs.rowFlag.resize(rowFlags.shape());  vbs.rowFlag  = False; vbs.rowFlag(rowFlags) = True;
    vbs.flagCube_p.resize(flagCube.shape());  vbs.flagCube_p = False; vbs.flagCube_p(flagCube!=0) = True;
    vbs.conjBeams_p=conjBeams_p;

    Vector<Double> pointingOffset(convFuncCtor_p->findPointingOffset(*image, vb));
    visResampler_p->makeVBRow2CFMap(*cfs2_p,*convFuncCtor_p, vb,
				    paChangeDetector.getParAngleTolerance(),
				    chanMap,polMap,pointingOffset);
    //    VBRow2CFMapType theMap(visResampler_p->getVBRow2CFMap());
    VBRow2CFBMapType& theMap=visResampler_p->getVBRow2CFBMap();
    //
    // For AzElApertures, this rotates the CFs.
    //
    convFuncCtor_p->prepareConvFunction(vb,theMap);
    //    visResampler_p->setConvFunc(cfs_p);
  }

  //
  //---------------------------------------------------------------
  //
  // Predict the coherences as well as their derivatives w.r.t. the
  // pointing offsets.
  //
  void AWProjectFT::nget(VisBuffer& vb,
			  // These offsets should be appropriate for the VB
			  Array<Float>& l_off, Array<Float>& m_off,
			  Cube<Complex>& Mout,
			  Cube<Complex>& dMout1,
			  Cube<Complex>& dMout2,
			  Int Conj, Int doGrad)
  {
    LogIO log_l(LogOrigin("AWProjectFT", "nget[R&D]"));
    Int startRow, endRow, nRow;
    nRow=vb.nRow();
    startRow=0;
    endRow=nRow-1;

    Mout = dMout1 = dMout2 = Complex(0,0);

    findConvFunction(*image, vb);
    Int NAnt=0;
    Nant_p     = vb.msColumns().antenna().nrow();
    if (doPointing)   
      NAnt = findPointingOffsets(vb,l_offsets,m_offsets,False);

    l_offsets=l_off;
    m_offsets=m_off;
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING to correct for an image inversion problem
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;

    Cube<Int> flags(vb.flagCube().shape());
    flags=0;
    flags(vb.flagCube())=True;
    
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);
    
    //Here we redo the match or use previous match
    //
    //Channel matching for the actual spectral window of buffer
    //
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    
    Vector<Int> rowFlags(vb.nRow());
    rowFlags=0;
    rowFlags(vb.flagRow())=True;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    
    IPosition s,gradS;
    Cube<Complex> visdata,gradVisAzData,gradVisElData;
    //
    // visdata now references the Mout data structure rather than to the internal VB storeage.
    //
    visdata.reference(Mout);

    if (doGrad)
      {
	// The following should reference some slice of dMout?
	gradVisAzData.reference(dMout1);
	gradVisElData.reference(dMout2);
      }
      visResampler_p->setParams(uvScale,uvOffset,dphase);
      visResampler_p->setMaps(chanMap, polMap);
    visResampler_p->setFreqMaps(expandedSpwFreqSel_p,expandedSpwConjFreqSel_p);
      //  Vector<Int> ConjCFMap, CFMap;
      makeCFPolMap(vb,cfStokes_p,CFMap_p);
      makeConjPolMap(vb,CFMap_p,ConjCFMap_p);
      visResampler_p->setCFMaps(CFMap_p, ConjCFMap_p);
    //
    // Begin the actual de-gridding.
    //
    if(isTiled) 
      {
	log_l << "The sky model is tiled" << LogIO::NORMAL << LogIO::POST;
	Double invLambdaC=vb.frequency()(0)/C::c;
	Vector<Double> uvLambda(2);
	Vector<Int> centerLoc2D(2);
	centerLoc2D=0;
	
	// Loop over all rows
	for (Int rownr=startRow; rownr<=endRow; rownr++) 
	  {
	    
	    // Calculate uvw for this row at the center frequency
	    uvLambda(0)=uvw(0, rownr)*invLambdaC;
	    uvLambda(1)=uvw(1, rownr)*invLambdaC;
	    centerLoc2D=gridder->location(centerLoc2D, uvLambda);
	    
	    // Is this point on the grid?
	    if(gridder->onGrid(centerLoc2D)) 
	      {
		
		// Get the tile
		Array<Complex>* dataPtr=getDataPointer(centerLoc2D, True);
		gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
		Int aNx=dataPtr->shape()(0);
		Int aNy=dataPtr->shape()(1);
		
		// Now use FORTRAN to do the gridding. Remember to 
		// ensure that the shape and offsets of the tile are 
		// accounted for.
		
		Vector<Double> actualOffset(3);
		for (Int i=0;i<2;i++) 
		  actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
		
		actualOffset(2)=uvOffset(2);
		IPosition s(vb.modelVisCube().shape());
		
		Int ScanNo=0, tmpPAI;
		Double area=1.0;
		tmpPAI = 1;
		runFortranGetGrad(uvw,dphase,visdata,s,
				  gradVisAzData,gradVisElData,
				  Conj,flags,rowFlags,rownr,
				  actualOffset,dataPtr,aNx,aNy,npol,nchan,vb,NAnt,ScanNo,sigma,
				  l_offsets,m_offsets,area,doGrad,tmpPAI);
	      }
	  }
      }
    else 
      {
	IPosition s(vb.modelVisCube().shape());
	Int ScanNo=0, tmpPAI, trow=-1;
	Double area=1.0;
	tmpPAI = 1;
	runFortranGetGrad(uvw,dphase,visdata/*vb.modelVisCube()*/,s,
			  gradVisAzData, gradVisElData,
			  Conj,flags,rowFlags,trow,
			  uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
			  l_offsets,m_offsets,area,doGrad,tmpPAI);
      }
    
  }
  void AWProjectFT::get(VisBuffer& vb,       
			 VisBuffer& gradVBAz,
			 VisBuffer& gradVBEl,
			 Cube<Float>& pointingOffsets,
			 Int row,  // default row=-1 
			 Type whichVBColumn, // default whichVBColumn = FTMachine::MODEL
			 Type whichGradVBColumn,// default whichGradVBColumn = FTMachine::MODEL
			 Int Conj, Int doGrad) // default Conj=0, doGrad=1
  {
    // If row is -1 then we pass through all rows
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
	initVisBuffer(vb,whichVBColumn);
	if (doGrad)
	  {
	    initVisBuffer(gradVBAz, whichGradVBColumn);
	    initVisBuffer(gradVBEl, whichGradVBColumn);
	  }
      }
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
	initVisBuffer(vb,whichVBColumn,row);
	if (doGrad)
	  {
	    initVisBuffer(gradVBAz, whichGradVBColumn,row);
	    initVisBuffer(gradVBEl, whichGradVBColumn,row);
	  }
      }
    
    findConvFunction(*image, vb);

    Nant_p     = vb.msColumns().antenna().nrow();
    Int NAnt=0;
    if (doPointing)   
      NAnt = findPointingOffsets(vb,pointingOffsets,l_offsets,m_offsets,False);

    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING to correct for an image inversion problem
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;
    
    
    Cube<Int> flags(vb.flagCube().shape());
    flags=0;
    flags(vb.flagCube())=True;
    //    
    //Check if ms has changed then cache new spw and chan selection
    //
    if(vb.newMS()) matchAllSpwChans(vb);
    
    //Here we redo the match or use previous match
    //
    //Channel matching for the actual spectral window of buffer
    //
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    
    Vector<Int> rowFlags(vb.nRow());
    rowFlags=0;
    rowFlags(vb.flagRow())=True;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
	
    for (Int rownr=startRow; rownr<=endRow; rownr++) 
      if (vb.antenna1()(rownr) != vb.antenna2()(rownr)) 
	rowFlags(rownr) = (vb.flagRow()(rownr)==True);
    
    IPosition s,gradS;
    Cube<Complex> visdata,gradVisAzData,gradVisElData;
    if (whichVBColumn == FTMachine::MODEL) 
      {
	s = vb.modelVisCube().shape();
	visdata.reference(vb.modelVisCube());
      }
    else if (whichVBColumn == FTMachine::OBSERVED)  
      {
	s = vb.visCube().shape();
	visdata.reference(vb.visCube());
      }
    
    if (doGrad)
      {
	if (whichGradVBColumn == FTMachine::MODEL) 
	  {
	    //	    gradS = gradVBAz.modelVisCube().shape();
	    gradVisAzData.reference(gradVBAz.modelVisCube());
	    gradVisElData.reference(gradVBEl.modelVisCube());
	  }
	else if (whichGradVBColumn == FTMachine::OBSERVED)  
	  {
	    //	    gradS = gradVBAz.visCube().shape();
	    gradVisAzData.reference(gradVBAz.visCube());
	    gradVisElData.reference(gradVBEl.visCube());
	  }
      }
      visResampler_p->setParams(uvScale,uvOffset,dphase);
    visResampler_p->setFreqMaps(expandedSpwFreqSel_p,expandedSpwConjFreqSel_p);
      visResampler_p->setMaps(chanMap, polMap);
      //  Vector<Int> ConjCFMap, CFMap;
    makeCFPolMap(vb,cfStokes_p,CFMap_p);
  makeConjPolMap(vb,CFMap_p,ConjCFMap_p);
  visResampler_p->setCFMaps(CFMap_p, ConjCFMap_p);
    
    if(isTiled) 
      {
	Double invLambdaC=vb.frequency()(0)/C::c;
	Vector<Double> uvLambda(2);
	Vector<Int> centerLoc2D(2);
	centerLoc2D=0;
	
	// Loop over all rows
	for (Int rownr=startRow; rownr<=endRow; rownr++) 
	  {
	    
	    // Calculate uvw for this row at the center frequency
	    uvLambda(0)=uvw(0, rownr)*invLambdaC;
	    uvLambda(1)=uvw(1, rownr)*invLambdaC;
	    centerLoc2D=gridder->location(centerLoc2D, uvLambda);
	    
	    // Is this point on the grid?
	    if(gridder->onGrid(centerLoc2D)) 
	      {
		
		// Get the tile
		Array<Complex>* dataPtr=getDataPointer(centerLoc2D, True);
		gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
		Int aNx=dataPtr->shape()(0);
		Int aNy=dataPtr->shape()(1);
		
		// Now use FORTRAN to do the gridding. Remember to 
		// ensure that the shape and offsets of the tile are 
		// accounted for.
		
		Vector<Double> actualOffset(3);
		for (Int i=0;i<2;i++) 
		  actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
		
		actualOffset(2)=uvOffset(2);
		IPosition s(vb.modelVisCube().shape());
		
		Int ScanNo=0, tmpPAI;
		Double area=1.0;
		tmpPAI = 1;
		runFortranGetGrad(uvw,dphase,visdata,s,
				  gradVisAzData,gradVisElData,
				  Conj,flags,rowFlags,rownr,
				  actualOffset,dataPtr,aNx,aNy,npol,nchan,vb,NAnt,ScanNo,sigma,
				  l_offsets,m_offsets,area,doGrad,tmpPAI);
	      }
	  }
      }
    else 
      {
	
	IPosition s(vb.modelVisCube().shape());
	Int ScanNo=0, tmpPAI;
	Double area=1.0;

	tmpPAI = 1;

	runFortranGetGrad(uvw,dphase,visdata/*vb.modelVisCube()*/,s,
			  gradVisAzData, gradVisElData,
			  Conj,flags,rowFlags,row,
			  uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
			  l_offsets,m_offsets,area,doGrad,tmpPAI);
// 	runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
// 		      uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
// 		      l_offsets,m_offsets,area,doGrad,tmpPAI);
      }
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::get(VisBuffer& vb, Cube<Complex>& modelVis, 
			 Array<Complex>& griddedVis, Vector<Double>& scale,
			 Int row)
  {

    (void)scale; //Suppress the warning

    Int nX=griddedVis.shape()(0);
    Int nY=griddedVis.shape()(1);
    Vector<Double> offset(2);
    offset(0)=Double(nX)/2.0;
    offset(1)=Double(nY)/2.0;
    // If row is -1 then we pass through all rows
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
	modelVis.set(Complex(0.0,0.0));
      } 
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
	modelVis.xyPlane(row)=Complex(0.0,0.0);
      }
    
    Int NAnt=0;
    
    if (doPointing) 
      NAnt = findPointingOffsets(vb,l_offsets, m_offsets,True);
    
    
    //  
    // Get the uvws in a form that Fortran can use
    //
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //
    //NEGATING to correct for an image inversion problem
    //
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;
    
    Cube<Int> flags(vb.flagCube().shape());
    flags=0;
    flags(vb.flagCube())=True;
    
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);
    
    //Channel matching for the actual spectral window of buffer
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    
    Vector<Int> rowFlags(vb.nRow());
    rowFlags=0;
    rowFlags(vb.flagRow())=True;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    
    visResampler_p->setParams(uvScale,uvOffset,dphase);
    visResampler_p->setMaps(chanMap, polMap);
    visResampler_p->setFreqMaps(expandedSpwFreqSel_p,expandedSpwConjFreqSel_p);

    IPosition s(modelVis.shape());
    Int Conj=0,doGrad=0,ScanNo=0;
    Double area=1.0;
    Int tmpPAI=1;
    runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
		  offset,&griddedVis,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		  l_offsets,m_offsets,area,doGrad,tmpPAI);
  }
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::initVisBuffer(VisBuffer& vb, Type whichVBColumn)
  {
    if (whichVBColumn      == FTMachine::MODEL)    vb.modelVisCube()=Complex(0.0,0.0);
    else if (whichVBColumn == FTMachine::OBSERVED) vb.visCube()=Complex(0.0,0.0);
  }
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::initVisBuffer(VisBuffer& vb, Type whichVBColumn, Int row)
  {
    if (whichVBColumn == FTMachine::MODEL)
      vb.modelVisCube().xyPlane(row)=Complex(0.0,0.0);
    else if (whichVBColumn == FTMachine::OBSERVED)
      vb.visCube().xyPlane(row)=Complex(0.0,0.0);
  }

  void AWProjectFT::ComputeResiduals(VisBuffer&vb, Bool useCorrected)
  {
    VBStore vbs;

    vbs.nRow_p = vb.nRow();
    vbs.beginRow_p = 0;
    vbs.endRow_p = vbs.nRow_p;

    vbs.modelCube_p.reference(vb.modelVisCube());
    if (useCorrected) vbs.correctedCube_p.reference(vb.correctedVisCube());
    else vbs.visCube_p.reference(vb.visCube());
    vbs.useCorrected_p = useCorrected;
    visResampler_p->ComputeResiduals(vbs);
  }

  // void AWProjectFT::ComputeResiduals(VBStore &vbs)
  // {
  //   vbs.nRow_p = vb.nRow();
  //   vbs.modelCube_p.reference(vb.modelVisCube());
  //   if (useCorrected) vbs.correctedCube_p.reference(vb.correctedVisCube());
  //   else vbs.visCube_p.reference(vb.visCube());
  //   vbs.useCorrected_p = useCorrected;
  //   visResampler_p->ComputeResiduals(vbs);
  // }

} //# NAMESPACE CASA - END




  // //
  // //---------------------------------------------------------------
  // //
  // void AWProjectFT::normalizeImage(Lattice<Complex>& skyImage,
  // 				   const Matrix<Double>& sumOfWts,
  // 				   Lattice<Float>& sensitivityImage,
  // 				   Lattice<Complex>& sensitivitySqImage,
  // 				   Bool fftNorm)
  // {
  //   //
  //   // Apply the gridding correction
  //   //    
  //   Int inx = skyImage.shape()(0);
  //   Int iny = skyImage.shape()(1);
  //   //    Vector<Complex> correction(inx);
	  
  //   Vector<Float> sincConv(nx);
  //   Float centerX=nx/2;
  //   for (Int ix=0;ix<nx;ix++) 
  //     {
  // 	Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
  // 	if(ix==centerX) sincConv(ix)=1.0;
  // 	else 	    sincConv(ix)=sin(x)/x;
  //     }
	  
  //   IPosition cursorShape(4, inx, 1, 1, 1);
  //   IPosition axisPath(4, 0, 1, 2, 3);
  //   LatticeStepper lsx(skyImage.shape(), cursorShape, axisPath);
  //   LatticeIterator<Complex> lix(skyImage, lsx);
    
  //   LatticeStepper lavgpb(sensitivityImage.shape(),cursorShape,axisPath);
  //   LatticeIterator<Float> liavgpb(sensitivityImage, lavgpb);
  //   LatticeStepper lavgpbSq(sensitivitySqImage.shape(),cursorShape,axisPath);
  //   LatticeIterator<Complex> liavgpbSq(sensitivitySqImage, lavgpbSq);
	  
  //   for(lix.reset(),liavgpb.reset(),liavgpbSq.reset();
  // 	!lix.atEnd();
  // 	lix++,liavgpb++,liavgpbSq++) 
  //     {
  // 	Int pol=lix.position()(2);
  // 	Int chan=lix.position()(3);
  // 	//	Int iy=lix.position()(1);

  // 	//	gridder->correctX1D(correction,iy);
	    
  // 	Vector<Complex> PBCorrection(liavgpb.rwVectorCursor().shape());
  // 	Vector<Float> avgPBVec(liavgpb.rwVectorCursor().shape());
  // 	Vector<Complex> avgPBSqVec(liavgpbSq.rwVectorCursor().shape());
	    
  // 	avgPBSqVec= liavgpbSq.rwVectorCursor();
  // 	avgPBVec = liavgpb.rwVectorCursor();

  // 	for(int i=0;i<PBCorrection.shape();i++)
  // 	  {
  // 	    PBCorrection(i)=(avgPBSqVec(i)/avgPBVec(i));///(sincConv(i)*sincConv(iy));
  // 	    if ((abs(avgPBVec(i))) >= pbLimit_p)
  // 	      lix.rwVectorCursor()(i) /= PBCorrection(i);
  // 	  }

  // 	//	if(sumOfWts(pol, chan)>0.0)  // UUU Changing this again....
  // 	  {
  // 	    if(fftNorm)
  // 	      {
  // 		if(sumOfWts(pol, chan)>0.0) // UUU Moving this inside the fftNorm check.
  // 		  {
  // 		    Complex rnorm(Float(inx)*Float(iny)/sumOfWts(pol,chan));
  // 		    lix.rwCursor()*=rnorm;
  // 		  }
  // 		else 
  // 		  lix.woCursor()=0.0;
  // 	      }
  // 	    else 
  // 	      {
  // 		Complex rnorm(Float(inx)*Float(iny));
  // 		lix.rwCursor()*=rnorm;
  // 	      }
  // 	  }
  // 	  //else 
  // 	  //lix.woCursor()=0.0;
  //     }
  // }
  // //
  // //---------------------------------------------------------------
  // //
  // void AWProjectFT::normalizeImage(Lattice<Complex>& skyImage,
  // 				   const Matrix<Double>& sumOfWts,
  // 				   Lattice<Float>& sensitivityImage,
  // 				   Bool fftNorm)
  // {
  //   //
  //   // Apply the gridding correction
  //   //    
  //   Int inx = skyImage.shape()(0);
  //   Int iny = skyImage.shape()(1);
  //   Vector<Complex> correction(inx);
	  
  //   Vector<Float> sincConv(nx);
  //   Float centerX=nx/2;
  //   for (Int ix=0;ix<nx;ix++) 
  //     {
  // 	Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
  // 	if(ix==centerX) sincConv(ix)=1.0;
  // 	else 	    sincConv(ix)=sin(x)/x;
  //     }
	  
  //   IPosition cursorShape(4, inx, 1, 1, 1);
  //   IPosition axisPath(4, 0, 1, 2, 3);
  //   LatticeStepper lsx(skyImage.shape(), cursorShape, axisPath);
  //   //    LatticeIterator<Complex> lix(skyImage, lsx);
  //   LatticeIterator<Complex> lix(skyImage, lsx);
    
  //   LatticeStepper lavgpb(sensitivityImage.shape(),cursorShape,axisPath);
  //   // Array<Float> senArray;sensitivityImage.get(senArray,True);
  //   // ArrayLattice<Float> senLat(senArray,True);
  //   //    LatticeIterator<Float> liavgpb(senLat, lavgpb);
  //   LatticeIterator<Float> liavgpb(sensitivityImage, lavgpb);

  //   for(lix.reset(),liavgpb.reset();
  // 	!lix.atEnd();
  // 	lix++,liavgpb++) 
  //     {
  // 	Int pol=lix.position()(2);
  // 	Int chan=lix.position()(3);
	
  // 	//	if(sumOfWts(pol, chan)>0.0)  // UUU Changing this again.....
  // 	  {
  // 	    Int iy=lix.position()(1);
  // 	    gridder->correctX1D(correction,iy);
	    
  // 	    Vector<Float> avgPBVec(liavgpb.rwVectorCursor().shape());
	    
  // 	    avgPBVec = liavgpb.rwVectorCursor();

  // 	    for(int i=0;i<avgPBVec.shape();i++)
  // 	      {
  // 		//
  // 		// This with the PS functions
  // 		//
  // 		// PBCorrection(i)=FUNC(avgPBVec(i))*sincConv(i)*sincConv(iy);
  // 		// if ((abs(PBCorrection(i)*correction(i))) >= pbLimit_p)
  // 		// 	lix.rwVectorCursor()(i) /= PBCorrection(i)*correction(i);
  // 		// else if (!makingPSF)
  // 		// 	lix.rwVectorCursor()(i) /= correction(i)*sincConv(i)*sincConv(iy);
  // 		//
  // 		// This without the PS functions
  // 		//
  // 		Float tt = pbFunc(avgPBVec(i),pbLimit_p)*sincConv(i)*sincConv(iy);
  //                 //		PBCorrection(i)=pbFunc(avgPBVec(i),pbLimit_p)*sincConv(i)*sincConv(iy);
  //                 //                lix.rwVectorCursor()(i) /= PBCorrection(i);
  // 		//                lix.rwVectorCursor()(i) *= tt;
  // 		//		if (!makingPSF)

  // 		lix.rwVectorCursor()(i) /= tt;
  // 	      }
	    
  // 	    if(fftNorm)
  // 	      {
  // 		if(sumOfWts(pol, chan)>0.0) // UUU Moved this inside the fftNorm check.
  // 		  {
  // 		    Complex rnorm(Float(inx)*Float(iny)/sumOfWts(pol,chan));
  // 		    lix.rwCursor()*=rnorm;
  // 		  }
  // 		else 
  // 		  lix.woCursor()=0.0;
  // 	      }
  // 	    else 
  // 	      {
  // 		Complex rnorm(Float(inx)*Float(iny));
  // 		lix.rwCursor()*=rnorm;
  // 	      }
  // 	  }
  // 	  //else 
  // 	  //lix.woCursor()=0.0;
  //     }
  // }
