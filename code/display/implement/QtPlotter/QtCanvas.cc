//# Copyright (C) 2005
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

#include <casa/aips.h>
#include <casa/Containers/Record.h>

#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/Util.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QAction>
#include <QMouseEvent>
#include <cmath>
#include <QtGui>
#include <iostream>
#include <graphics/X11/X_exit.h>


 
namespace casa { 

QtCanvas::~QtCanvas()
{}

QtCanvas::QtCanvas(QWidget *parent)
        : QWidget(parent),
          title(), yLabel(), welcome(),
          showTopAxis( true ), showToolTips( true ), displayStepFunction( false ),
          gaussianContextMenu( this )
{    
    setMouseTracking(true);
    setAttribute(Qt::WA_NoBackground);
    setBackgroundRole(QPalette::Dark);
    setForegroundRole(QPalette::Light);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    rubberBandIsShown = false;
    xRangeIsShown=false;
    imageMode = false;
    setPlotSettings(QtPlotSettings());
    xRangeMode=false;
    autoScaleX = 2;
    autoScaleY = 2;
    plotError  = 2;
    showGrid   = 2;

    initGaussianEstimateContextMenu();
}


void QtCanvas::initGaussianEstimateContextMenu(){
	QAction* centerPeakAction = new QAction("(Center,Peak)", this );
	centerPeakAction->setStatusTip( "Set the (Center,Peak) of the Gaussian Estimate");
	connect( centerPeakAction, SIGNAL(triggered()), this, SLOT(gaussianCenterPeakSelected()));
	gaussianContextMenu.addAction( centerPeakAction );
	QAction* fwhmAction = new QAction( "(Center +/- FWHM/2, Peak/2)", this );
	fwhmAction->setStatusTip("Specify the fwhm");
	connect( fwhmAction, SIGNAL(triggered()), this, SLOT(gaussianFWHMSelected()));
	gaussianContextMenu.addAction( fwhmAction );
}

void QtCanvas::setPlotSettings(const QtPlotSettings &settings)
{
    if (autoScaleX != 0 && autoScaleY != 0) {
       zoomStack.resize(1);
       zoomStack[0] = settings;
       curZoom = 0;
    }
    else {
       if (zoomStack.size() < 1) {
          zoomStack.resize(1);
          curZoom = 0;
       }
       zoomStack[curZoom] = settings;
    }

    
    curMarker = 0;
    refreshPixmap();
}

void QtCanvas::zoomOut()
{
	xRangeIsShown=false;
    if (curZoom > 0)
    {
        --curZoom;        
        refreshPixmap();
    }
    else
    {
    	if (curveMap.size() != 0)
    		defaultZoomOut();
    }
    emit xRangeChanged(1.0,0.0);
}
void QtCanvas::defaultZoomOut()
{
	QtPlotSettings prevSettings = zoomStack[curZoom];
	QtPlotSettings settings;
    std::vector<QtPlotSettings>::iterator it;
    it = zoomStack.begin();

    double zoomFactor = (double)FRACZOOM/100.0;
    settings.zoomOut( zoomFactor );

	//zoomStack.resize(zoomStack.size() + 1);
	it = zoomStack.insert ( it , settings);
    refreshPixmap();
}

void QtCanvas::zoomIn()
{
	//xRangeIsShown=false;
	//updatexRangeBandRegion();
	//emit xRangeChanged(1.0,0.0);

    if (curZoom < (int)zoomStack.size() - 1)
    {
    	xRangeIsShown=false;
       ++curZoom;
        refreshPixmap();
    	emit xRangeChanged(1.0,0.0);
    }
    else
    {
    	if (curveMap.size() != 0)
    		defaultZoomIn();
    }
}

void QtCanvas::defaultZoomIn()
{
	QtPlotSettings prevSettings = zoomStack[curZoom];
	QtPlotSettings settings;

	double zoomFactor = (double)FRACZOOM/100.0;
	settings.zoomIn( zoomFactor );

	//zoomStack.resize(curZoom + 1);
	zoomStack.push_back(settings);
	zoomIn();
}

void QtCanvas::zoomNeutral()
{
	xRangeIsShown=false;

    zoomStack.resize(1);
    zoomStack[0] = QtPlotSettings();
    curZoom = 0;

    setDataRange();
	emit xRangeChanged(1.0,0.0);
}

int QtCanvas::getLineCount()
{
    return curveMap.size();
}
CurveData* QtCanvas::getCurveData(int id)
{
    return &curveMap[id];
}
ErrorData* QtCanvas::getCurveError(int id)
{
    return &errorMap[id];
}
QString QtCanvas::getCurveName(int id)
{
    return legend[id];
}

void QtCanvas::setCurveData(int id, const CurveData &data, const ErrorData &error,
                                    const QString& lbl){
    curveMap[id]     = data;
    errorMap[id]     = error;
    legend[id]       = lbl;
    refreshPixmap();
}

void QtCanvas::setDataRange()
{
	if (autoScaleX == 0 && autoScaleY == 0)
		return;

	double xmin = 1000000000000000000000000.;
	double xmax = -xmin;
	double ymin = 1000000000000000000000000.;
	double ymax = -ymin;
	std::map<int, CurveData>::const_iterator it = curveMap.begin();
	while (it != curveMap.end())
	{
		int id = (*it).first;
		const CurveData &data = (*it).second;
		int maxPoints = data.size() / 2;

		const ErrorData &error=errorMap[id];
		int nErrPoints=error.size();

		//if (plotError && nErrPoints>1){
		if (plotError && nErrPoints>0){
			for (int i = 0; i < maxPoints; ++i){
				double dx = data[2 * i];
				double dyl = data[2 * i + 1] - error[i];
				double dyu = data[2 * i + 1] + error[i];
				xmin = (xmin > dx)  ? dx : xmin;
				xmax = (xmax < dx)  ? dx : xmax;
				ymin = (ymin > dyl) ? dyl : ymin;
				ymax = (ymax < dyu) ? dyu : ymax;
			}
		}
		else {
			for (int i = 0; i < maxPoints; ++i){
				double dx = data[2 * i];
				double dy = data[2 * i + 1];
				xmin = (xmin > dx) ? dx : xmin;
				xmax = (xmax < dx) ? dx : xmax;
				ymin = (ymin > dy) ? dy : ymin;
				ymax = (ymax < dy) ? dy : ymax;
			}
		}
		++it;
	}

	QtPlotSettings settings;
	adjustExtremes( &xmin, &xmax );
	adjustExtremes( &ymin, &ymax );

	//Store the results in the plot settings
	if (autoScaleX) {
		settings.setMinX(QtPlotSettings::xBottom, xmin);
		settings.setMaxX(QtPlotSettings::xBottom, xmax);
		settings.setMinX(QtPlotSettings::xTop, topAxisRange.first );
		settings.setMaxX(QtPlotSettings::xTop, topAxisRange.second );
	}
	if (autoScaleY) {
		settings.setMinY(ymin);
		settings.setMaxY(ymax);
	}
	settings.adjust();

	if ( curZoom > 0 ) {
		// if the canvas is zoomed, keep the zoom level, update unzoomed state...
		zoomStack[0] = settings;
		refreshPixmap();
	} else {
		// reset the canvas, zoom, etc.
		setPlotSettings(settings);
	}
}

void QtCanvas::clearCurve()
{
    curveMap.clear();
    legend.clear();
    profileFitMarkers.clear();
}

void QtCanvas::setTopAxisRange(const Vector<Float> &values, bool topAxisDescending ){
	double min = 1000000000000000000000000.;
	double max = -min;
	for ( int i = 0; i < static_cast<int>(values.size()); i++ ){
		if ( values[i] < min ){
			min = values[i];
		}
		if ( values[i] > max ){
			max = values[i];
		}
	}
	adjustExtremes( &min, &max );
	//Switch the min and max if we need to draw the labels on the top
	//axis in descending order.
	if ( topAxisDescending ){
		float tmp = min;
		min = max;
		max = tmp;
	}
	topAxisRange.first = min;
	topAxisRange.second = max;
	QtPlotSettings currentSettings = zoomStack[curZoom];
	currentSettings.setMinX(QtPlotSettings::xTop, min );
	currentSettings.setMaxX( QtPlotSettings::xTop, max );
	currentSettings.adjust();
	zoomStack[curZoom] = currentSettings;
	refreshPixmap();
}



QSize QtCanvas::minimumSizeHint() const
{
    return QSize(4 * MARGIN, 4 * MARGIN);
}

QSize QtCanvas::sizeHint() const
{
    return QSize(8 * MARGIN, 6 * MARGIN);
}



void QtCanvas::displayToolTip( QMouseEvent* event ) const {
	if ( showToolTips ){
        QPoint mouseLocation = event -> pos();
        int mouseX = mouseLocation.x();
        int mouseY = mouseLocation.y();
        double x = getDataX( mouseX );
        double y = getDataY( mouseY );

       	QtPlotSettings settings = zoomStack[curZoom];
        QString ptCoords = findCoords( x, y );
        if ( ! ptCoords.isNull() && ! ptCoords.isEmpty() ) {
        	QToolTip::showText(event->globalPos(), ptCoords );
        } else {
        	QToolTip::hideText();
        	event->ignore();
        }
	}
}

QString QtCanvas::findCoords( double x, double y ) const {
	const double X_ERROR = .05;
	const double Y_ERROR = .05;
	std::map<int, CurveData>::const_iterator it = curveMap.begin();
	QString coordStr;
	while (it != curveMap.end()){
		const CurveData &data = (*it).second;
		for ( int i = 0; i < static_cast<int>(data.size()); i++ ){
			double curveX = data[2*i];
			double curveY = data[2*i+1];
			if ( abs(curveX - x )< X_ERROR && abs(curveY-y) < Y_ERROR ){
				coordStr.append( "(" );
				coordStr.append(QString::number( curveX ));
				coordStr.append( " " +toolTipXUnit +", " );
				coordStr.append(QString::number( curveY ));
				coordStr.append( " " + toolTipYUnit+ ")");
				return coordStr;
			}
		}
		++it;
	}
	return coordStr;

}

int QtCanvas::getRectHeight() const {
	return height() - 2 * MARGIN;
}

int QtCanvas::getRectWidth() const {
	return width() - 2 * MARGIN;
}

int QtCanvas::getRectBottom() const {
	return height() - MARGIN;
}

int QtCanvas::getRectLeft() const {
	return MARGIN;
}

double QtCanvas::getDataX( int pixelPosition ) const {
	QtPlotSettings settings = zoomStack[curZoom];
	int rectWidth = getRectWidth();
	int rectLeft = getRectLeft();
	double xAxisSpan = settings.spanX( QtPlotSettings::xBottom );
	double xMin = settings.getMinX( QtPlotSettings::xBottom );
	double dataX = xMin + (pixelPosition - rectLeft) * xAxisSpan / (rectWidth -1);
	return dataX;
}

double QtCanvas::getDataY( int pixelPosition ) const {
	QtPlotSettings settings = zoomStack[curZoom];
	int rectHeight = getRectHeight();
	int rectBottom = getRectBottom();
	double yAxisSpan = settings.spanY();
	double yMin = settings.getMinY();
	double dataY = yMin + (rectBottom - pixelPosition) * yAxisSpan / (rectHeight - 1);
	return dataY;
}

int QtCanvas::getPixelX( double dataX ) const {
	QtPlotSettings settings = zoomStack[curZoom];
	int rectWidth = getRectWidth();
	int rectLeft = getRectLeft();
	double xAxisSpan = settings.spanX( QtPlotSettings::xBottom );
	double xMin = settings.getMinX( QtPlotSettings::xBottom );
	int pixelX = rectLeft + static_cast<int>( (dataX - xMin ) * (rectWidth - 1) / xAxisSpan );
	return pixelX;
}

int QtCanvas::getPixelY( double dataY ) const {
	QtPlotSettings settings = zoomStack[curZoom];
	int rectHeight = getRectHeight();
	int rectBottom = getRectBottom();
	double yAxisSpan = settings.spanY();
	double yMin = settings.getMinY();
	int pixelY = rectBottom - static_cast<int>((dataY - yMin)*(rectHeight - 1) / yAxisSpan);
	return pixelY;
}

void QtCanvas::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QVector<QRect> rects = event->region().rects();
	for (int i = 0; i < (int)rects.size(); ++i)
		painter.drawPixmap(rects[i], pixmap, rects[i]);

	 //painter.drawPixmap(0, 0, pixmap);
    if (rubberBandIsShown)
    {
        painter.setPen(Qt::yellow);
        painter.fillRect(rubberBandRect, Qt::transparent);
        painter.drawRect(rubberBandRect.normalized());
    }
    if ( xcursor.isValid( ) ) {
    	painter.setPen(xcursor);
    	QLine line( (int) currentCursorPosition.x( ), MARGIN, (int) currentCursorPosition.x( ), height() - MARGIN );
    	painter.drawLine(line);
    }
    if (xRangeIsShown)
    {
        painter.setPen(Qt::black);
        int xStart = getPixelX( xRangeStart );
        int xEnd = getPixelX( xRangeEnd );
		xRangeRect.setLeft(xStart);
		xRangeRect.setRight(xEnd);
		xRangeRect.setBottom(MARGIN);
		xRangeRect.setTop(height()-MARGIN-1);
		painter.fillRect(xRangeRect, QColor(100,100,100,100));
		painter.drawRect(xRangeRect.normalized());
	}

    //Paint the markers
    for ( int i = 0; i < profileFitMarkers.length(); i++ ){
    	profileFitMarkers[i].drawMarker( painter );
    }

	if (hasFocus())
	{
		QStyleOptionFocusRect option;
		option.init(this);
		option.backgroundColor = palette().color(QPalette::Background);
		style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter,
				this);
	}
}

void QtCanvas::resizeEvent(QResizeEvent *)
{       
    refreshPixmap();
}

void QtCanvas::mousePressEvent(QMouseEvent *event)
{
	if ( xcursor.isValid( ) ){
		QtPlotSettings currSettings = zoomStack[curZoom];
		double dx = currSettings.spanX(QtPlotSettings::xBottom) / getRectWidth();
		double channelSelectValue = currSettings.getMinX(QtPlotSettings::xBottom ) + dx * (event->pos().x()-MARGIN);
		emit channelSelect(channelSelectValue);
		return;
	}

	if (event->button() == Qt::LeftButton) {

		//We are starting a spectral line fitting or a collapse/moments
		//rectangle, unless we already have one.
		if (event->modifiers().testFlag(Qt::ShiftModifier)){
			xRangeMode    = true;
			xRangeIsShown = true;
			xRangeRect.setLeft(event->pos().x());
			xRangeRect.setRight(event->pos().x());
			xRangeRect.setBottom(MARGIN);
			xRangeRect.setTop(height()-MARGIN-1);
			xRectStart= event->pos().x();
			xRectEnd  = event->pos().x();
			updatexRangeBandRegion();

		} else {
			xRangeIsShown     = false;
			rubberBandIsShown = true;
			rubberBandRect.setTopLeft(event->pos());
			rubberBandRect.setBottomRight(event->pos());
			updateRubberBandRegion();
		}
		setCursor(Qt::CrossCursor);
	}

	else if (event->button() == Qt::RightButton) {
		if ( ! event->modifiers().testFlag(Qt::ShiftModifier)){
			//cout << "Right button! " << endl;
			int x = event->pos().x() - MARGIN;
			int y = event->pos().y() - MARGIN;
			//cout << "x=" << x << " y=" << y << endl;
			QtPlotSettings prevSettings = zoomStack[curZoom];
			QtPlotSettings settings;

			double dx = prevSettings.spanX(QtPlotSettings::xBottom) / getRectWidth();
			double dy = prevSettings.spanY() / ( getRectHeight() );
			x = (int)(prevSettings.getMinX(QtPlotSettings::xBottom) + dx * x);
			y = (int)(prevSettings.getMaxY() - dy * y);
			//cout << "x=" << x << " y=" << y << endl;

			std::map<int, CurveData>::const_iterator it = markerStack.begin();

			while (it != markerStack.end()){

				int id = (*it).first;
				const CurveData &data = (*it).second;
				//cout << " " << data[0] << " " << data[2]
				//        << " " << data[1] << " " << data[3] << endl;
				if ( x >= data[0] && x < data[2] &&
						y <= data[1] && y > data[3]) {
					markerStack[id] = markerStack[markerStack.size() - 1];
					markerStack.erase(markerStack.size() - 1);
					refreshPixmap();
					break;
				}
				++it;
			}
		}
		else {
			if ( xRangeIsShown ){
				//The user is specifying the (Center,Peak) or the FWHM
				//of a Gaussian estimate. The point must be in the selected
				//range to be valid.
				int xPos = event->pos().x();
				int yPos = event->pos().y();
				if ( xRangeRect.contains( xPos, yPos ) ){
					gaussianEstimateX = getDataX( xPos );
					gaussianEstimateY = getDataY( yPos );
					//Show a context menu to determine if the user is specifying
					//the (Center,Peak) or FWHM.
					gaussianContextMenu.exec( event->globalPos());
				}
				else {
					QString msg( "Initial Gaussian estimates must be within\n the specified spectral-line fitting range.");
					Util::showUserMessage( msg, this );
				}
			}
			else {
				QString msg( "Please specify a Spectral-Line Fitting range \nby shift-clicking the left mouse button\n and dragging it before you specify\n initial Gaussian estimates.");
				Util::showUserMessage( msg, this );
			}

		}
	}
}


void QtCanvas::mouseMoveEvent(QMouseEvent *event)
{
	// save cursor position for xcursor setup...
	currentCursorPosition = event->pos( );

	//qDebug()  << "Mouse moved event" << event->pos() << " global: " << event->globalPos();
	if (event->buttons() & Qt::LeftButton)
	{
		if (rubberBandIsShown){
			updateRubberBandRegion();
			rubberBandRect.setBottomRight(event->pos());
			updateRubberBandRegion();
		}
		else if (xRangeIsShown){
			updatexRangeBandRegion();
			//xRangeRect.setRight(event->pos().x());
			xRectEnd = event->pos().x();

			//QRect rect = xRangeRect.normalized();
			// zero the coordinates on the plot region
			//rect.translate(-MARGIN, -MARGIN);

			QtPlotSettings currSettings = zoomStack[curZoom];
			double dx = currSettings.spanX(QtPlotSettings::xBottom) / static_cast<double>(getRectWidth());
			double currMinX = currSettings.getMinX(QtPlotSettings::xBottom);
			xRangeStart = currMinX + dx * double(xRectStart-MARGIN);
			xRangeEnd   = currMinX + dx * double(xRectEnd-MARGIN);
			if (xRangeStart<xRangeEnd)
				emit xRangeChanged(xRangeStart, xRangeEnd);
			else
				emit xRangeChanged(xRangeEnd, xRangeStart);
			updatexRangeBandRegion();
		}
	}
	else if (showToolTips && !event->buttons() ) {
		displayToolTip( event );
	}
	if ( xcursor.isValid( ) ) update( );
}

void QtCanvas::mouseReleaseEvent(QMouseEvent *event)
{
	//qDebug() << "mouse release" << event->button() << Qt::LeftButton;

	if ( xcursor.isValid( ) ) return;

	if (event->button() == Qt::LeftButton)
	{
		if (xRangeMode){
			//xRangeIsShown = false;
			//updatexRangeBandRegion();
			//unsetCursor();

			QRect rect = xRangeRect.normalized();

			if (rect.left() < 0 || rect.top() < 0 ||
					rect.right() > width() ||
					rect.bottom() > height())
				return;

			if (rect.width() < 4){
				xRangeIsShown=false;
				return;
			}

			// zero the coordinates on the plot region
			//rect.translate(-MARGIN, -MARGIN);
			QtPlotSettings currSettings = zoomStack[curZoom];
			double dx = currSettings.spanX(QtPlotSettings::xBottom) / getRectWidth();
			double currMinX = currSettings.getMinX( QtPlotSettings::xBottom );
			xRangeStart = currMinX + dx * (xRectStart-MARGIN);
			xRangeEnd   = currMinX + dx * (xRectEnd-MARGIN);

			if (xRangeStart<xRangeEnd)
				emit xRangeChanged(xRangeStart, xRangeEnd);
			else
				emit xRangeChanged(xRangeEnd, xRangeStart);

			//updatexRangeBandRegion();
			//unsetCursor();

			//if (!shiftPressed)
			xRangeMode=false;
		}
		else {
			rubberBandIsShown = false;
			updateRubberBandRegion();
			unsetCursor();

			QRect rect = rubberBandRect.normalized();

			//qDebug() << "rect: left" << rect.left()
			//         << "right" << rect.right()
			//         << "top" << rect.top()
			//         << "bottom" << rect.bottom()
			//         << "width" << width()
			//         << "height" << height()
			//         << "MARGIN" << MARGIN;
			//zoom only if zoom box is in the plot region
			//if (rect.left() < MARGIN || rect.top() < MARGIN ||
			//   rect.right() > width() - MARGIN ||
			//    rect.bottom() > height() -  MARGIN)
			//    return;
			//        //zoom only if zoom box is in the plot region
			if (rect.left() < 0 || rect.top() < 0 ||
					rect.right() > width() ||
					rect.bottom() > height())
				return;
			//qDebug() << "inside";

			if (rect.width() < 4 || rect.height() < 4)
				return;
			//qDebug() << "big enough";

			//cout << "numCurves " << curveMap.size() << endl;
			//if (curveMap.size() == 0)
			//    return;

			// zero the coordinates on the plot region
			rect.translate(-MARGIN, -MARGIN);

			QtPlotSettings prevSettings = zoomStack[curZoom];
			QtPlotSettings settings;

			for ( int i = 0; i < QtPlotSettings::END_AXIS_INDEX; i++ ){
				QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(i);
				double dx = prevSettings.spanX(axisIndex) / getRectWidth();
				double prevMinX = prevSettings.getMinX( axisIndex );
				settings.setMinX( axisIndex, prevMinX + dx * rect.left() );
				settings.setMaxX( axisIndex, prevMinX + dx * rect.right() );
			}
			double dy = prevSettings.spanY() / (getRectHeight());
			double prevMaxY = prevSettings.getMaxY();
			settings.setMinY( prevMaxY - dy * rect.bottom() );
			settings.setMaxY( prevMaxY - dy * rect.top() );
			//qDebug() << "min-x: " << settings.minX << " max-x: " << settings.maxX;
			settings.adjust();
			if (curveMap.size() != 0) {
				//qDebug() << "zoomin ";
				zoomStack.resize(curZoom + 1);
				zoomStack.push_back(settings);
				zoomIn();
			}
		}
	}
}

void QtCanvas::keyPressEvent(QKeyEvent *event)
{
    if (!imageMode)
    switch (event->key())
    {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Left:
        zoomStack[curZoom].scroll(-1, 0);
        refreshPixmap();
        break;
    case Qt::Key_Right:
        zoomStack[curZoom].scroll(+1, 0);
        refreshPixmap();
        break;
    case Qt::Key_Down:
        zoomStack[curZoom].scroll(0, -1);
        refreshPixmap();
        break;
    case Qt::Key_Up:
        zoomStack[curZoom].scroll(0, +1);
        refreshPixmap();
        break;
    case Qt::Key_Escape:
    	if (xRangeIsShown){
    		xRangeIsShown=false;
    		updatexRangeBandRegion();
    		emit xRangeChanged(1.0,0.0);
    	}
        break;
#if defined(__APPLE__)
    case Qt::Key_Meta:
#else
    case Qt::Key_Control:
#endif
	xcursor = QColor(Qt::gray);
	update( );
	break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void QtCanvas::keyReleaseEvent(QKeyEvent *event) {
    if (!imageMode)
    switch (event->key()) {
	case Qt::Key_Meta:
	case Qt::Key_Control:
	    xcursor = QColor( );	// invalid color
	    update( );
	    break;
	default:
	    QWidget::keyPressEvent(event);
    }
}

void QtCanvas::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;

    if (event->orientation() == Qt::Horizontal)
        zoomStack[curZoom].scroll(numTicks, 0);
    else
        zoomStack[curZoom].scroll(0, numTicks);
    refreshPixmap();
}

void QtCanvas::gaussianCenterPeakSelected(){
	emit specFitEstimateSpecified( gaussianEstimateX, gaussianEstimateY, true );
}

void QtCanvas::gaussianFWHMSelected(){
	emit specFitEstimateSpecified( gaussianEstimateX, gaussianEstimateY, false );
}

void QtCanvas::updateRubberBandRegion(){
    update();
}

void QtCanvas::updatexRangeBandRegion(){
    update();
}


void QtCanvas::setShowTopAxis( bool showAxis ){
	bool oldShowAxis = showTopAxis;
	showTopAxis = showAxis;
	if ( oldShowAxis != showTopAxis ){
		refreshPixmap();
	}
}

void QtCanvas::refreshPixmap()
{
	pixmap = QPixmap(size());
	pixmap.fill(this, 0, 0);
	QPainter painter(&pixmap);

	drawLabels(&painter);
	if (!imageMode)
	{
		drawGrid(&painter);
		drawCurves(&painter);
		//if (xRangeIsShown)
		//	drawxRange(&painter);
	}
	else
	{
		drawBackBuffer(&painter);
		drawTicks(&painter);
	}
	if (welcome.text != "")
		drawWelcome(&painter);
	update();
}
void QtCanvas::drawBackBuffer(QPainter *painter)
{
    QRect rect(MARGIN, MARGIN, getRectWidth(), getRectHeight());
    QtPlotSettings settings = zoomStack[curZoom];
    int minX = (int)(settings.getMinX(QtPlotSettings::xBottom));
    int maxX = (int)(settings.getMaxX(QtPlotSettings::xBottom));
    int maxY = (int)(settings.getMaxY());
    int minY = (int)(settings.getMinY());
    QRect src( minX, minY, maxX, maxY) ;
    painter->drawPixmap(rect, backBuffer, src);
}

QString QtCanvas::getXTickLabel( int tickIndex, int tickCount, QtPlotSettings::AxisIndex axisIndex ) const {
	QtPlotSettings settings = zoomStack[curZoom];
	double label = settings.getMinX(axisIndex) + (tickIndex * settings.spanX(axisIndex) / tickCount);
	if (abs(label) < 0.00000005) label = 0.f;
	QString tickLabel = QString::number( label );
	if ( axisIndex == QtPlotSettings::xTop && !showTopAxis ){
	    tickLabel="";
	}
	return tickLabel;
}

void QtCanvas::drawGrid(QPainter *painter)
	{
    QRect rect(MARGIN, MARGIN, getRectWidth(), getRectHeight());
    QtPlotSettings settings = zoomStack[curZoom];
    QPen quiteDark(QPalette::Dark);
    QPen light(QPalette::Highlight);

   for ( int j = 0; j < QtPlotSettings::END_AXIS_INDEX; j++ ){
        QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(j);
        int xTickCount = settings.getNumTicksX( axisIndex );
        for (int i = 0; i <= xTickCount; ++i) {
        	int x = rect.left() + (i * (rect.width() - 1) / xTickCount );

        	//painter->setPen(quiteDark);
        	if (showGrid && axisIndex == QtPlotSettings::xBottom){
        		painter->setPen(quiteDark);
        		painter->drawLine(x, rect.top(), x, rect.bottom());
        	}

        	//Drawing the tick marks
        	painter->setPen(light);
        	if ( axisIndex == QtPlotSettings::xBottom ){
        		painter->drawLine(x, rect.bottom(), x, rect.bottom() + 5);
        	}
        	else if ( axisIndex == QtPlotSettings::xTop ){
        		painter->drawLine(x, rect.top(), x, rect.top() - 5);
        	}

        	//Drawing the tick labels
        	QString tickLabel = getXTickLabel( i, xTickCount, axisIndex );
        	const int AXIS_LABEL_OFFSET = 5;
        	int yPosition = rect.bottom() + AXIS_LABEL_OFFSET;
        	if ( axisIndex == QtPlotSettings::xTop ){
        		yPosition = rect.top() - 4 * AXIS_LABEL_OFFSET;
        	}
        	painter->drawText(x - 50, yPosition, 100, 15,
                          Qt::AlignHCenter | Qt::AlignTop,
                          tickLabel );
        }
    }
    int tickCountY = settings.getNumTicksY();
    for (int j = 0; j <= tickCountY; ++j)
    {
        int y = rect.bottom() - (j * (rect.height() - 1) / tickCountY );
        double label = settings.getMinY() + (j * settings.spanY() / tickCountY );

        if (abs(label) < 0.00000005) label = 0.f;                                 
        //painter->setPen(quiteDark);
        if (showGrid) {
           painter->setPen(quiteDark);
      	  painter->drawLine(rect.left(), y, rect.right(), y);
        }
        painter->setPen(light);
        painter->drawLine(rect.left() - 5, y, rect.left(), y);
        painter->drawText(rect.left() - MARGIN, y - 10,
                          MARGIN - 5, 20,
                          Qt::AlignRight | Qt::AlignVCenter,
                          QString::number(label));
    }
    painter->drawRect(rect);
    //painter->setPen(Qt::red);
    //painter->drawRect(rubberBandRect);
}
void QtCanvas::drawTicks(QPainter *painter)
{
	QtPlotSettings settings = zoomStack[curZoom];
	QPen quiteDark(QPalette::Dark);
	QPen light(QPalette::Highlight);
	for ( int j = 0; j < QtPlotSettings::END_AXIS_INDEX; j++ ){
		int startY = MARGIN;
		if ( j == QtPlotSettings::xTop ){
			startY = height() - MARGIN;
		}
		QRect rect(MARGIN, startY, getRectWidth(), getRectHeight());
		QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(j);
		int tickCountX = settings.getNumTicksX( axisIndex );
		for (int i = 0; i <= tickCountX; ++i){
			int x = rect.left() + (i * (rect.width() - 1) / tickCountX );
			QString tickLabel = getXTickLabel( i, tickCountX, axisIndex );
			painter->setPen(quiteDark);
			painter->drawLine(x, rect.top(), x, rect.top() + 5);
			painter->setPen(light);
			painter->drawLine(x, rect.bottom() - 5, x, rect.bottom());
			painter->drawText(x - 50, rect.bottom() + 5, 100, 15,
							  Qt::AlignHCenter | Qt::AlignTop,
							  tickLabel );
		}
	}
	QRect rect(MARGIN, MARGIN, getRectWidth(), getRectHeight());
	int tickCountY = settings.getNumTicksY();
    for (int j = 0; j <= tickCountY; ++j){
        int y = rect.bottom() - (j * (rect.height() - 1) / tickCountY );
        double label = settings.getMinY() + (j * settings.spanY() / tickCountY );
        if (abs(label) < 0.00000005) label = 0.f;                                  
        painter->setPen(quiteDark);
        painter->drawLine(rect.right() - 5, y, rect.right(), y);
        painter->setPen(light);
        painter->drawLine(rect.left(), y, rect.left() + 5, y);
        painter->drawText(rect.left() - MARGIN / 2, y - 10,
                          MARGIN - 5, 20,
                          Qt::AlignRight | Qt::AlignVCenter,
                          QString::number(label));
    }
    painter->drawRect(rect);
    //painter->setPen(Qt::red);
    //painter->drawRect(rubberBandRect);
}

void QtCanvas::drawLabels(QPainter *painter)
{
    QFont ft(painter->font());
    QPen pen(painter->pen());
    
    painter->setPen(title.color);
    painter->setFont(QFont(title.fontName, title.fontSize));
    painter->drawText(MARGIN, 8, getRectWidth(), MARGIN / 2,
                          Qt::AlignHCenter | Qt::AlignTop, title.text);


    int endIndex = QtPlotSettings::END_AXIS_INDEX;
    if ( !showTopAxis ){
    	endIndex = QtPlotSettings::xTop;
    }
    for ( int i = 0; i < endIndex; i++ ){
    	QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(i);
    	painter->setPen(xLabel[axisIndex].color);
    	painter->setFont(QFont(xLabel[axisIndex].fontName, xLabel[axisIndex].fontSize));
    	int yPosition = height() - MARGIN / 2;
    	QString labelText = xLabel[axisIndex].text;
    	if ( axisIndex == QtPlotSettings::xTop ){
    		yPosition = MARGIN / 3 ;
    	}
    	painter->drawText(MARGIN, yPosition, getRectWidth(), MARGIN / 2,
            Qt::AlignHCenter | Qt::AlignTop, labelText );
    }
    QPainterPath text;     
    QFont font(yLabel.fontName, yLabel.fontSize); 
    QRect fontBoundingRect = QFontMetrics(font).boundingRect(yLabel.text); 
    text.addText(-QPointF(fontBoundingRect.center()), font, yLabel.text);                   
    font.setPixelSize(50);
    painter->rotate(-90);
    painter->translate(- height() / 2, MARGIN / 6);
    painter->fillPath(text, yLabel.color);
    painter->translate(height() / 2, - MARGIN / 6);
    painter->rotate(90);
    
    painter->setPen(pen);                   
    painter->setFont(ft);
    

}

void QtCanvas::drawWelcome(QPainter *painter)
{
    QFont ft(painter->font());
    QPen pen(painter->pen());
    
    painter->setPen(welcome.color);
    painter->setFont(QFont(title.fontName, welcome.fontSize));
    painter->drawText(MARGIN, MARGIN,
                      getRectWidth(), getRectHeight(),
                      Qt::AlignHCenter | Qt::AlignVCenter, 
                      welcome.text);
    painter->setPen(pen);                   
    painter->setFont(ft);
    

}
void QtCanvas::drawRects(QPainter *painter)
{
    QtPlotSettings settings = zoomStack[curZoom];
    QRect rect(MARGIN, MARGIN, getRectWidth(), getRectHeight());

    painter->setClipRect(rect.x() + 1, rect.y() + 1,
                         rect.width() - 2, rect.height() - 2);

    std::map<int, CurveData>::const_iterator it = markerStack.begin();

    int h = (*pMask).ncolumn();
    int w = (*pMask).nrow();
    for (int i = 0; i < w; i++) 
        for (int j = 0; j < h; j++) {
	      (*pMask)(i, j) = 1;
	}
    while (it != markerStack.end())
    {
        int id = (*it).first;
        const CurveData &data = (*it).second;

        QPainterPath points;
        //cout << " id=" << id << endl; 
	//cout << " " << data[0] << " " << data[2] 
	//        << " " << data[1] << " " << data[3] << endl;
	int h = (*pMask).ncolumn();
	for (int i = (int)data[0]; i < (int)data[2]; i++) 
        for (int j = (int)data[3]; j < (int)data[1]; j++) {
	      (*pMask)(i, h - j) = 0;
	}
	
	int x1 = getPixelX( data[0] );
	int x2 = getPixelX( data[2] );
	int y1 = getPixelX( data[1] );
	int y2 = getPixelX( data[3] );

    points.addRect((int)x1, (int)y1, (int)(x2 - x1), (int)(y2 - y1));

    painter->setPen(Qt::green);
	//cout << "curMarker=" << curMarker << " id=" << id << endl;
	if (id == curMarker)
	painter->setPen(Qt::white);
        painter->drawPath(points);
        ++it;
    }
}


void QtCanvas::drawxRange(QPainter *painter){
	int xStart = getPixelX( xRangeStart );
	int xEnd = getPixelX( xRangeEnd );

    xRangeRect.setBottom(MARGIN);
	xRangeRect.setTop(height()-MARGIN-1);
	xRangeRect.setRight(xStart);
	xRangeRect.setLeft(xEnd);
    painter->fillRect(xRangeRect, QColor(100,100,100,100));
    painter->drawRect(xRangeRect.normalized());
}



QColor QtCanvas::getDiscreteColor(const int &d)
{
	// maps an integer value against the 14 usefull colors
	// defined in Qt::GlobalColor;
	// is repetitive, but should suffice for all practical
	// purposes;
	QColor lColor;
	const int cpicker = d % 14;

	switch (cpicker) {
	case 0:
		lColor = Qt::red;
		break;
	case 1:
		lColor = Qt::blue;
		break;
	case 2:
		lColor = Qt::darkGreen;
		break;
	case 3:
		lColor = Qt::cyan;
		break;
	case 4:
		lColor = Qt::lightGray;
		break;
	case 5:
		lColor = Qt::magenta;
		break;
	case 6:
		lColor = Qt::yellow;
		break;
	case 7:
		lColor = Qt::darkRed;
		break;
	case 8:
		lColor = Qt::darkBlue;
		break;
	case 9:
		lColor = Qt::green;
		break;
	case 10:
		lColor = Qt::darkCyan;
		break;
	case 11:
		lColor = Qt::darkGray;
		break;
	case 12:
		lColor = Qt::darkMagenta;
		break;
	case 13:
		lColor = Qt::darkYellow;
		break;
	default:
		// should never get here
		lColor = Qt::gray;
	}
	return lColor;
}

void QtCanvas::addDiamond( int x, int y, int diamondSize, QPainterPath& points ) const {
	 // plots a diamond from
  			 // primitive lines to have a 2D item
  	for (int i = 0; i < diamondSize; ++i){
  		// closes the centre
  		if (i==1){
  			points.lineTo(x + i, y);
  		}
  		else {
    		points.moveTo(x + i, y);
  		}
  		points.lineTo(x, y - i);
  		points.lineTo(x - i, y);
  		points.lineTo(x, y + i);
  		points.lineTo(x + i, y);
    }
}

void QtCanvas::drawCurves(QPainter *painter)
{

    QFont ft(painter->font());
    QPen pen(painter->pen());
    
    QtPlotSettings settings = zoomStack[curZoom];
    QRect rect(MARGIN, MARGIN, getRectWidth(), getRectHeight());

    painter->setClipRect(rect.x() + 1, rect.y() + 1,
                         rect.width() - 2, rect.height() - 2);

    std::map<int, CurveData>::const_iterator it = curveMap.begin();
    int siz = curveMap.size();

    int sz = siz > 1 ? siz : 1;
    QColor *colorFolds = new QColor[sz];

    while (it != curveMap.end())
    {
   	 int id = (*it).first;
   	 const CurveData &data = (*it).second;
   	 const ErrorData &error = errorMap[id];

   	 int maxPoints = data.size() / 2;
   	 QPainterPath points;

   	 // get a colour
   	 colorFolds[id] = getDiscreteColor(id);

   	 if (maxPoints == 1) {
   		 int x = getPixelX( data[0] );
   		 int y = getPixelY( data[1] );
   		 if (fabs(x) < 32768 && fabs(y) < 32768){
   			addDiamond( x, y, QT_DIAMOND_SIZE, points );
   		 }

   		 if (plotError && (error.size() > 0)){

   			 double dx = data[0] - settings.getMinX(QtPlotSettings::xBottom);
   			 double dy = data[1] - settings.getMinY();
   			 double de = error[0];

   			 double x = rect.left() + (dx * (rect.width() - 1)
   					 / settings.spanX(QtPlotSettings::xBottom));
   			 double yl = rect.bottom() - ((dy-de) * (rect.height() - 1)
   					 / settings.spanY());
   			 double yu = rect.bottom() - ((dy+de) * (rect.height() - 1)
   					 / settings.spanY());

   			 if (fabs(x) < 32768 && fabs(yl) < 32768 && fabs(yu) < 32768)
   			 {
   				 points.moveTo((int)x, (int)yl);
   				 points.lineTo((int)x, (int)yu);
   			 }
   		 }

   	 }
   	 else {
   		 for (int i = 0; i < maxPoints; ++i){
   			 int x = getPixelX( data[2*i] );
   			 int y = getPixelY( data[2*i+1] );
   			 if (fabs(x) < 32768 && fabs(y) < 32768){
   				 if (i == 0){
   					 points.moveTo(x, y);
   				 }
   				 else {
   					 if (displayStepFunction ){
   						 int prevX = getPixelX( data[2*(i-1)] );
   						 int prevY = getPixelY( data[2*i-1] );
   						 points.moveTo(prevX, prevY);
   						 points.lineTo(x, prevY);
   						 //Plot the last point
   						 if ( i == maxPoints - 1){
   							addDiamond( x, y, 2, points );
   						 }
   					 }
   					 else {
   						 points.lineTo(x, y);
   					 }
   				 }
   			 }
   		 }

   		 if (plotError && (error.size() > 0)){
   			 for (int i = 0; i < maxPoints; ++i)
   			 {
   				 double dx = data[2 * i] - settings.getMinX(QtPlotSettings::xBottom);
   				 double dy = data[2 * i + 1] - settings.getMinY();
   				 double de = error[i];
   				 double x = rect.left() + (dx * (rect.width() - 1)
   						 / settings.spanX(QtPlotSettings::xBottom));
   				 double yl = rect.bottom() - ((dy-de) * (rect.height() - 1)
   						 / settings.spanY());
   				 double yu = rect.bottom() - ((dy+de) * (rect.height() - 1)
   						 / settings.spanY());

   				 if (fabs(x) < 32768 && fabs(yl) < 32768 && fabs(yu) < 32768)
   				 {
   					 points.moveTo((int)x, (int)yl);
   					 points.lineTo((int)x, (int)yu);
   				 }
   			 }
   		 }

   	 }
   	 painter->setPen(colorFolds[id]);
   	 painter->drawPath(points);

   	 if (siz > 1) {
   		 painter->setFont(QFont(xLabel[QtPlotSettings::xBottom].fontName, xLabel[QtPlotSettings::xBottom].fontSize));
   		 painter->drawText(MARGIN + 4, MARGIN + (5 + id * 15),
   				 getRectWidth(), MARGIN / 2,
   				 Qt::AlignLeft | Qt::AlignTop, legend[id]);
   	 }
   	 ++it;
    }

    delete [] colorFolds;
    painter->setPen(pen);                   
    painter->setFont(ft);
}

void QtCanvas::addPolyLine(const Vector<Float> &x,
                           const Vector<Float> &y, 
                           const QString& lb)
{
    Int xl, yl;
    x.shape(xl);
    y.shape(yl);
    CurveData data;
    for (uInt i = 0; i < (uInt)min(xl, yl); i++){
        data.push_back(x[i]);
        data.push_back(y[i]);
    }
    int j = curveMap.size();
    setCurveData(j, data, ErrorData(), lb);

    setDataRange();
    return;
}

void QtCanvas::plotPolyLines(QString path)
{
    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        int lineCount = 0;
        int pointCount = 0;
        CurveData data;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (!line.isNull() && !line.isEmpty())
            {
                if (line.startsWith('#')) {
                   if (line.contains("title"))
                      setTitle(line.mid(7)); 
                   if (line.contains("xLabel"))
                      setXLabel(line.mid(8)); 
                   if (line.contains("yLabel"))
                      setYLabel(line.mid(8)); 
                }
                else {
                   QStringList coords = line.trimmed().split(QRegExp("\\s+"));
                   if (coords.size() >= 1)
                   {
                       double x = coords[0].toDouble();
                       double y = coords[1].toDouble();
                       data.push_back(x);
                       data.push_back(y);
                       pointCount++;
                   }
                   if (in.atEnd() && pointCount > 0)
                   {
                       setCurveData(lineCount, data);
                   }
                }
            }
            else
            {
                if (pointCount > 0)
                {
                    setCurveData(lineCount, data);
                    pointCount = 0;
                    lineCount++;
                    data.clear();
                }
            }

        }
    }

    setDataRange();
    return;
}
void QtCanvas::plotPolyLine(const Vector<Float> &x, const Vector<Float> &y, const Vector<Float> &e,
                            const QString& lb)
{
    //for (int i=0; i< x.nelements(); i++)
       //cout << x(i) << " " << y(i) << endl;
    Int xl, yl, el;
    x.shape(xl);
    y.shape(yl);
    e.shape(el);
    CurveData data;
    ErrorData error;
    for (uInt i = 0; i < (uInt)min(xl, yl); i++)
    {
        data.push_back(x[i]);
        data.push_back(y[i]);
    }

    for (uInt i = 0; i < (uint)el; i++)
        error.push_back(e[i]);

    setCurveData(0, data, error, lb);

    setDataRange();
    return;
}


template<class T> void QtCanvas::plotPolyLine(const Vector<T> &x, const Vector<T>&y)
{
	//qDebug() << "plot poly line double";
	//for (int i=0; i< x.nelements(); i++)
	//   cout << x(i) << " " << y(i) << endl;
	Int xl, yl;
	x.shape(xl);
	y.shape(yl);
	CurveData data;
	for (Int i = 0; i < min(xl, yl); i++) {
		data.push_back(x[i]);
		data.push_back(y[i]);
	}
	setCurveData(0, data);

	setDataRange();
	return;
}

template<class T> void QtCanvas::plotPolyLine(const Matrix<T> &x)
{

	Int xl, yl;
	x.shape(xl, yl);
	int nr = xl / 2;
	int nc = yl / 2;
	int n = min (nr, nc);
	if (n > 0) {
		// CurveData *data = new CurveData[n];
		std::vector<CurveData> data;
		if (n < nr) {
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < xl; j++) {
					data[i].push_back(x(uInt(j), uInt(2 * i)));
					data[i].push_back(x(uInt(j), uInt(2 * i + 1)));
				}
				setCurveData(i, data[i]);
			}
		}
		else {
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < yl; j++) {
					data[i].push_back(x(2 * i, j));
					data[i].push_back(x(2 * i + 1, j));
				}
				setCurveData(i, data[i]);
			}
		}
		//delete [] data;
	}
	setDataRange();
	return;
}

void QtCanvas::setImageMode(bool b)
{
    imageMode = b;
}

void QtCanvas::increaseCurZoom()
{
    curZoom++;
    
}
int QtCanvas::getCurZoom()
{
    return curZoom;
}
int QtCanvas::getZoomStackSize()
{
    return (int)zoomStack.size();
}
void QtCanvas::setTitle(const QString &text, int fontSize, int iclr, const QString &font)
{ 
 	title.text = text;
 	title.fontName = font;
 	title.fontSize = fontSize;
 	title.color = getDiscreteColor(iclr);
}
void QtCanvas::setXLabel(const QString &text, int fontSize, int iclr, const QString &font, QtPlotSettings::AxisIndex axisIndex )
{ 
	xLabel[axisIndex].text = text;
 	xLabel[axisIndex].fontName = font;
 	xLabel[axisIndex].fontSize = fontSize;
 	xLabel[axisIndex].color = getDiscreteColor(iclr);
}
void QtCanvas::setYLabel(const QString &text, int fontSize, int iclr, const QString &font)
{ 
	yLabel.text = text;
 	yLabel.fontName = font;
 	yLabel.fontSize = fontSize;
 	yLabel.color = getDiscreteColor(iclr);
} 
void QtCanvas::setWelcome(const QString &text, int fontSize, int iclr, const QString &font)
{ 
 	welcome.text = text;
 	welcome.fontName = font;
 	welcome.fontSize = fontSize;
 	welcome.color = getDiscreteColor(iclr);
}
QPixmap* QtCanvas::graph()
{
   return &pixmap;
}

void QtCanvas::adjustExtremes( double* const min, double* const max ) const {
	const float BOUND = 0.0001f;
	const float ADJUSTMENT = 0.00001f;
	if ( fabs(*max - *min) < BOUND ){
		*max = *max + ADJUSTMENT;
		*min = *min - ADJUSTMENT;
	}
}

ProfileFitMarker QtCanvas::getMarker( int index ) const {
	ProfileFitMarker marker( this );
	if ( index < profileFitMarkers.length() ){
		marker = profileFitMarkers[index];
	}
	return marker;
}

void QtCanvas::setMarker( int index, ProfileFitMarker& marker ) {
	if ( index < profileFitMarkers.length() ){
		profileFitMarkers[index] = marker;
	}
	else {
		profileFitMarkers.append( marker );
	}
}

void QtCanvas::setProfileFitMarkerCenterPeak( int index, double center, double peak ){
	ProfileFitMarker marker = getMarker( index );
	marker.setCenterPeak( center, peak );
	setMarker( index, marker );
	update();
}

void QtCanvas::setProfileFitMarkerFWHM( int index, double fwhm, double fwhmHeight ){
	ProfileFitMarker marker = getMarker( index );
	marker.setFWHM( fwhm, fwhmHeight );
	setMarker( index, marker );
	update();
}

bool QtCanvas::isDisplayStepFunction() const{
	return displayStepFunction;
}
void QtCanvas::setDisplayStepFunction( bool displayAsStepFunction ){
	bool oldDisplayStepFunction = displayStepFunction;
	displayStepFunction = displayAsStepFunction;
	if ( oldDisplayStepFunction != displayStepFunction ){
		refreshPixmap();
	}
}
}
