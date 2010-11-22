/*
 * ImageReorderer.cc
 *
 *  Created on: May 7, 2010
 *      Author: dmehring
 */

#include <casa/Containers/HashMap.h>
#include <images/Images/FITSImage.h>
#include <images/Images/ImageReorderer.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/MIRIADImage.h>

namespace casa {

ImageReorderer::ImageReorderer(
	const String& imagename, const String& order, const String& outputImage
)
: _log(new LogIO), _image(0), _order(IPosition()), _outputImage(outputImage) {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_1");
	*_log << origin;
	_construct(imagename, outputImage);
	*_log << origin;
	Regex intRegex("^[0-9]+$");
	if (order.matches(intRegex)) {
		_order = _getOrder(order);
	}
	else {
		*_log << "Incorrect order specification " << order
			<< ". All characters must be digits." << LogIO::EXCEPTION;
	}
}

ImageReorderer::ImageReorderer(
	const ImageInterface<Float> * const image, const String& order, const String& outputImage
)
: _log(new LogIO), _image(image->cloneII()), _order(IPosition()), _outputImage(outputImage) {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_1");
	*_log << origin;
	_construct("", outputImage);
	*_log << origin;
	Regex intRegex("^[0-9]+$");
	if (order.matches(intRegex)) {
		_order = _getOrder(order);
	}
	else {
		*_log << "Incorrect order specification " << order
			<< ". All characters must be digits." << LogIO::EXCEPTION;
	}
}


ImageReorderer::ImageReorderer(
	const String& imagename, const Vector<String> order, const String& outputImage
)
: _log(new LogIO), _image(0), _order(IPosition()), _outputImage(outputImage) {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_2");
	*_log << origin;
	_construct(imagename, outputImage);
	*_log << origin;
	Vector<String> orderCopy = order;
	_order = _getOrder(orderCopy);
}

ImageReorderer::ImageReorderer(
	const ImageInterface<Float> * const image, const Vector<String> order, const String& outputImage
)
: _log(new LogIO), _image(image->cloneII()), _order(IPosition()), _outputImage(outputImage) {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_2");
	*_log << origin;
	_construct("", outputImage);
	*_log << origin;
	Vector<String> orderCopy = order;
	_order = _getOrder(orderCopy);
}

ImageReorderer::ImageReorderer(
	const String& imagename, uInt order, const String& outputImage
)
: _log(new LogIO), _image(0), _order(IPosition()), _outputImage(outputImage) {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_3");
	*_log << origin;
	_construct(imagename, outputImage);
	*_log << origin;
	_order = _getOrder(order);
}

ImageReorderer::ImageReorderer(
	const ImageInterface<Float> * const image, uInt order, const String& outputImage
)
: _log(new LogIO), _image(image->cloneII()), _order(IPosition()), _outputImage(outputImage) {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_3");
	*_log << origin;
	_construct("", outputImage);
	*_log << origin;
	_order = _getOrder(order);
}


ImageInterface<Float>* ImageReorderer::reorder() const {
	LogOrigin origin("ImageReorderer", __FUNCTION__);
	*_log << origin;
	// get the image data
	Array<Float> dataCopy;
	_image->get(dataCopy);
	reorderArray(dataCopy, _order);
	CoordinateSystem csys = _image->coordinates();
	CoordinateSystem newCsys = csys;
	Vector<Int> orderVector = _order.asVector();
	newCsys.transpose(orderVector, orderVector);
	IPosition shape = _image->shape();
	IPosition newShape(_order.size());
	for (uInt i=0; i<newShape.size(); i++) {
		newShape[i] = shape[_order[i]];
	}
	ImageInterface<Float>* output = 0;
	if (_outputImage.empty()) {
		output = new TempImage<Float>(TiledShape(newShape), newCsys);
	}
	else {
		output = new PagedImage<Float>(TiledShape(newShape), newCsys, _outputImage);
	}
	output->put(reorderArray(dataCopy, _order));
	if (! _outputImage.empty()) {
		output->flush();
	}
	return output;
}

ImageReorderer::~ImageReorderer() {
	delete _image;
	delete _log;
}

void ImageReorderer::_construct(const String& imagename, const String& outfile) {
	LogOrigin origin("ImageReorderer", __FUNCTION__);
	*_log << origin;
	if (_image != 0) {
		// image specified in constructor, do nothing
	}
	else if (imagename.empty()) {
		*_log << "imagename cannot be blank" << LogIO::EXCEPTION;
	}
	else {
		// Register the functions to create a FITSImage or MIRIADImage object.
		FITSImage::registerOpenFunction();
		MIRIADImage::registerOpenFunction();
		try {
			ImageUtilities::openImage(_image, imagename, *_log);
		}
		catch(AipsError err) {
			if (_image == 0) {
				*_log << "Unable to open image '" + imagename + "'" << LogIO::EXCEPTION;
			}
			RETHROW(err);
		}
	}
	if (! _outputImage.empty()) {
		File outputImageFile(_outputImage);
		switch(outputImageFile.getWriteStatus()) {
		case File::CREATABLE:
			// handle just to avoid compiler warning
			break;
		case File::OVERWRITABLE:
			// fall through to NOT_OVERWRITABLE
			// FIXME add caller specified Bool overwrite flag
		case File::NOT_OVERWRITABLE:
			*_log << "Requested output image " << _outputImage
			<< " already exists and will not be overwritten" << LogIO::EXCEPTION;
		case File::NOT_CREATABLE:
			*_log << "Requested output image " << _outputImage
			<< " cannot be created. Perhaps a permissions issue?" << LogIO::EXCEPTION;
		}
	}
}

IPosition ImageReorderer::_getOrder(uInt order) const {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_1");
	*_log << origin;
	uInt naxes = _image->ndim();
	uInt raxes = uInt(log10(order)) + 1;
	if (naxes != raxes) {
		istringstream is;
		is >> order;
		if (! String(is.str()).contains("0")) {
			raxes++;
		}
	}
	if (raxes != naxes) {
		*_log << "Image has " << naxes << " axes but " << raxes
				<< " were given for reordering. Number of axes to reorder must match the number of image axes"
				<< LogIO::EXCEPTION;
	}
	if (raxes > 10) {
		*_log << "Only images with less than 10 axes can currently be reordered. This image has "
				<< naxes << " axes" << LogIO::EXCEPTION;
	}
	IPosition myorder(naxes);
	uInt mag = 1;
	for (uInt i=1; i<myorder.size(); i++) {
		mag *= 10;
	}
	uInt scratchOrder = order;
	for (uInt i=0; i<myorder.size(); i++) {
		uInt index = scratchOrder/mag;
		if (index >= naxes) {
			*_log << "Image does not contain zero-based axis number " << index
					<< " but this was incorrectly specified in order parameter. "
					<< order << " All digits in the order parameter must be greater "
					<< "than or equal to zero and less than the number of image axes."
					<< LogIO::EXCEPTION;
		}
		for (uInt j=0; j<i; j++) {
			if (index == myorder[j]) {
				*_log << "Axis number " << index
						<< " specified multiple times in order parameter "
						<< order << " . It can only be specified once."
						<< LogIO::EXCEPTION;
			}
		}
		myorder[i] = index;
		scratchOrder -= index*mag;
		mag /= 10;
	}
	return myorder;

}

IPosition ImageReorderer::_getOrder(const String& order) const {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_2");
	*_log << origin;
	return _getOrder(String::toInt(order));
}

IPosition ImageReorderer::_getOrder(Vector<String>& order) const {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_3");
	*_log << origin;
	uInt naxes = _image->ndim();
	uInt raxes = order.size();
	if (raxes != naxes) {
		*_log << "Image has " << naxes << " axes but " << raxes
				<< " were given for reordering. Number of axes to reorder must match the number of image axes"
				<< LogIO::EXCEPTION;
	}
	Vector<String> axisNames = _image->coordinates().worldAxisNames();
	_downcase(axisNames);
	_downcase(order);
	IPosition myorder(naxes);

	Vector<String> matchMap(naxes,"");
	for (uInt i=0; i<order.size(); i++) {
		String spec = order[i];
		Regex orderRE("^" + spec + ".*");
		Vector<String> matchedNames(0);
		Vector<uInt> matchedNumbers(0);
		for (uInt j=0; j<axisNames.size(); j++) {
			if (axisNames[j].matches(orderRE)) {
				uInt oldSize = matchedNames.size();
				matchedNames.resize(oldSize + 1, True);
				matchedNames[oldSize] = axisNames[j];
				matchedNumbers.resize(oldSize + 1, True);
				matchedNumbers[oldSize] = j;
			}
		}
		if(matchedNames.size() == 0) {
			*_log << "No axis matches requested axis " << spec
				<< ". Image axis names are " << axisNames << LogIO::EXCEPTION;
		}
		else if (matchedNames.size() > 1) {
			*_log << "Multiple axes " << matchedNames << " match requested axis "
				<< spec << LogIO::EXCEPTION;
		}
		uInt axisIndex = matchedNumbers[0];
		if (matchMap[axisIndex].empty()) {
			myorder[i] = axisIndex;
			matchMap[axisIndex] = spec;
		}
		else {
			*_log << "Ambiguous axis specification. Both " << matchMap[axisIndex]
			    << " and " << spec << " match image axis name " << matchedNames[0]
			    << LogIO::EXCEPTION;
		}
	}
	*_log << "Old to new axis mapping is " << myorder << LogIO::NORMAL;
	return myorder;
}

void ImageReorderer::_downcase(Vector<String>& vec) const {
	for (Vector<String>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
		iter->downcase();
	}
}
}
