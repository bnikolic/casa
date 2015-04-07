//# RegionManager.h: framework independent class that provides 
//# functionality to tool of same name
//# Copyright (C) 2007
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

#ifndef IMAGEANALYSIS_CASACREGIONMANAGER_H
#define IMAGEANALYSIS_CASACREGIONMANAGER_H

#include <images/Regions/RegionManager.h>

namespace casa {

/**
 * image component class 
 *
 * This is a casa based class to provide the funtionality to the 
 * RegionManager Tool
 *
 * @author
 * @version 
 **/

class CasacRegionManager: public RegionManager {

public:
	const static String ALL;

	enum StokesControl {
		USE_FIRST_STOKES,
		USE_ALL_STOKES
	};

	CasacRegionManager();
	CasacRegionManager(const CoordinateSystem& csys);
	virtual ~CasacRegionManager();


	// convert to a record a region specified by a rectangular directional <src>box</src>,
	// <src>chans</src>, and <src>stokes</src>, or althernatively a pointer to
	// a region record. If box, chans, or stokes is not blank, <src>regionPtr</src> should
	// be null. Processing happens in the following order:
	// 1. if <src>box</src> is not empty it, along with <src>chans</src> and <src>stokes</src>
	// if specified, are used to determine the returned record. In this case <src>stokes</src>
	// is not altered.
	// 2. else if <src>regionPtr</src> is not null, it is used to determine the returned Record.
	// In this case, stokes may be modified to reflect the stokes parameters included in the
	// corresponding region.
	// 3. else if <src>regionName</src> is not empty, the region file of the same is read and
	// used to create the record, or if <src>regionName</src> is of the form "imagename:regionname" the region
	// record is read from the corresponding iamge. In this case, stokes may be modified
	// to reflect the stokes parameters included in the corresponding region.
	// 4. else <src>chans</src> and <src>stokes</src> are used to determine the region Record, or
	// if not given, the whole <src>imShape</src> is used.
	// <src>box</src> is specified in comma seperated quadruplets representing blc and trc pixel
	// locations, eg "100, 110, 200, 205". <src>stokes</src> is specified as the concatentation of
	// stokes parameters, eg "IQUV". <src>chans</src> is specified in ways supported by CASA, eg
	// "1~10" for channels 1 through 10.

	Record fromBCS(
		String& diagnostics, uInt& nSelectedChannels, String& stokes,
		const Record  * const regionPtr, const String& regionName,
		const String& chans, const StokesControl stokesControl,
		const String& box, const IPosition& imShape, const String& imageName=""
	);

	static vector<uInt> consolidateAndOrderRanges(
		const vector<uInt>& ranges
	);

	// <src>nChannels</src> is the total number of spectral channels in the image
	vector<uInt> setSpectralRanges(
		String specification, uInt& nSelectedChannels, const uInt nChannels
	) const;

private:

	String _pairsToString(const vector<uInt>& pairs) const;

	vector<uInt> _setPolarizationRanges(
		String& specification, const String& firstStokes, const uInt nStokes,
		const StokesControl stokesControl
	) const;

	vector<Double> _setBoxCorners(const String& box) const;

	ImageRegion _fromBCS(
			String& diagnostics, uInt& nSelectedChannels, String& stokes,
			const String& chans, const StokesControl stokesControl,
			const String& box, const IPosition& imShape
	) const;

	ImageRegion _fromBCS(
			String& diagnostics,
			const vector<Double>& boxCorners, const vector<uInt>& chanEndPts,
			const vector<uInt>& polEndPts, const IPosition imShape
	) const;

	static void _setRegion(
			Record& regionRecord, String& diagnostics,
			const Record* regionPtr
	);

	String _stokesFromRecord(
			const Record& region, const StokesControl stokesControl, const IPosition& shape
	) const;

	void _setRegion(
			Record& regionRecord, String& diagnostics,
			const String& regionName, const IPosition& imShape,
			const String& imageName
	);
};

} // casa namespace
#endif
