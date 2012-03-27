//# ComponentShape.h: Base class for component shapes
//# Copyright (C) 1998,1999,2000,2001
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
//# $Id: ComponentShape.h 20739 2009-09-29 01:15:15Z Malte.Marquarding $

#ifndef ANNOTATIONS_ANNOTATIONBASE_H
#define ANNOTATIONS_ANNOTATIONBASE_H

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <boost/regex.hpp>

namespace casa {

// <summary>Base class for annotations</summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd">
// </reviewed>

// <synopsis>
// Base class for annotations

// In order to minimize maintainability, many parameters are not
// set in the constructor but can be set by mutator methods.
//
// Input directions will be converted to the reference frame of the
// input coordinate system upon construction if necessary.
// </synopsis>

class AnnotationBase {
public:

	typedef vector<float> RGB;

	typedef Vector<std::pair<Quantity,Quantity> > Direction;

	enum Type {
		// annotations only
		LINE,
		VECTOR,
		TEXT,
		SYMBOL,
		// regions
		RECT_BOX,
		CENTER_BOX,
		ROTATED_BOX,
		POLYGON,
		CIRCLE,
		ANNULUS,
		ELLIPSE
	};

	enum Keyword {
		COORD,
		RANGE,
		FRAME,
		CORR,
		VELTYPE,
		RESTFREQ,
		LINEWIDTH,
		LINESTYLE,
		SYMSIZE,
		SYMTHICK,
		COLOR,
		FONT,
		FONTSIZE,
		FONTSTYLE,
		USETEX,
		LABEL,
		LABELCOLOR,
		LABELPOS,
		LABELOFF,
		UNKNOWN_KEYWORD,
		N_KEYS
	};

	enum LineStyle {
		SOLID,
		DASHED,
		DOT_DASHED,
		DOTTED
	};

	enum FontStyle {
		NORMAL,
		BOLD,
		ITALIC,
		ITALIC_BOLD
	};

    static const RGB BLACK;
    static const RGB BLUE;
    static const RGB CYAN;
    static const RGB GRAY;
    static const RGB GREEN;
    static const RGB MAGENTA;
    static const RGB ORANGE;
    static const RGB RED;
    static const RGB WHITE;
    static const RGB YELLOW;

	static const String DEFAULT_LABEL;
	static const RGB DEFAULT_COLOR;
	static const LineStyle DEFAULT_LINESTYLE;
	static const uInt DEFAULT_LINEWIDTH;
	static const uInt DEFAULT_SYMBOLSIZE;
	static const uInt DEFAULT_SYMBOLTHICKNESS;
	static const String DEFAULT_FONT;
	static const uInt DEFAULT_FONTSIZE;
	static const FontStyle DEFAULT_FONTSTYLE;
	static const Bool DEFAULT_USETEX;
	static const RGB DEFAULT_LABELCOLOR;
	static const String DEFAULT_LABELPOS;
	static const vector<Int> DEFAULT_LABELOFF;

	static const boost::regex rgbHexRegex;

	virtual ~AnnotationBase();

	Type getType() const;

	static LineStyle lineStyleFromString(const String& ls);

	// Given a string, return the corresponding annotation type or throw
	// an error if the string does not correspond to an allowed type.
	static Type typeFromString(const String& type);

	static String typeToString(const Type type);

	static String keywordToString(const Keyword key);

	static String lineStyleToString(const LineStyle linestyle);

	static FontStyle fontStyleFromString(const String& fs);

	static String fontStyleToString(const FontStyle fs);

	void setLabel(const String& label);

	String getLabel() const;

    // <src>color</src> must either be a recognized color name or
    // a valid rgb hex string, else an expection is thrown
	void setColor(const String& color);
	
    // color must have three elements all with values between 0 and 255 inclusive
    // or an exception is thrown.
    void setColor(const RGB& color);

    // returns the color name if it is recognized or its rgb hex string 
	String getColorString() const;

	static String colorToString(const RGB& color);

    // get the color associated with this object
    RGB getColor() const;

	void setLineStyle(const LineStyle lineStyle);

	LineStyle getLineStyle() const;

	void setLineWidth(const uInt linewidth);

	uInt getLineWidth() const;

	void setSymbolSize(const uInt symbolsize);

	uInt getSymbolSize() const;

	void setSymbolThickness(const uInt symbolthickness);

	uInt getSymbolThickness() const;

	void setFont(const String& font);

	String getFont() const;

	void setFontSize(const uInt fontsize);

	uInt getFontSize() const;

	void setFontStyle(const FontStyle& fontstyle);

	FontStyle getFontStyle() const;

	void setUseTex(const Bool usetex);

	Bool isUseTex() const;

	// is the object a region?
	virtual Bool isRegion() const;

	// is the object only an annotation? Can only be false if the object
	// is a region
	inline virtual Bool isAnnotationOnly() const { return True; }

	// set "pix" as valid unit. This should be called externally
	// before creating quantities which have pixel units.
	static void unitInit();

    // <src>color</src> must either be a recognized color name or
    // a valid rgb hex string, else an expection is thrown
	void setLabelColor(const String& color);

    // color must have three elements all with values between 0 and 255 inclusive
    // or an exception is thrown.
    void setLabelColor(const RGB& color);

    // returns the color name if it is recognized or its rgb hex string

	String getLabelColorString() const;

    // get the color associated with this object's label
    RGB getLabelColor() const;

    // returns one of top, bottom, left, or right.
	String getLabelPosition() const;

	// <src>position</src> must have a value in top, bottom, left, or right.
	// case is ignored.
	void setLabelPosition(const String& position);

	// <src>offset</src> must have two elements
	void setLabelOffset(const vector<Int>& offset);

	vector<Int> getlabelOffset() const;

	virtual ostream& print(ostream &os) const = 0;

	// These parameters are included at the global scope. Multiple runs
	// on the same object are cumulative; if a key exists in the current
	// settings but not in <src>globalKeys</src> that key will still exist
	// in the globals after setGlobals has run.
	void setGlobals(const Vector<Keyword>& globalKeys);

	// print a set of keyword value pairs
	static ostream& print(
		ostream& os, const map<Keyword, String>& params
	);

	// print a line style representation
	static ostream& print(
		ostream& os, const LineStyle ls
	);

	// print a font style representation
	static ostream& print(
		ostream& os, const FontStyle fs
	);

	// Get a list of the user-friendly color names supported
	static std::list<std::string> colorChoices();

	// get the coordinate system associated with this object.
	// This is the same coordinates system used to construct the object.

	inline const CoordinateSystem& getCsys() const {
		return _csys;
	}

	inline Direction getDirections() const {
		return _convertedDQs;
	}

protected:

	AnnotationBase(
		const Type type, const String& dirRefFrameString,
		const CoordinateSystem& csys
	);

	// use only if the frame of the input directions is the
	// same as the frame of the coordinate system
	AnnotationBase(
		const Type type, const CoordinateSystem& csys
	);

	// the implicitly defined copy constructor is fine
	// AnnotationBase(const AnnotationBase& other);

	// assignment operator
	AnnotationBase& operator= (const AnnotationBase& other);

	static void _checkMixed(
		const String& origin,
		const Direction& dirs
	);

	MDirection _directionFromQuantities(
		const Quantity& q0, const Quantity& q1
	);

	void _checkAndConvertDirections(
		const String& origin,
		const Direction& dirs
	);

	virtual void _printPairs(ostream &os) const;

	inline const Vector<MDirection>& _getConvertedDirections() const {
		return _convertedDirections;
	}

	inline const IPosition& _getDirectionAxes() const {
		return _directionAxes;
	}

private:
	Type _type;
	MDirection::Types _directionRefFrame;
	CoordinateSystem _csys;
	IPosition _directionAxes;
	String _label, _font, _labelPos;
    RGB _color, _labelColor;
	FontStyle _fontstyle;
	LineStyle _linestyle;
	uInt _fontsize, _linewidth, _symbolsize,
		_symbolthickness;
	Bool _usetex;
	Vector<MDirection> _convertedDirections;

	map<Keyword, Bool> _globals;
	map<Keyword, String> _params;
	Bool _printGlobals;
	vector<Int> _labelOff;
    Direction _convertedDQs;

	static Bool _doneUnitInit, _doneColorInit;
	static map<String, LineStyle> _lineStyleMap;
	static map<String, Type> _typeMap;
	static map<string, RGB> _colors;
	static map<RGB, string> _rgbNameMap;
	static std::list<std::string> _colorNames;

	const static String _class;

	void _init();
	void _initParams();

	static void _initColors();

	static RGB _colorStringToRGB(const String& s);

	static Bool _isRGB(const RGB& rgb);

	void _testConvertToPixel() const;

	static void _initTypeMap();
};

inline ostream &operator<<(ostream& os, const AnnotationBase& annotation) {
	return annotation.print(os);
};

inline ostream &operator<<(ostream& os, const AnnotationBase::LineStyle& ls) {
	return AnnotationBase::print(os, ls);
};

inline ostream &operator<<(ostream& os, const AnnotationBase::FontStyle& fs) {
	return AnnotationBase::print(os, fs);
};

inline ostream &operator<<(ostream& os, const map<AnnotationBase::Keyword, String>& x) {
	return AnnotationBase::print(os, x);
};

// Just need a identifiable exception class for exception handling.
class WorldToPixelConversionError : public AipsError {
public:
	WorldToPixelConversionError(String msg) : AipsError(msg) {}
};

}

#endif
