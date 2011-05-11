//# DSWorldPoly.cc : Implementation of a world coords DSPoly
//# Copyright (C) 1998,1999,2000,2001,2002
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
//# $Id: 

#include <display/DisplayShapes/DSWorldPoly.h>
#include <display/DisplayShapes/DSPixelPoly.h>
#include <display/DisplayShapes/DSScreenPoly.h>

#include <display/DisplayShapes/DSWorldPolyLine.h>

#include <casa/Quanta/UnitMap.h>
#include <casa/Containers/List.h>
#include <display/Display/WorldCanvas.h>
#include <casa/Arrays/ArrayMath.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
namespace casa { //# NAMESPACE CASA - BEGIN

DSWorldPoly::DSWorldPoly() :
  DSPoly(),
  itsPD(0),
  itsWC(0),
  itsWorldXPoints(0),
  itsWorldYPoints(0) {
  
  // Default cons. We know nothing about anything.
  
}

DSWorldPoly::DSWorldPoly(const Record& settings, PanelDisplay* pd) :
  DSPoly(),
  itsPD(pd),
  itsWC(0),
  itsWorldXPoints(0),
  itsWorldYPoints(0) { 
  
  setOptions(settings);
}

DSWorldPoly::DSWorldPoly(DSScreenPoly& other, PanelDisplay* pd) :
  DSPoly(),
  itsPD(pd),
  itsWC(0),
  itsWorldXPoints(0),
  itsWorldYPoints(0) {
  
  Record shapeSettings = other.getRawOptions();
  DSPoly::setOptions(shapeSettings);
  updateWCoords();
  
}

DSWorldPoly::DSWorldPoly(DSPixelPoly& other, PanelDisplay* pd) :
  DSPoly(),
  itsPD(pd),
  itsWC(0),
  itsWorldXPoints(0),
  itsWorldYPoints(0) {
  
  Record shapeSettings = other.getRawOptions();
  DSPoly::setOptions(shapeSettings);
  updateWCoords();
}

DSWorldPoly::DSWorldPoly(DSWorldPolyLine& other) :
  DSPoly(),
  itsPD(other.panelDisplay()) ,
  itsWC(0),
  itsWorldXPoints(0),
  itsWorldYPoints(0)
{

  Record otherOptions = other.getOptions();
  if (otherOptions.isDefined("polylinepoints")) {
    Record points = otherOptions.subRecord("polylinepoints");
    otherOptions.removeField("polylinepoints");
    otherOptions.defineRecord("polygonpoints", points);
  }
  
  setOptions(otherOptions);
  updateWCoords();
}

DSWorldPoly::~DSWorldPoly() {

}

void DSWorldPoly::recalculateScreenPosition() {
  if (!itsWC) {
    Matrix<Float> points = getPoints();
    if (points.nelements()) {

      // Slightly dodgy... but probably more appropriate than a throw.
      // We will be using "old" screen pos i.e. before a "recalculation"
      // to find our WC.

      itsWC = chooseWCFromPixPoints(points, itsPD);
      if (!itsWC) {
	throw(AipsError("Couldn't recalculate screen pos of polygon "
			"as I couldn't find a valid WC to use"));
      }
    } else {
      return;
    }
  }
  
  Matrix<Float> pointsToSet(itsWorldXPoints.nelements(), 2);

  // TODO Units here..

  for (uInt i=0; i<itsWorldXPoints.nelements(); i++) {
    Vector<Double> worldPoint(2);
    Vector<Double> pixPoint(2);
    worldPoint(0) = itsWorldXPoints(i).getValue();
    worldPoint(1) = itsWorldYPoints(i).getValue();
    itsWC->worldToPix(pixPoint, worldPoint);
    pointsToSet(i,0) = Float(pixPoint(0));
    pointsToSet(i,1) = Float(pixPoint(1));
  }
  DSPoly::setPoints(pointsToSet);
}

void DSWorldPoly::draw(PixelCanvas* pc) {


  if (itsWC && getPoints().nelements()) {
    Matrix<Float> pnts = getPoints();
    if (inWorldCanvasDrawArea(pnts, itsWC)) {
      DSPoly::draw(pc);
    }
  }

}


Bool DSWorldPoly::setOptions(const Record& settings) {
  Bool localChange = False;
  Record toSet = settings;
  
  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "world") {
      throw(AipsError("I (DSWorldPoly) was expecting an option record which"
		      " had coords == \'world\'. Please use a \'lock\' or"
		      " \'revert\' function"
		      " to change my co-ord system"));
    }
  }
  
  if (settings.isDefined("polygonpoints")) {
    if (!itsWC) {
      itsWC = chooseWCFromWorldPoints(settings, "polygonpoints", itsPD);
      if (!itsWC) {
	throw(AipsError("Couldn't find a suitable world canvas on which "
			"to put the Polygon"));
      }
    }
    
    // TODO fix this
    matrixFloatFromQuant(toSet, "polygonpoints", "?");

    Matrix<Float> worldPoints = toSet.asArrayFloat("polygonpoints");
    Matrix<Float> pixelPoints(worldPoints.nrow(), worldPoints.ncolumn());
    
    for (uInt i=0 ; i < worldPoints.nrow() ; i ++) {
      Vector<Double> wrld(2);
      Vector<Double> pix(2);
      wrld(0) = Double(worldPoints(i, 0));
      wrld(1) = Double(worldPoints(i, 1));
      itsWC->worldToPix(pix, wrld);

      pixelPoints(i,0) = Float(pix(0));
      pixelPoints(i,1) = Float(pix(1));
    }
    toSet.removeField("polygonpoints");
    toSet.define("polygonpoints", pixelPoints);
    
  }
  

  if (DSPoly::setOptions(toSet)) {
    localChange = True;
    updateWCoords();
  }
  
  return localChange;
}

Record DSWorldPoly::getOptions() {
  Record toReturn;
  toReturn = DSPoly::getOptions();
  
  if (toReturn.isDefined("polygonpoints")) {
    if(toReturn.dataType("polygonpoints") != TpArrayFloat) {
      throw(AipsError("Bad data type returned for field \'polygonpoints\'"));
    }

    if (!itsWC) {
      itsWC = chooseWCFromPixPoints(toReturn.asArrayFloat("polygonpoints"),
				    itsPD);
      
      if (!itsWC) {
	throw(AipsError("DSWorldPoly couldn't convert to world  position "
			"since "
			"it couldn't find a valid worldcanvas"));
      }
      
    }
    
    // -> World options.
    Vector<String> units = itsWC->coordinateSystem().worldAxisUnits();
    String encodeTo = units(0);

    for (uInt i=0; i < units.nelements(); i++) {
      if (units(i) != encodeTo) {
	throw(AipsError("Not sure yet how to deal with the sitatuion that "
			"arose in DSWorldPoly.cc #235!?!"));
      }
    }
    
    matrixFloatToQuant(toReturn, "polygonpoints", encodeTo);

  }

  // Shouldn't happen (should never be defined) .. but why not
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }
  
  toReturn.define("coords", "world");
  return toReturn;
}



void DSWorldPoly::updateWCoords() {
  Matrix<Float> pixelPoints = getPoints();
  
  if (pixelPoints.nelements()) {

    if (!itsWC) {
      itsWC = chooseWCFromPixPoints(pixelPoints, itsPD);
      if (!itsWC) {
	throw(AipsError("Couldn't update world coordinates of polygon "
			"as I couldn't find a valid WC to use"));
      }
    }
    // Ok, valid itsWC and points
    
    Vector<String> units = itsWC->coordinateSystem().worldAxisUnits();
    String encodeTo = units(0);
    
    for (uInt i=0; i < units.nelements(); i++) {
      if (units(i) != encodeTo) {
	throw(AipsError("Not sure *yet* how to deal with the sitatuion that "
			"arose in DSWorldPoly.cc #199!?!"));
      }
    }
    
    itsWorldXPoints.resize(pixelPoints.nrow());
    itsWorldYPoints.resize(pixelPoints.nrow());
    
    for (uInt i=0; i < pixelPoints.nrow() ; i ++) {
      Vector<Double> pixPoint(2);
      Vector<Double> worldPoint(2);
      pixPoint(0) = Double(pixelPoints(i,0));
      pixPoint(1) = Double(pixelPoints(i,1));
      itsWC->pixToWorld(worldPoint, pixPoint);
      
      itsWorldXPoints(i) = Quantity(worldPoint(0), encodeTo);
      itsWorldYPoints(i) = Quantity(worldPoint(1), encodeTo);
    }

  }
}

void DSWorldPoly::move(const Float& dX, const Float& dY) {
  DSPoly::move(dX, dY);
  updateWCoords();
}

void DSWorldPoly::setCenter(const Float& xPos, const Float& yPos) {
  DSPoly::setCenter(xPos, yPos);
  updateWCoords();
}

void DSWorldPoly::rotate(const Float& angle) {
  DSPoly::rotate(angle);
  updateWCoords();
}

void DSWorldPoly::scale(const Float& scaleFactor) {
  DSPoly::scale(scaleFactor);
  updateWCoords();
}

void DSWorldPoly::addPoint(const Vector<Float>& newPos) {
  DSPoly::addPoint(newPos);
  updateWCoords();
}

void DSWorldPoly::setPoints(const Matrix<Float>& points) {
  DSPoly::setPoints(points);
  updateWCoords();
}

void DSWorldPoly::changePoint(const Vector<Float>&pos, const Int n) {
  DSPoly::changePoint(pos, n);
  updateWCoords();
}

void DSWorldPoly::changePoint(const Vector<Float>& pos) {
  DSPoly::changePoint(pos);
  updateWCoords();
}





} //# NAMESPACE CASA - END

