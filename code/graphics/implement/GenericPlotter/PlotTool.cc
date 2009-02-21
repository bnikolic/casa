//# PlotTool.cc: Tool class definitions (higher-level event handlers).
//# Copyright (C) 2008
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
#include <graphics/GenericPlotter/PlotTool.h>
#include <graphics/GenericPlotter/PlotFactory.h>

#include <casa/OS/Time.h>

#include <iomanip>

namespace casa {

//////////////////////////
// PLOTTOOL DEFINITIONS //
//////////////////////////

PlotTool::PlotTool(PlotCoordinate::System sys): m_canvas(NULL),m_factory(NULL),
        m_active(false), m_blocking(true), m_xAxis(X_BOTTOM), m_yAxis(Y_LEFT),
        m_coordSystem(sys), m_lastEventHandled(false) { }

PlotTool::PlotTool(PlotAxis xAxis, PlotAxis yAxis, PlotCoordinate::System sys):
        m_canvas(NULL), m_factory(NULL), m_active(false), m_blocking(true),
        m_xAxis(xAxis), m_yAxis(yAxis), m_coordSystem(sys),
        m_lastEventHandled(false) { }

PlotTool::~PlotTool() { if(m_factory != NULL) delete m_factory; }


bool PlotTool::isActive() const { return m_active; }
void PlotTool::setActive(bool isActive) { m_active = isActive; }
bool PlotTool::isBlocking() const { return m_blocking; }
void PlotTool::setBlocking(bool blocking) { m_blocking = blocking; }
PlotAxis PlotTool::getXAxis() const { return m_xAxis; }
PlotAxis PlotTool::getYAxis() const { return m_yAxis; }
PlotCoordinate::System PlotTool::getCoordinateSystem() const {
    return m_coordSystem; }
bool PlotTool::lastEventWasHandled() const { return m_lastEventHandled; }


PlotCanvas* PlotTool::canvas() const { return m_canvas; }
bool PlotTool::isAttached() const { return m_canvas != NULL; }

void PlotTool::attach(PlotCanvas* canvas) {
    if(m_canvas != NULL) detach();
    m_canvas = canvas;
    if(m_canvas != NULL) {
        // only generate a new factory if needed
        if(m_factory == NULL) m_factory = m_canvas->implementationFactory();
        else if(m_factory->implementation() != m_canvas->implementation()){
            delete m_factory;
            m_factory = m_canvas->implementationFactory();
        }
    }
}

void PlotTool::detach() { m_canvas = NULL; }


////////////////////////////////
// PLOTSELECTTOOL DEFINITIONS //
////////////////////////////////

PlotSelectTool::PlotSelectTool(PlotCoordinate::System system) :
        PlotMouseTool(system), m_drawRects(false) { }

PlotSelectTool::PlotSelectTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system) : PlotMouseTool(xAxis, yAxis, system),
        m_drawRects(false) { }

PlotSelectTool::~PlotSelectTool() { }

void PlotSelectTool::addNotifier(PlotSelectToolNotifier* notifier) {
    if(notifier == NULL) return;
    for(unsigned int i = 0; i < m_notifiers.size(); i++)
        if(m_notifiers[i] == notifier) return;
    m_notifiers.push_back(notifier);
}

void PlotSelectTool::setSelectLine(PlotLinePtr line) {
    if(line != m_selLine) {
        m_selLine = line;
        if(m_canvas != NULL) {
            if(!line.null()) m_canvas->setSelectLine(line);
            else             m_canvas->setSelectLineShown(false);
        }
    }
}

void PlotSelectTool::setSelectLine(bool on) {
    if(m_canvas != NULL) m_canvas->setSelectLineShown(on); }

void PlotSelectTool::setDrawRects(bool on) {
    if(on != m_drawRects) {
        if(m_canvas != NULL && m_rects.size() > 0) {
            for(unsigned int i = 0; i < m_rects.size(); i++) {
                if(on) m_canvas->drawShape(m_rects[i]);
                else   m_canvas->removeShape(m_rects[i]);
            }
        }
        m_drawRects = on;
    }
}

void PlotSelectTool::setRectLine(PlotLinePtr line) {
    if(line != m_rectLine) {
        m_rectLine = line;
        for(unsigned int i = 0; i < m_rects.size(); i++)
            m_rects[i]->setLine(line);
    }
}

void PlotSelectTool::setRectFill(PlotAreaFillPtr fill) {
    if(fill != m_rectFill) {
        m_rectFill = fill;
        for(unsigned int i = 0; i < m_rects.size(); i++)
            m_rects[i]->setAreaFill(fill);
    }
}

unsigned int PlotSelectTool::numSelectedRects() const{ return m_rects.size(); }

void PlotSelectTool::getSelectedRects(vector<double>& upperLeftXs,
        vector<double>& upperLeftYs, vector<double>& lowerRightXs,
        vector<double>& lowerRightYs, PlotCoordinate::System system) const {
    vector<PlotRegion> v = getSelectedRects(system);
    upperLeftXs.resize(v.size());  upperLeftYs.resize(v.size());
    lowerRightXs.resize(v.size()); lowerRightYs.resize(v.size());
    for(unsigned int i = 0; i < v.size(); i++) {
        upperLeftXs[i]  = v[i].upperLeft().x();
        upperLeftYs[i]  = v[i].upperLeft().y();
        lowerRightXs[i] = v[i].lowerRight().x();
        lowerRightYs[i] = v[i].lowerRight().y();
    }
}

vector<PlotRegion>
PlotSelectTool::getSelectedRects(PlotCoordinate::System system) const {
    vector<PlotRegion> v(m_rects.size());
    vector<PlotCoordinate> coords;
    for(unsigned int i = 0; i < v.size(); i++) {
        coords = m_rects[i]->coordinates();
        // WARNING: assumption that the PlotRectangle implementation behaves
        // correctly.
        if(coords[0].system() != system && m_canvas != NULL)
            coords[0] = m_canvas->convertCoordinate(coords[0], system);
        if(coords[1].system() != system && m_canvas != NULL)
            coords[1] = m_canvas->convertCoordinate(coords[1], system);
        v[i] = PlotRegion(coords[0], coords[1]);
    }
    return v;
}

void PlotSelectTool::clearSelectedRects() {
    if(m_drawRects && m_canvas != NULL && m_rects.size() > 0) {
        vector<PlotItemPtr> v(m_rects.size());
        for(unsigned int i = 0; i < v.size(); i++) v[i] = m_rects[i];
        m_canvas->removePlotItems(v);
    }
    m_rects.clear();
}

void PlotSelectTool::setActive(bool active) {
    if(active != m_active) {
        PlotMouseTool::setActive(active);
        if(m_canvas != NULL) {
            if(active && !m_selLine.null()) m_canvas->setSelectLine(m_selLine);
            else                          m_canvas->setSelectLineShown(active);
            if(!active) m_canvas->setCursor(NORMAL_CURSOR);
        }
    }
}

void PlotSelectTool::handleMouseEvent(const PlotEvent& event) {
    m_lastEventHandled = false;
    if(m_canvas == NULL || !m_active) return; // shouldn't happen
    
    // cursor nice-ness
    const PlotMousePressEvent* mp; const PlotMouseReleaseEvent* mr;
    if((mp = dynamic_cast<const PlotMousePressEvent*>(&event)) != NULL)
        m_canvas->setCursor(CROSSHAIR);
    else if((mr = dynamic_cast<const PlotMouseReleaseEvent*>(&event)) != NULL)
        m_canvas->setCursor(NORMAL_CURSOR);
    
    // for now we only care about select events, assuming that the canvas has
    // done its selection-pen-drawing duty
    const PlotSelectEvent* select=dynamic_cast<const PlotSelectEvent*>(&event);
    if(select != NULL && m_canvas != NULL) {
        PlotRegion r = select->region();
        if(r.upperLeft().system() != m_coordSystem ||
           r.lowerRight().system() != m_coordSystem)
            r = m_canvas->convertRegion(r, m_coordSystem);
        PlotShapeRectanglePtr rect = m_factory->shapeRectangle(r.upperLeft(),
                r.lowerRight());
        rect->setLine(m_rectLine);
        rect->setAreaFill(m_rectFill);
        if(m_drawRects) m_canvas->plotItem(rect, ANNOTATION);
        m_rects.push_back(rect);
        m_lastEventHandled = true;
        
        for(unsigned int i = 0; i < m_notifiers.size(); i++)
            m_notifiers[i]->notifySelectionAdded(*this);
    }
}

void PlotSelectTool::attach(PlotCanvas* canvas) {
    PlotMouseTool::attach(canvas);
    if(canvas != NULL && m_active) {
        for(unsigned int i = 0; i < m_rects.size(); i++)
            canvas->drawShape(m_rects[i]);
        if(!m_selLine.null()) m_canvas->setSelectLine(m_selLine);
        else                  m_canvas->setSelectLineShown(true);
    }
}

void PlotSelectTool::detach() {
    if(m_canvas != NULL && m_drawRects && m_rects.size() > 0) {
        for(unsigned int i = 0; i < m_rects.size(); i++)
            m_canvas->removeShape(m_rects[i]);
    }
    if(m_canvas != NULL) {
        m_canvas->setCursor(NORMAL_CURSOR);
        m_canvas->setSelectLineShown(false);
    }
    PlotMouseTool::detach();
}


//////////////////////////////
// PLOTZOOMTOOL DEFINITIONS //
//////////////////////////////

PlotZoomTool::PlotZoomTool(PlotCoordinate::System sys) : PlotMouseTool(sys),
        m_stack(NULL) { }

PlotZoomTool::PlotZoomTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System sys) : PlotMouseTool(xAxis, yAxis, sys),
        m_stack(NULL) { }

PlotZoomTool::~PlotZoomTool() { }

void PlotZoomTool::addNotifier(PlotZoomToolNotifier* notifier) {
    if(notifier == NULL) return;
    for(unsigned int i = 0; i < m_notifiers.size(); i++)
        if(m_notifiers[i] == notifier) return;
    m_notifiers.push_back(notifier);
}

void PlotZoomTool::setSelectLine(PlotLinePtr line) {
    if(line != m_selLine) {
        m_selLine = line;
        if(m_canvas != NULL) {
            if(!line.null()) m_canvas->setSelectLine(line);
            else             m_canvas->setSelectLineShown(false);
        }
    }
}

void PlotZoomTool::setSelectLine(bool on) {
    if(m_canvas != NULL && m_active) m_canvas->setSelectLineShown(on); }

vector<PlotRegion> PlotZoomTool::getZoomStack(PlotCoordinate::System s) const {
    if(m_stack == NULL) return vector<PlotRegion>();
    
    vector<PlotRegion> stack = m_stack->stack();
    if(s == m_coordSystem || m_stack->size() == 0 || m_canvas == NULL)
        return stack;
    
    vector<PlotRegion> v(stack.size());
    for(unsigned int i = 0; i < v.size(); i++)
        v[i] = m_canvas->convertRegion(stack[i], s);    
    return v;
}

unsigned int PlotZoomTool::getStackIndex() const {
    if(m_stack == NULL) return 0;
    else                return m_stack->stackIndex();
}

void PlotZoomTool::setActive(bool active) {
    if(active != m_active) {
        PlotMouseTool::setActive(active);
        if(m_canvas != NULL) {
            if(active && !m_selLine.null()) m_canvas->setSelectLine(m_selLine);
            else                          m_canvas->setSelectLineShown(active);
            if(!active) m_canvas->setCursor(NORMAL_CURSOR);
            if(active && m_stack->size() <= 1)
                m_stack->setBase(m_canvas->axesRanges(m_xAxis, m_yAxis),
                                 m_xAxis, m_yAxis);
        }
    }
}

void PlotZoomTool::handleMouseEvent(const PlotEvent& event) {
    m_lastEventHandled = false;
    if(m_canvas == NULL || !m_active) return;
    
    // cursor nice-ness
    const PlotMousePressEvent* mp; const PlotMouseReleaseEvent* mr;
    if((mp = dynamic_cast<const PlotMousePressEvent*>(&event)) != NULL)
        m_canvas->setCursor(CROSSHAIR);
    else if((mr = dynamic_cast<const PlotMouseReleaseEvent*>(&event)) != NULL)
        m_canvas->setCursor(NORMAL_CURSOR);
    
    const PlotSelectEvent* s; const PlotWheelEvent* w; const PlotClickEvent* c;
    
    // on select: zoom to area
    if((s = dynamic_cast<const PlotSelectEvent*>(&event)) != NULL) {
        PlotRegion r = s->region();
        if(r.upperLeft().system() != m_coordSystem ||
           r.lowerRight().system() != m_coordSystem)
            r = m_canvas->convertRegion(r, m_coordSystem);

        m_stack->addRegion(r, m_xAxis, m_yAxis);
        if(m_coordSystem != PlotCoordinate::WORLD)
            r = m_canvas->convertRegion(r, PlotCoordinate::WORLD);
        m_canvas->setAxesRegion(r, m_xAxis, m_yAxis);
        m_lastEventHandled = true;
        
    // on wheel go through zoom stack 
    } else if((w = dynamic_cast<const PlotWheelEvent*>(&event)) != NULL) {
        m_canvas->canvasAxesStackMove(w->delta());
        m_lastEventHandled = true;
        
    // go to base on right click, zoom in 50% centered on double click
    } else if((c = dynamic_cast<const PlotClickEvent*>(&event)) != NULL) {
        if(c->button() == PlotClickEvent::CONTEXT) {
            m_canvas->canvasAxesStackMove(0);
            m_lastEventHandled = true;
            
        } else if(c->button() == PlotClickEvent::DOUBLE) {
            pair<double, double> xRange = m_canvas->axisRange(m_xAxis),
                                 yRange = m_canvas->axisRange(m_yAxis);
            double xDelta = (xRange.second - xRange.first) / 4,
                   yDelta = (yRange.second - yRange.first) / 4;
            PlotCoordinate coord = c->where();
            if(coord.system() != PlotCoordinate::WORLD)
                coord=m_canvas->convertCoordinate(coord,PlotCoordinate::WORLD);

            PlotRegion r(PlotCoordinate(coord.x()- xDelta, coord.y()+ yDelta),
                         PlotCoordinate(coord.x()+ xDelta, coord.y()- yDelta));
            if(m_coordSystem != PlotCoordinate::WORLD)
                m_stack->addRegion(m_canvas->convertRegion(r, m_coordSystem),
                                  m_xAxis, m_yAxis);
            else m_stack->addRegion(r, m_xAxis, m_yAxis);
            m_canvas->setAxesRegion(r, m_xAxis, m_yAxis);
            m_lastEventHandled = true;
        }
    }
    
    if(m_lastEventHandled) notifyWatchers();
}

void PlotZoomTool::reset() {
    if(m_stack != NULL) m_stack->clearStack(true);
}


void PlotZoomTool::attach(PlotCanvas* canvas) {
    PlotMouseTool::attach(canvas);
    if(canvas != NULL) m_stack = &canvas->canvasAxesStack();
    if(canvas != NULL && m_active) {
        if(!m_selLine.null()) m_canvas->setSelectLine(m_selLine);
        else                  m_canvas->setSelectLineShown(false);
        if(m_stack->size() == 0)
            m_stack->setBase(m_canvas->axesRanges(m_xAxis, m_yAxis),
                             m_xAxis, m_yAxis);
    }
}

void PlotZoomTool::detach() {
    if(m_canvas != NULL) {
        m_canvas->setCursor(NORMAL_CURSOR);
        m_canvas->setSelectLineShown(false);
    }
    m_stack = NULL;
    PlotMouseTool::detach();
}

void PlotZoomTool::notifyWatchers() {
    for(unsigned int i = 0; i < m_notifiers.size(); i++)
        m_notifiers[i]->notifyZoomChanged(*this);
}


/////////////////////////////
// PLOTPANTOOL DEFINITIONS //
/////////////////////////////

PlotPanTool::PlotPanTool(PlotCoordinate::System sys) : PlotMouseTool(sys),
        m_inDraggingMode(false), m_stack(NULL) { }

PlotPanTool::PlotPanTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System sys) : PlotMouseTool(xAxis, yAxis, sys),
        m_inDraggingMode(false), m_stack(NULL) { }

PlotPanTool::~PlotPanTool() { }

void PlotPanTool::addNotifier(PlotPanToolNotifier* notifier) {
    if(notifier == NULL) return;
    for(unsigned int i = 0; i < m_notifiers.size(); i++)
        if(m_notifiers[i] == notifier) return;
    m_notifiers.push_back(notifier);
}

vector<PlotRegion> PlotPanTool::getPanStack(PlotCoordinate::System s) const {
    if(m_stack == NULL) return vector<PlotRegion>();
    
    vector<PlotRegion> stack = m_stack->stack();
    if(s == m_coordSystem || m_stack->size() == 0 || m_canvas == NULL)
        return stack;
    
    vector<PlotRegion> v(stack.size());
    for(unsigned int i = 0; i < v.size(); i++)
        v[i] = m_canvas->convertRegion(stack[i], s);    
    return v;
}

unsigned int PlotPanTool::getStackIndex() const {
    if(m_stack == NULL) return 0;
    else                return m_stack->stackIndex();
}

void PlotPanTool::setActive(bool active) {
    if(active != m_active) {
        PlotMouseTool::setActive(active);
        if(m_canvas != NULL) {
            m_canvas->setCursor(active ? HAND_OPEN : NORMAL_CURSOR);
            if(active && m_stack->size() == 0)
                m_stack->setBase(m_canvas->axesRanges(m_xAxis, m_yAxis),
                                 m_xAxis, m_yAxis);
        }
    }
}

void PlotPanTool::handleMouseEvent(const PlotEvent& event) {
    m_lastEventHandled = false;
    if(m_canvas == NULL) return;
    
    const PlotMousePressEvent* mp; const PlotMouseReleaseEvent* mr;
    const PlotMouseDragEvent* mm; const PlotWheelEvent* w;
    const PlotClickEvent* c;
    
    // dragging
    if((mp = dynamic_cast<const PlotMousePressEvent*>(&event)) != NULL) {
        if(mp->button() != PlotClickEvent::SINGLE &&
           mp->button() != PlotClickEvent::DOUBLE) return;
        m_inDraggingMode = true;
        m_canvas->setCursor(HAND_CLOSED);
        m_lastCoord = mp->where();
        if(m_lastCoord.system() != PlotCoordinate::WORLD)
            m_lastCoord = m_canvas->convertCoordinate(m_lastCoord);
        m_lastEventHandled = true;
        
    } else if((mr= dynamic_cast<const PlotMouseReleaseEvent*>(&event))!= NULL){
        if(!m_inDraggingMode) return;
        m_inDraggingMode = false;
        m_canvas->setCursor(HAND_OPEN);
        m_stack->addRegion(m_canvas->axesRanges(m_xAxis, m_yAxis),
                           m_xAxis, m_yAxis);
        m_lastEventHandled = true;
        
    } else if((mm = dynamic_cast<const PlotMouseDragEvent*>(&event)) != NULL) {
        if(!m_inDraggingMode) return;
        PlotCoordinate c = mm->where();
        if(c.system() != PlotCoordinate::WORLD)
            c = m_canvas->convertCoordinate(c);
        m_canvas->moveAxesRanges(m_xAxis, m_lastCoord.x() - c.x(),
                                 m_yAxis, m_lastCoord.y() - c.y());
        m_lastCoord = c;
        m_lastEventHandled = true;
    
    // stack navigation
    } else if((w = dynamic_cast<const PlotWheelEvent*>(&event)) != NULL) {
        m_canvas->canvasAxesStackMove(w->delta());
        m_lastEventHandled = true;
        
    } else if((c = dynamic_cast<const PlotClickEvent*>(&event)) != NULL) {
        if(c->button() == PlotClickEvent::CONTEXT) {
            m_canvas->canvasAxesStackMove(0);
            m_lastEventHandled = true;
        }
    }
    
    if(m_lastEventHandled) notifyWatchers();
}

void PlotPanTool::reset() {
    if(m_stack != NULL) m_stack->clearStack(true);
}


void PlotPanTool::attach(PlotCanvas* canvas) {
    PlotMouseTool::attach(canvas);
    if(canvas != NULL) {
        m_stack = &canvas->canvasAxesStack();
        if(m_active && m_stack->size() == 0)
            m_stack->setBase(m_canvas->axesRanges(m_xAxis, m_yAxis),
                             m_xAxis, m_yAxis);
    }
}

void PlotPanTool::detach() {
    if(m_canvas != NULL) m_canvas->setCursor(NORMAL_CURSOR);
    m_stack = NULL;
    PlotMouseTool::detach();
}

void PlotPanTool::notifyWatchers() {
    for(unsigned int i = 0; i < m_notifiers.size(); i++)
        m_notifiers[i]->notifyPanChanged(*this);
}


/////////////////////////////////
// PLOTTRACKERTOOL DEFINITIONS //
/////////////////////////////////

PlotTrackerTool::PlotTrackerTool(PlotCoordinate::System sys) :
        PlotMouseTool(sys), m_drawText(false), m_format(DEFAULT_FORMAT) { }

PlotTrackerTool::PlotTrackerTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System sys) : PlotMouseTool(xAxis, yAxis, sys),
        m_drawText(false), m_format(DEFAULT_FORMAT) { }

PlotTrackerTool::~PlotTrackerTool() { }

void PlotTrackerTool::addNotifier(PlotTrackerToolNotifier* notifier) {
    if(notifier == NULL) return;
    for(unsigned int i = 0; i < m_notifiers.size(); i++)
        if(m_notifiers[i] == notifier) return;
    m_notifiers.push_back(notifier);
}

void PlotTrackerTool::setDrawText(bool draw) { m_drawText = draw; }
bool PlotTrackerTool::drawsText() const { return m_drawText; }

void PlotTrackerTool::setFormat(const String& format) { m_format = format; }

PlotAnnotationPtr PlotTrackerTool::getAnnotation() { return m_annotation; }

PlotCoordinate PlotTrackerTool::getCoordinate(PlotCoordinate::System s) const {
    if(m_annotation.null() || m_canvas == NULL) return PlotCoordinate();
    PlotCoordinate c = m_annotation->coordinate();
    if(s == c.system()) return c;
    else                return m_canvas->convertCoordinate(c, s);
}

void PlotTrackerTool::setActive(bool active) {
    if(active != m_active) {
        PlotMouseTool::setActive(active);
        if(m_canvas != NULL && !m_annotation.null())
            m_canvas->removeAnnotation(m_annotation);
    }
}

void PlotTrackerTool::handleMouseEvent(const PlotEvent& ev) {
    m_lastEventHandled = false;
    if(m_canvas == NULL) return;
    
    const PlotMouseMoveEvent* m = dynamic_cast<const PlotMouseMoveEvent*>(&ev);
    if(m != NULL) {
        bool autorescale = m_canvas->axesAutoRescale();
        if(m_drawText && !m_annotation.null()) {
            m_canvas->removeAnnotation(m_annotation);
            m_canvas->setAxesAutoRescale(false);
        }
        PlotCoordinate c = m->where();
        m_annotation->setCoordinate(c);
        m_annotation->setText(formattedString(c.x(), c.y()));
        if(m_drawText) {
            m_canvas->plotItem(m_annotation, ANNOTATION);
            m_canvas->setAxesAutoRescale(autorescale);
        }
        m_lastEventHandled = true;
    }
    
    if(m_lastEventHandled) notifyWatchers();
}


// Protected Methods //

void PlotTrackerTool::attach(PlotCanvas* canvas) {
    PlotMouseTool::attach(canvas);
    if(m_canvas != NULL) {
        m_annotation = m_factory->annotation("", PlotCoordinate());
        if(m_active) {
            PlotMouseMoveEventHandlerPtr h(this, false);
            m_canvas->registerMouseMoveHandler(h, m_coordSystem);
        }
    }
}

void PlotTrackerTool::detach() {
    if(m_canvas != NULL && m_active) {
        PlotMouseMoveEventHandlerPtr h(this, false);
        m_canvas->unregisterMouseMoveHandler(h);
    }
    PlotMouseTool::detach();
    m_annotation = NULL;
}

void PlotTrackerTool::notifyWatchers() {
    for(unsigned int i = 0; i < m_notifiers.size(); i++)
        m_notifiers[i]->notifyTrackerChanged(*this);
}


// Static Members //

String PlotTrackerTool::formattedString(const String& format, double x,
        double y, PlotCanvas* canvas, PlotAxis xAxis, PlotAxis yAxis) {
    stringstream ss;
    
    // calculate time values
    int xy, xm, xd, xh, xn, yy, ym, yd, yh, yn;
    double xs, ys, temp = x;
    if(canvas != NULL && canvas->axisScale(xAxis) == DATE_MJ_SEC)
        temp /= 86400;
    Time tx(temp + 2400000.5);
    xs = modf(temp, &xs);
    
    temp = y;
    if(canvas != NULL && canvas->axisScale(yAxis) == DATE_MJ_SEC)
        temp /= 86400;
    Time ty(temp + 2400000.5);
    ys = modf(temp, &ys);
    
    xy = tx.year();       yy = ty.year();
    xm = tx.month();      ym = ty.month();
    xd = tx.dayOfMonth(); yd = ty.dayOfMonth();
    xh = tx.hours();      yh = ty.hours();
    xn = tx.minutes();    yn = ty.minutes();
    xs = modf(x, &xs);    ys = modf(y, &ys);
    xs += tx.seconds();   ys += ty.seconds();
    
    unsigned int i = 0, j = 0;
    String token;
    while(i < format.length()) {
        // get the next token
        j = format.find(FORMAT_DIVIDER, i);
        if(j > i) {
            if(j < format.size()) { // text between i and token
                ss << format.substr(i, (j - i));
                i = j;
                
            } else { // no next tokens
                ss << format.substr(i);
                break;
            }
        }
        
        // i points to the first character of the divider
        i += FORMAT_DIVIDER.size();
        // i points to the first character of the token
        
        j = format.find(FORMAT_DIVIDER, i);
        if(j >= format.size()) {
            // no closing divider
            i -= FORMAT_DIVIDER.size();
            ss << format.substr(i);
            break;
        }
        
        token = format.substr(i, (j - i));
        i = j + FORMAT_DIVIDER.size(); // i after token
        
        if(token.substr(0, FORMAT_X.size()) == FORMAT_X ||
           token.substr(0, FORMAT_Y.size()) == FORMAT_Y) {
            if(token == FORMAT_X)      ss << x;                
            else if(token == FORMAT_Y) ss << y;
            else {
                // x or y as a date, or invalid token
                bool isX = token.substr(0, FORMAT_X.size()) == FORMAT_X;
                if(isX) token = token.substr(FORMAT_X.size());
                else    token = token.substr(FORMAT_Y.size());
                
                if(token == FORMAT_YEAR)        ss << (isX ? xy : yy);
                else if(token == FORMAT_MONTH)  ss << (isX ? xm : ym);
                else if(token == FORMAT_DAY)    ss << (isX ? xd : yd);
                else if(token == FORMAT_HOUR)   ss << (isX ? xh : yh);
                else if(token == FORMAT_MINUTE) ss << (isX ? xn : yn);
                else if(token == FORMAT_SECOND) ss << (isX ? xs : ys);
                else // invalid token
                    ss << FORMAT_DIVIDER << (isX ? FORMAT_X : FORMAT_Y)
                       << token << FORMAT_DIVIDER;
            }
            
        } else if(token.substr(0, FORMAT_PRECISION.size())== FORMAT_PRECISION){
            if(token.size() == FORMAT_PRECISION.size()) // invalid
                ss << FORMAT_DIVIDER << FORMAT_PRECISION << FORMAT_DIVIDER;
            else {
                token = token.substr(FORMAT_PRECISION.size());
                int prec;
                if(sscanf(token.c_str(), "%d", &prec) == 1)
                    ss << setprecision(prec);
                else // invalid
                    ss << FORMAT_DIVIDER << FORMAT_PRECISION << token
                       << FORMAT_DIVIDER;
            }
            
        } else {
            // not a valid token
            ss << FORMAT_DIVIDER << token << FORMAT_DIVIDER;
        }
    }
    
    return ss.str();
}

const String PlotTrackerTool::FORMAT_DIVIDER = "%%";
const String PlotTrackerTool::FORMAT_X = "x";
const String PlotTrackerTool::FORMAT_Y = "y";
const String PlotTrackerTool::FORMAT_YEAR = ":y";
const String PlotTrackerTool::FORMAT_MONTH = ":m";
const String PlotTrackerTool::FORMAT_DAY = ":d";
const String PlotTrackerTool::FORMAT_HOUR = ":h";
const String PlotTrackerTool::FORMAT_MINUTE = ":n";
const String PlotTrackerTool::FORMAT_SECOND = ":s";
const String PlotTrackerTool::FORMAT_PRECISION = "P";

const String PlotTrackerTool::DEFAULT_FORMAT = "(" + FORMAT_DIVIDER + FORMAT_X+
        FORMAT_DIVIDER + ", " + FORMAT_DIVIDER + FORMAT_Y + FORMAT_DIVIDER +
        ")";


////////////////////////////////////
// PLOTMOUSETOOLGROUP DEFINITIONS //
////////////////////////////////////

// Public Methods //

PlotMouseToolGroup::PlotMouseToolGroup() { }

PlotMouseToolGroup::~PlotMouseToolGroup() { }

unsigned int PlotMouseToolGroup::numTools() const { return m_tools.size(); }
vector<PlotMouseToolPtr> PlotMouseToolGroup::tools() const { return m_tools; }

unsigned int PlotMouseToolGroup::addTool(PlotMouseToolPtr tool, bool makeAtv) {
    if(tool.null()) return m_tools.size();
    for(unsigned int i = 0; i < m_tools.size(); i++)
        if(m_tools[i] == tool) return i;    
    
    m_tools.push_back(tool);
    if(m_canvas != NULL) tool->attach(m_canvas);
    
    // bookkeeping: blocking, active, active tool
    tool->setBlocking(m_blocking);
    tool->setActive(false);
    if(makeAtv) setActiveTool(tool);
    
    return m_tools.size() - 1;
}

bool PlotMouseToolGroup::removeTool(PlotMouseToolPtr tool) {
    if(tool.null()) return false;
    for(unsigned int i = 0; i < m_tools.size(); i++) {
        if(m_tools[i] == tool) {
            m_tools.erase(m_tools.begin() + i);
            if(tool == m_activeTool) {
                if(m_tools.size() > 0) setActiveTool(m_tools[0]);
                else                   m_activeTool = PlotMouseToolPtr();
            }
            return true;
        }
    }
    return false;
}

PlotMouseToolPtr PlotMouseToolGroup::toolAt(unsigned int index) const {
    if(index < m_tools.size()) return m_tools[index];
    else                       return PlotMouseToolPtr();
}

unsigned int PlotMouseToolGroup::indexOf(PlotMouseToolPtr tool) const {
    if(tool.null()) return m_tools.size();
    for(unsigned int i = 0; i < m_tools.size(); i++)
        if(m_tools[i] == tool) return i;
    return m_tools.size();
}

void PlotMouseToolGroup::setActiveTool(PlotMouseToolPtr tool) {
    addTool(tool, false);
    m_activeTool = tool;
    for(unsigned int i = 0; i < m_tools.size(); i++)
        m_tools[i]->setActive(false);
    if(!tool.null()) tool->setActive(m_active);
}

void PlotMouseToolGroup::setActive(bool isActive) {
    PlotMouseTool::setActive(isActive);
    if(!m_activeTool.null()) m_activeTool->setActive(isActive);
}

void PlotMouseToolGroup::setBlocking(bool blocking) {
    PlotMouseTool::setBlocking(blocking);
    for(unsigned int i = 0; i < m_tools.size(); i++)
        m_tools[i]->setBlocking(blocking);
}

void PlotMouseToolGroup::handleMouseEvent(const PlotEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleMouseEvent(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}
void PlotMouseToolGroup::handleSelect(const PlotSelectEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleSelect(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}
void PlotMouseToolGroup::handleClick(const PlotClickEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleClick(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}
void PlotMouseToolGroup::handleMousePress(const PlotMousePressEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleMousePress(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}
void PlotMouseToolGroup::handleMouseRelease(const PlotMouseReleaseEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleMouseRelease(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}
void PlotMouseToolGroup::handleMouseDrag(const PlotMouseDragEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleMouseDrag(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}
void PlotMouseToolGroup::handleMouseMove(const PlotMouseMoveEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleMouseMove(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}
void PlotMouseToolGroup::handleWheel(const PlotWheelEvent& event) {
    if(m_active && !m_activeTool.null()) {
        m_activeTool->handleWheel(event);
        m_lastEventHandled = m_activeTool->lastEventWasHandled();
    } else m_lastEventHandled = false;
}

PlotAxis PlotMouseToolGroup::getXAxis() const {
    if(!m_activeTool.null()) return m_activeTool->getXAxis();
    else if(m_tools.size() > 0) return m_tools[0]->getXAxis();
    else return m_xAxis;
}

PlotAxis PlotMouseToolGroup::getYAxis() const {
    if(!m_activeTool.null()) return m_activeTool->getYAxis();
    else if(m_tools.size() > 0) return m_tools[0]->getYAxis();
    else return m_yAxis;
}

PlotCoordinate::System PlotMouseToolGroup::getCoordinateSystem() const {
    if(!m_activeTool.null()) return m_activeTool->getCoordinateSystem();
    else if(m_tools.size() > 0) return m_tools[0]->getCoordinateSystem();
    else return m_coordSystem;
}

bool PlotMouseToolGroup::lastEventWasHandled() const {
    if(!m_activeTool.null()) return m_activeTool->lastEventWasHandled();
    else return false;
}

void PlotMouseToolGroup::reset() {
    for(unsigned int i = 0; i < m_tools.size(); i++) {
        m_tools[i]->reset();
    }
}


// Protected Methods //

void PlotMouseToolGroup::attach(PlotCanvas* canvas) {
    PlotMouseTool::attach(canvas);
    for(unsigned int i = 0; i< m_tools.size(); i++) m_tools[i]->attach(canvas);
}

void PlotMouseToolGroup::detach() {
    PlotMouseTool::detach();
    for(unsigned int i = 0; i < m_tools.size(); i++) m_tools[i]->detach();
}


////////////////////////////////////////////
// PLOTSTANDARDMOUSETOOLGROUP DEFINITIONS //
////////////////////////////////////////////

PlotStandardMouseToolGroup::PlotStandardMouseToolGroup(Tool activeTool,
        PlotCoordinate::System system) {
    addTool(new PlotSelectTool(system));
    addTool(new PlotZoomTool(system));
    addTool(new PlotPanTool(system));
    setActiveTool(activeTool);
    m_tracker = new PlotTrackerTool(system);
    m_tracker->setBlocking(false);
    m_coordSystem = system;
}

PlotStandardMouseToolGroup::PlotStandardMouseToolGroup(PlotAxis xAxis,
        PlotAxis yAxis,Tool activeTool, PlotCoordinate::System system) {
    addTool(new PlotSelectTool(xAxis, yAxis, system));
    addTool(new PlotZoomTool(xAxis, yAxis, system));
    addTool(new PlotPanTool(xAxis, yAxis, system));
    setActiveTool(activeTool);
    m_tracker = new PlotTrackerTool(xAxis, yAxis, system);
    m_tracker->setBlocking(false);
    m_xAxis = xAxis;
    m_yAxis = yAxis;
    m_coordSystem = system;
}

PlotStandardMouseToolGroup::PlotStandardMouseToolGroup(
        PlotSelectToolPtr selectTool, PlotZoomToolPtr zoomTool,
        PlotPanToolPtr panTool, PlotTrackerToolPtr trackerTool,
        Tool activeTool) {
    addTool(!selectTool.null() ? selectTool : new PlotSelectTool());
    addTool(!zoomTool.null()   ? zoomTool   : new PlotZoomTool());
    addTool(!panTool.null()    ? panTool    : new PlotPanTool());
    setActiveTool(activeTool);
    m_tracker = !trackerTool.null() ? trackerTool : new PlotTrackerTool();
    m_tracker->setBlocking(false);
}

PlotStandardMouseToolGroup::~PlotStandardMouseToolGroup() { }

void PlotStandardMouseToolGroup::setActiveTool(Tool tool) {
    if(tool == NONE) {
        PlotMouseToolGroup::setActiveTool(PlotMouseToolPtr());
        return;
    }
    for(unsigned int i = 0; i < m_tools.size(); i++) {
        if((dynamic_cast<PlotSelectTool*>(&*m_tools[i]) != NULL &&
           tool == SELECT) || (dynamic_cast<PlotZoomTool*>(&*m_tools[i])!= NULL
           && tool==ZOOM)|| (dynamic_cast<PlotPanTool*>(&*m_tools[i])!= NULL)){
            PlotMouseToolGroup::setActiveTool(i);
            return;
        }
    }
}

PlotStandardMouseToolGroup::Tool
PlotStandardMouseToolGroup::activeToolType() const {
    if(m_activeTool.null()) return NONE;
    if(dynamic_cast<const PlotSelectTool*>(&*m_activeTool)!=NULL)return SELECT;
    if(dynamic_cast<const PlotZoomTool*>(&*m_activeTool) != NULL)return ZOOM;
    if(dynamic_cast<const PlotPanTool*>(&*m_activeTool) != NULL) return PAN;
    return NONE;
}

void PlotStandardMouseToolGroup::turnTracker(bool on) {
    m_tracker->setActive(on); }
bool PlotStandardMouseToolGroup::trackerIsOn() const {
    return m_tracker->isActive(); }
void PlotStandardMouseToolGroup::turnTrackerDrawText(bool on) {
    m_tracker->setDrawText(on); }
bool PlotStandardMouseToolGroup::trackerDrawsText() const {
    return m_tracker->drawsText(); }

PlotSelectToolPtr PlotStandardMouseToolGroup::selectTool() {
    for(unsigned int i = 0; i < m_tools.size(); i++)
        if(dynamic_cast<PlotSelectTool*>(&*m_tools[i]) != NULL)
            return PlotSelectToolPtr(m_tools[i]);

    // shouldn't happen!
    PlotSelectToolPtr t = new PlotSelectTool();
    m_tools.push_back(t);
    return t;
}

PlotZoomToolPtr PlotStandardMouseToolGroup::zoomTool() {
    for(unsigned int i = 0; i < m_tools.size(); i++)
        if(dynamic_cast<PlotZoomTool*>(&*m_tools[i]) != NULL)
            return PlotZoomToolPtr(m_tools[i]);

    // shouldn't happen!
    PlotZoomToolPtr t = new PlotZoomTool();
    m_tools.push_back(t);
    return t;
}

PlotPanToolPtr PlotStandardMouseToolGroup::panTool() {
    for(unsigned int i = 0; i < m_tools.size(); i++)
        if(dynamic_cast<PlotPanTool*>(&*m_tools[i]) != NULL)
            return PlotPanToolPtr(m_tools[i]);

    // shouldn't happen!
    PlotPanToolPtr t = new PlotPanTool();
    m_tools.push_back(t);
    return t;
}

PlotTrackerToolPtr PlotStandardMouseToolGroup::trackerTool() {
    return m_tracker; }


// Protected Methods //

void PlotStandardMouseToolGroup::attach(PlotCanvas* canvas) {
    PlotMouseToolGroup::attach(canvas);
    m_tracker->attach(canvas);
}

void PlotStandardMouseToolGroup::detach() {
    PlotMouseToolGroup::detach();
    m_tracker->detach();
}

}
