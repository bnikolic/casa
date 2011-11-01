//# FITSQualityImage.cc: Class providing native access to FITS images
//# Copyright (C) 2001,2002
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
//# $Id: FITSImage.cc 20859 2010-02-03 13:14:15Z gervandiepen $

#include <images/Images/FITSQualityImage.h>

#include <images/Images/FITSImage.h>
#include <images/Images/FITSErrorImage.h>
#include <lattices/Lattices/FITSQualityMask.h>

#include <images/Images/FITSImgParser.h>
#include <fits/FITS/hdu.h>
#include <fits/FITS/fitsio.h>
#include <fits/FITS/FITSKeywordUtil.h>
#include <images/Images/ImageInfo.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/MaskSpecifier.h>
#include <images/Images/ImageOpener.h>
#include <lattices/Lattices/TiledShape.h>
#include <lattices/Lattices/TempLattice.h>
#include <lattices/Lattices/FITSMask.h>
#include <tables/Tables/TiledFileAccess.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/QualityCoordinate.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/Record.h>
#include <tables/LogTables/LoggerHolder.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/File.h>
#include <casa/Quanta/Unit.h>
#include <measures/Measures/Quality.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Utilities/ValType.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>

#include <casa/iostream.h>



namespace casa { //# NAMESPACE CASA - BEGIN

FITSQualityImage::FITSQualityImage(const String& name)
: ImageInterface<Float>(),
  name_p          (name),
  fullname_p      (name),
  fitsdata_p      (0),
  fitserror_p     (0),
  whichDataHDU_p  (0),
  whichErrorHDU_p (0),
  whichMaskHDU_p  (0),
  isClosed_p      (False),
  isDataClosed_p  (False),
  isErrorClosed_p (False)
{
   getExtInfo();
   setup();
}

FITSQualityImage::FITSQualityImage(const String& name, uInt whichDataHDU, uInt whichErrorHDU)
: ImageInterface<Float>(),
  name_p          (name),
  fullname_p      (name),
  fitsdata_p      (0),
  fitserror_p     (0),
  whichDataHDU_p  (whichDataHDU),
  whichErrorHDU_p (whichErrorHDU),
  whichMaskHDU_p  (0),
  errType_p       (FITSErrorImage::DEFAULT),
  isClosed_p      (False),
  isDataClosed_p  (False),
  isErrorClosed_p (False)
{
  setup();
}


FITSQualityImage::FITSQualityImage (const FITSQualityImage& other)
: ImageInterface<Float>(other),
  name_p          (other.name_p),
  fitsdata_p      (0),
  fitserror_p     (0),
  shape_p         (other.shape_p),
  whichDataHDU_p  (other.whichDataHDU_p),
  whichErrorHDU_p (other.whichErrorHDU_p),
  whichMaskHDU_p  (other.whichMaskHDU_p),
  errType_p       (other.errType_p),
  isClosed_p      (other.isClosed_p),
  isDataClosed_p  (other.isDataClosed_p),
  isErrorClosed_p (other.isErrorClosed_p)
{
	if (other.fitsdata_p != 0) {
		fitsdata_p = dynamic_cast<FITSImage *>(other.fitsdata_p->cloneII());
	}
	if (other.fitserror_p != 0) {
		fitserror_p = dynamic_cast<FITSErrorImage *>(other.fitserror_p->cloneII());
	}
    if (fitsdata_p != 0 && fitserror_p != 0 && fitsdata_p->isMasked())
  	  pPixelMask_p = new FITSQualityMask(fitsdata_p, fitserror_p);
}
 
FITSQualityImage& FITSQualityImage::operator=(const FITSQualityImage& other)
// 
// Assignment. Uses reference semantics
//
{
   if (this != &other) {
      ImageInterface<Float>::operator= (other);
      delete fitsdata_p;
      fitsdata_p = 0;
      if (other.fitsdata_p != 0) {
    	  fitsdata_p = dynamic_cast<FITSImage *>(other.fitsdata_p->cloneII());

      }
      delete fitserror_p;
      fitserror_p = 0;
      if (other.fitserror_p != 0) {
    	  fitserror_p = dynamic_cast<FITSErrorImage *>(other.fitserror_p->cloneII());
      }
      if (fitsdata_p != 0 && fitserror_p != 0 && fitsdata_p->isMasked())
    	  pPixelMask_p = new FITSQualityMask(fitsdata_p, fitserror_p);
      name_p          = other.name_p;
      shape_p         = other.shape_p;
      whichDataHDU_p  = other.whichDataHDU_p;
      whichErrorHDU_p = other.whichErrorHDU_p;
      whichMaskHDU_p  = other.whichMaskHDU_p;
      errType_p       = other.errType_p;
      isClosed_p      = other.isClosed_p;
      isDataClosed_p  = other.isDataClosed_p;
      isErrorClosed_p = other.isErrorClosed_p;
   }
   return *this;
} 


FITSQualityImage::~FITSQualityImage()
{
	if (fitsdata_p) {
		delete fitsdata_p;
		fitsdata_p=0;
	}
	if (fitserror_p){
		delete fitserror_p;
		fitserror_p=0;
	}
}


ImageInterface<Float>* FITSQualityImage::cloneII() const
{
   return new FITSQualityImage (*this);
}


String FITSQualityImage::imageType() const
{
   return "FITSQualityImage";
}

void FITSQualityImage::resize(const TiledShape&)
{
   throw (AipsError ("FITSQualityImage::resize - a FITSQualityImage is not writable"));
}

Bool FITSQualityImage::isMasked() const
{
	return fitsdata_p->isMasked();
}

Bool FITSQualityImage::hasPixelMask() const
{
	return fitsdata_p->isMasked();
}

const Lattice<Bool>& FITSQualityImage::pixelMask() const
{
   if (!fitsdata_p->isMasked()) {
      throw (AipsError ("FITSQualityImage::pixelMask - no pixelmask used"));
   }
   return *pPixelMask_p;
}

Lattice<Bool>& FITSQualityImage::pixelMask()
{
   if (!fitsdata_p->isMasked()) {
      throw (AipsError ("FITSQualityImage::pixelMask - no pixelmask used"));
   }
   return *pPixelMask_p;
}

const LatticeRegion* FITSQualityImage::getRegionPtr() const
{
   return 0;
}

Bool FITSQualityImage::doGetSlice(Array<Float>& buffer, const Slicer& section)
{
	// get the section dimension
	IPosition shp = section.length();
	uInt ndim=section.ndim();

	// resize the buffer
	if (!buffer.shape().isEqual(shp)) buffer.resize(shp);

	// set the in all except the last dimension
	IPosition tmpStart(ndim-1);
	IPosition tmpEnd(ndim-1);
	IPosition tmpStride(ndim-1);
	for (uInt index=0; index<ndim-1; index++) {
		tmpStart(index)  = section.start()(index);
		tmpEnd(index)    = section.end()(index);
		tmpStride(index) = section.stride()(index);
	}

	// generate a slicer for all except the last dimension;
	// used for getting the data from the individual extensions
	Slicer subSection(tmpStart, tmpEnd, tmpStride, Slicer::endIsLast);

	// analyze the request
	if (section.start()(ndim-1) != section.end()(ndim-1)){

		// data and error is requested
		Array<Float> subData;
		Array<Float> subError;
		Array<Float> tmp;

		// prepare the call
		// for data values
		IPosition subStart(ndim);
		IPosition subEnd(ndim);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subData.shape().isEqual(subSection.length())) subData.resize(subSection.length());

		// get the data values
		reopenDataIfNeeded();
		fitsdata_p->doGetSlice(subData, subSection);
		tempCloseData();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subData.addDegenerate(1);


		// prepare the call
		// for error values
		subStart(ndim-1) = 1;
		subEnd(ndim-1)   = 1;

		// re-size the buffer
		if (!subError.shape().isEqual(subSection.length())) subError.resize(subSection.length());

		// get the error values
		reopenErrorIfNeeded();
		fitserror_p->doGetSlice(subError, subSection);
		tempCloseError();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subError.addDegenerate(1);
	}
	else if (section.start()(ndim-1)==0) {

		// only data is requested
		Array<Float> subData;
		Array<Float> tmp;

		// prepare the call
		// for data values
		IPosition subStart(ndim);
		IPosition subEnd(ndim);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subData.shape().isEqual(subSection.length())) subData.resize(subSection.length());

		// get the data values
		reopenDataIfNeeded();
		fitsdata_p->doGetSlice(subData, subSection);
		tempCloseData();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subData.addDegenerate(1);
	}
	else if (section.start()(ndim-1)==1) {

		// only error values are requested
		Array<Float> subError;
		Array<Float> tmp;

		// prepare the call
		// for error values
		IPosition subStart(ndim, 1);
		IPosition subEnd(ndim, 1);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subError.shape().isEqual(subSection.length())) subError.resize(subSection.length());

		// get the error values
		reopenErrorIfNeeded();
		fitserror_p->doGetSlice(subError, subSection);
		tempCloseError();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subError.addDegenerate(1);
	}

	return False;
}

Bool FITSQualityImage::doGetMaskSlice (Array<Bool>& buffer, const Slicer& section)
{
	if (!fitsdata_p->isMasked()) {
		buffer.resize (section.length());
		buffer = True;
		return False;
	}
//
	reopenIfNeeded();
	return pPixelMask_p->getSlice (buffer, section);
}

void FITSQualityImage::doPutSlice (const Array<Float>&, const IPosition&,
                            const IPosition&)
{
   throw (AipsError ("FITSQualityImage::putSlice - "
		     "is not possible as FITSQualityImage is not writable"));
}

Bool FITSQualityImage::isPersistent() const
{
  return True;
}

Bool FITSQualityImage::isPaged() const
{
  return True;
}

Bool FITSQualityImage::isWritable() const
{
// Its too hard to implement putMaskSlice becuase
// magic blanking is used. It means we lose
// the data values if the mask is put somewhere

   return False;
}

String FITSQualityImage::name (Bool stripPath) const
{
   return fitsdata_p->name(stripPath);
}


IPosition FITSQualityImage::shape() const
{
   return shape_p.shape();
}

uInt FITSQualityImage::advisedMaxPixels() const
{
   return shape_p.tileShape().product();
}

IPosition FITSQualityImage::doNiceCursorShape (uInt) const
{
   return shape_p.tileShape();
}

Bool FITSQualityImage::ok() const
{
   return True;
}  

void FITSQualityImage::tempClose()
{
	//cout << "close!" << endl;
   if (! isClosed_p) {
	   fitsdata_p->tempClose();
	   fitserror_p->tempClose();
   }
}
void FITSQualityImage::tempCloseData()
{
   if (! isDataClosed_p) {
		//cout << "Data closed!" << endl;
	   fitsdata_p->tempClose();
   }
   isDataClosed_p = True;
}
void FITSQualityImage::tempCloseError()
{
   if (! isErrorClosed_p) {
	   //cout << "Error closed!" << endl;
	   fitserror_p->tempClose();
   }
   isErrorClosed_p = True;
}

void FITSQualityImage::reopen()
{
	//cout << "reopen!" << endl;
   if (isClosed_p) {
	   fitsdata_p->reopen();
	   fitserror_p->reopen();
   }
}

DataType FITSQualityImage::dataType () const
{
	return fitsdata_p->dataType();
}

uInt FITSQualityImage::maximumCacheSize() const
{
   reopenIfNeeded();
   return fitsdata_p->maximumCacheSize();
}

void FITSQualityImage::setMaximumCacheSize (uInt howManyPixels)
{
   reopenIfNeeded();
   fitsdata_p->setMaximumCacheSize(howManyPixels);
   fitserror_p->setMaximumCacheSize(howManyPixels);
}

void FITSQualityImage::setCacheSizeFromPath (const IPosition& sliceShape,
				      const IPosition& windowStart,
				      const IPosition& windowLength,
				      const IPosition& axisPath)
{
   reopenIfNeeded();
   fitsdata_p->setCacheSizeFromPath(sliceShape, windowStart,
			       windowLength, axisPath);
   fitserror_p->setCacheSizeFromPath(sliceShape, windowStart,
			       windowLength, axisPath);
}

void FITSQualityImage::setCacheSizeInTiles (uInt howManyTiles)
{  
   reopenIfNeeded();
   fitsdata_p->setCacheSizeInTiles(howManyTiles);
   fitserror_p->setCacheSizeInTiles(howManyTiles);
}


void FITSQualityImage::clearCache()
{
   if (! isClosed_p) {
	   fitsdata_p->clearCache();
	   fitserror_p->clearCache();
   }
}

void FITSQualityImage::showCacheStatistics (ostream& os) const
{
   reopenIfNeeded();
   os << "FITSQualityImage statistics : ";
   fitsdata_p->showCacheStatistics(os);
   fitserror_p->showCacheStatistics(os);
}

void FITSQualityImage::getExtInfo()
{
   LogIO os(LogOrigin("FITSQualityImage", "getExtInfo", WHERE));

	String extexpr;
	Int whichDataHDU;
	Int whichErrorHDU;
	Int whichMaskHDU;
	Int maskValue;
	String errTypeStr;
	String maskTypeStr;

	// decompose the name into the fits name and extension list
	name_p  = FITSImage::get_fitsname(fullname_p);
	extexpr = String(fullname_p, name_p.size(), fullname_p.size()-name_p.size());

	// open a FITS parser
	FITSImgParser fip = FITSImgParser(name_p);

	// make sure that the extension expression
	// can be loaded as a quality image
	if (!fip.is_qualityimg(extexpr))
	   throw (AipsError ("FITSQualityImage::getExtInfo - "
	   		"The extensions " + extexpr + " in image: " + name_p + " can not be loaded as quality image!"));

	// determine all information to be able loading a quality image
	fip.get_quality_data(extexpr, whichDataHDU, whichErrorHDU, errTypeStr, whichMaskHDU, maskTypeStr, maskValue);

	// store the data extension,
	// exit if there is none
	if (whichDataHDU > -1)
		whichDataHDU_p = (uInt)whichDataHDU;
	else
	   throw (AipsError ("FITSQualityImage::getExtInfo - "
	   		"No data extension"));

	// store the error extension,
	// exit if there is none
	// Note: Since mask files can not yet loaded,
	//       the error extension is essential to
	//       make a quality image
	if (whichErrorHDU > -1)
		whichErrorHDU_p = (uInt)whichErrorHDU;
	else
	   throw (AipsError ("FITSQualityImage::getExtInfo - "
	   		"No error extension"));

	// convert the keyword value to
	// an error type
	if (errTypeStr.size()>0){
		errType_p = FITSErrorImage::stringToErrorType(errTypeStr);
		if (errType_p == FITSErrorImage::UNKNOWN)
		   throw (AipsError ("FITSQualityImage::getExtInfo - "
		   		"Unknown ERRTYPE value: " + errTypeStr));
	}
	else{
      os << LogIO::WARN << "No proper error type defined in the error extension. Assuming MSE (mean squared error)." << LogIO::POST;
	}

	// store the mask extension
	// notify that the mask extension
	// is not used.
	if (whichMaskHDU > -1){
		whichMaskHDU_p = whichMaskHDU;
		os << LogIO::NORMAL << "A dedicated mask extension can not yet be loaded!" << LogIO::POST;
	}
}

void FITSQualityImage::setup()
{
	// open the various fits extensions
	fitsdata_p  = new FITSImage(name_p, 0, whichDataHDU_p);
	fitserror_p = new FITSErrorImage(name_p, 0, whichErrorHDU_p, errType_p);

	// do some checks on the input images
	Bool ok;
	ok = checkInput();

	// create the pixel mask
	pPixelMask_p = new FITSQualityMask(fitsdata_p, fitserror_p);

	IPosition data_shape=fitsdata_p->shape();
	IPosition mm_shape(data_shape.nelements()+1);

	// set the shape
	for (uInt index=0; index<data_shape.nelements(); index++)
		mm_shape(index) = data_shape(index);
	mm_shape(mm_shape.nelements()-1)=2;

	// grab the coo-sys of the data image image
	CoordinateSystem cSys = fitsdata_p->coordinates();

	Vector<Int> quality(2);
	quality(0) = Quality::DATA;
	quality(1) = Quality::ERROR;
	QualityCoordinate qualAxis(quality);
	cSys.addCoordinate(qualAxis);

	setCoordsMember(cSys);
	setImageInfo(fitsdata_p->imageInfo());

	// Form the tile shape.
	shape_p = TiledShape (mm_shape, TiledFileAccess::makeTileShape(mm_shape));
}

Bool FITSQualityImage::checkInput(){

	// make sure the data end error extensions
	// are NOT identical
	if (whichDataHDU_p == whichErrorHDU_p)
		throw (AipsError("Data and error extensions must be different!"));

	// make sure the data and error image have the same dimension
	if (fitsdata_p->shape() != fitserror_p->shape())
		throw (AipsError("Data and error image have different shape!"));

	// make sure the data and error image have the same coordinate system
	CoordinateSystem dataCSys  = fitsdata_p->coordinates();
	CoordinateSystem errorCSys = fitserror_p->coordinates();
	if (!dataCSys.near(errorCSys, 10e-6))
		throw (AipsError("Data and error image have different coordinate system!"));

	return True;
}

void FITSQualityImage::reopenIfNeeded() const
  { if (isClosed_p){
	  fitsdata_p->reopen();
	  fitserror_p->reopen();
  	  }
  }
void FITSQualityImage::reopenErrorIfNeeded()
  { if (isErrorClosed_p){
	  fitserror_p->reopen();
	  isErrorClosed_p = False;
  	  }
  }
void FITSQualityImage::reopenDataIfNeeded()
  { if (isDataClosed_p){
	  fitsdata_p->reopen();
	  isDataClosed_p = False;
  	  }
  }
} //# NAMESPACE CASA - END

