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

#ifndef IMAGEANALYSIS_IMAGEREGRIDDER_H
#define IMAGEANALYSIS_IMAGEREGRIDDER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <scimath/Mathematics/Interpolate2D.h>
#include <casa/namespace.h>

#include <tr1/memory.hpp>

namespace casa {

template <class T> class SubImage;
template <class T> class TempImage;

class ImageRegridder : public ImageTask {
	// <summary>
	// Top level interface which regrids an image to a specified coordinate system
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Regrids image.
	// </etymology>

	// <synopsis>
	// High level interface for regridding an image.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImageCollapser collapser();
	// collapser.collapse();
	// </srcblock>
	// </example>

public:

	// if <src>outname</src> is empty, no image will be written
	// if <src>overwrite</src> is True, if image already exists it will be removed
	// if <src>overwrite</src> is False, if image already exists exception will be thrown
	// <group>

	ImageRegridder(
		const ImageTask::shCImFloat image,
		const Record *const regionRec,
		const String& maskInp, const String& outname, Bool overwrite,
		const CoordinateSystem& csysTo, const IPosition& axes,
		const IPosition& shape, Bool dropdeg=False
	);

	ImageRegridder(
		const ImageTask::shCImFloat image, const String& outname,
		const ImageTask::shCImFloat templateIm, const IPosition& axes=IPosition(),
		const Record *const regionRec=0,
		const String& maskInp="", Bool overwrite=False,
		 Bool dropdeg=False, const IPosition& shape=IPosition()
	);
	// </group>

	// destructor
	~ImageRegridder();

	// perform the regrid. If <src>wantReturn</src> is True, return a pointer to the
	// collapsed image. The returned pointer is created via new(); it is the caller's
	// responsibility to delete the returned pointer. If <src>wantReturn</src> is False,
	// a NULL pointer is returned and pointer deletion is performed internally.
	std::tr1::shared_ptr<ImageInterface<Float> > regrid(Bool wantReturn) const;

	// regrid the spectral axis in velocity space rather than frequency space?
	void setSpecAsVelocity(Bool v) { _specAsVelocity = v; }

	inline String getClass() const { return _class; }

	// Set interpolation method.
	void setMethod(const String& method) { _method = Interpolate2D::stringToMethod(method); }

	void setMethod(Interpolate2D::Method method) { _method = method; }

	void setDebug(Int debug) { _debug = debug; }

	void setDoRefChange(Bool d) { _doRefChange = d; }

	void setReplicate(Bool r) { _replicate = r; }

	void setDecimate(Int d) { _decimate = d; }

	void setForceRegrid(Bool f) { _forceRegrid = f; }

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return vector<Coordinate::Type>(0);
	}

private:
	const CoordinateSystem _csysTo;
	IPosition _axes, _shape, _kludgedShape;
	Bool _dropdeg, _specAsVelocity, _doRefChange, _replicate, _forceRegrid;
	Int _debug, _decimate;
	static const String _class;
	Interpolate2D::Method _method;
	vector<String> _outputStokes;
	uInt _nReplicatedChans;

	// disallow default constructor
	ImageRegridder();

	void _finishConstruction();

	std::tr1::shared_ptr<ImageInterface<Float> > _regrid() const;

	std::tr1::shared_ptr<ImageInterface<Float> > _regridByVelocity() const;

	static Bool _doImagesOverlap(
		const ImageInterface<Float>& image0,
		const ImageInterface<Float>& image1
	);

	static Vector<std::pair<Double, Double> > _getDirectionCorners(
		const DirectionCoordinate& dc,
		const IPosition& directionShape
	);

	void _checkOutputShape(
		const SubImage<Float>& subImage,
		const std::set<Coordinate::Type>& coordsToRegrid
	) const;

	void _decimateStokes(
		std::tr1::shared_ptr<ImageInterface<Float> >& workIm
	) const;

	static Bool _doRectanglesIntersect(
		const Vector<std::pair<Double, Double> >& corners0,
		const Vector<std::pair<Double, Double> >& corners1
	);


};
}

#endif