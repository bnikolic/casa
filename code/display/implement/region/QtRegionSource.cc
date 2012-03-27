//# qtregionsource.cc: Qt regionfactory for generating regions
//# with surrounding Gui functionality
//# Copyright (C) 2011
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

#include <display/region/QtRegionSource.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/region/QtRectangle.qo.h>
#include <display/region/QtPolygon.qo.h>
#include <display/region/QtEllipse.qo.h>
#include <display/region/QtPoint.qo.h>
#include <display/ds9/ds9parser.h>
#include <display/DisplayDatas/DisplayData.h>
#include <imageanalysis/Annotations/AnnRectBox.h>
#include <imageanalysis/Annotations/AnnEllipse.h>
#include <imageanalysis/Annotations/AnnSymbol.h>
#include <imageanalysis/Annotations/AnnPolygon.h>
#include <measures/Measures/MCDirection.h>

namespace casa {
    namespace viewer {

	QtRegionSource::QtRegionSource( RegionCreator *rc, QtDisplayPanelGui *panel ) : RegionSource(rc), panel_(panel) {
	    connect( panel_->regionDock( ), SIGNAL(loadRegions( bool &, const QString &, const QString &)), SLOT(loadRegions(bool &, const QString &, const QString &)) );
	}

	QtRegionSource::~QtRegionSource( ) { }

	// std::tr1::shared_ptr<Rectangle> QtRegionSource::rectangle( int blc_x, int blc_y, int trc_x, int trc_y ) {
	//     return std::tr1::shared_ptr<Rectangle>(new QtRectangle( this, blc_x, blc_y, trc_x, trc_y ));
	// }
	std::tr1::shared_ptr<Rectangle> QtRegionSource::rectangle( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
	    QtRectangle *result = new QtRectangle( this, wc, blc_x, blc_y, trc_x, trc_y, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Rectangle>(result);
	}

	std::tr1::shared_ptr<Polygon> QtRegionSource::polygon( WorldCanvas *wc, double x1, double y1 ) {
	    QtPolygon *result = new QtPolygon( this, wc, x1, y1, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    return std::tr1::shared_ptr<Polygon>(result);
	    // return std::tr1::shared_ptr<Polygon>( );
	}

	std::tr1::shared_ptr<Polygon> QtRegionSource::polygon( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
	    QtPolygon *result = new QtPolygon( this, wc, pts, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Polygon>(result);
	    // return std::tr1::shared_ptr<Polygon>( );
	}

	std::tr1::shared_ptr<Rectangle> QtRegionSource::ellipse( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
	    QtEllipse *result = new QtEllipse( this, wc, blc_x, blc_y, trc_x, trc_y, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Rectangle>(result);
	}

	std::tr1::shared_ptr<Rectangle> QtRegionSource::point( WorldCanvas *wc, double x, double y ) {
	    QtPoint *result = new QtPoint( this, wc, x, y, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Rectangle>(result);
	}

	QtRegionDock *QtRegionSource::dock( ) { return panel_->regionDock( ); }
	int QtRegionSource::numFrames( ) const { return panel_->numFrames( ); }

	void QtRegionSource::loadRegions( bool &handled, const QString &path, const QString &type ) {
	    if ( ! handled ) {
		handled = true;
		ConstListIter<WorldCanvas*> wcl = panel_->displayPanel()->panelDisplay()->myWCLI;
		wcl.toStart( );
		// really need to change this to find the world canvas for the display data that is currently visible...
		// instead of just taking the first one in the list...  <<<see also QtDisplayPanel.cc>>>
		WorldCanvas *wc_ = 0;
		if(wcl.len() > 0) {
		    wc_ = wcl.getRight( );
		}
		if ( wc_ ) {
		    if ( type.compare("CASA region file") == 0 ) {
			load_crtf_regions( wc_, path );
		    } else if ( type.compare("DS9 region file") == 0 ) {
			casa::viewer::ds9context context( wc_ );
			casa::viewer::ds9parser parser;
			parser.parse_file( context, path.toAscii( ).constData( ) );
		    } else {
			fprintf( stderr, "QtRegionSource::loadRegions( bool &handled, const QString &path, const QString &type ):\n" );
			fprintf( stderr, "\tinternal error, invalid type: %s...\n", type.toAscii( ).constData( ) );
		    }
		} else {
		    fprintf( stderr, "QtRegionSource::loadRegions( bool &handled, const QString &path, const QString &type ):\n" );
		    fprintf( stderr, "\tinternal error, no world canvas...\n" );
		}
	    }
	}

	void QtRegionSource::load_crtf_regions( WorldCanvas *wc, const QString &path ) {
	    // find viewers's coordinate system type...
	    const CoordinateSystem &viewer_cs = wc->coordinateSystem( );
	    MDirection::Types cstype = get_coordinate_type( viewer_cs );
	    if ( cstype == MDirection::EXTRA ) return;

	    // need shape(?!?!) to get the annotation region list...
	    const DisplayData *dd = wc->displaylist().front();
	    std::vector<int> axes = dd->displayAxes( );
	    IPosition shape(viewer_cs.nPixelAxes( ));
	    for ( int i=0; i < shape.size( ); ++i )
		shape(i) = dd->dataShape( )[axes[i]];

	    try {
		RegionTextList region_list( path.toAscii( ).constData( ), viewer_cs, shape );
		const casa::Vector<AsciiAnnotationFileLine> &lines = region_list.getLines( );
		for ( uInt i=0; i < lines.size( ); ++i ) {
		    if ( lines[i].getType() == AsciiAnnotationFileLine::ANNOTATION ) {
			const AnnotationBase* annotation = lines[i].getAnnotationBase();
			const AnnRectBox *rectangle=0;
			const AnnEllipse *ellipse=0;
			const AnnSymbol *symbol=0;
			const AnnPolygon *polygon = 0;
			if ( rectangle=dynamic_cast<const AnnRectBox*>(annotation) ) {
			    load_crtf_rectangle( wc, cstype, rectangle );
			} else if ( ellipse = dynamic_cast<const AnnEllipse*>(annotation) ) {
			    load_crtf_ellipse( wc, cstype, ellipse );
			} else if ( symbol = dynamic_cast<const AnnSymbol*>(annotation) ) {
			    load_crtf_point( wc, cstype, symbol );
			} else if ( polygon = dynamic_cast<const AnnPolygon*>(annotation) ) {
			    load_crtf_polygon( wc, cstype, polygon );
			}
		    }
		}
	    } catch(...) {
		fprintf( stderr, "loading of %s failed...\n", path.toAscii( ).constData( ) );
		return;
	    }
	}

	inline int get_font_style( AnnotationBase::FontStyle annfontstyle ) {
	    return ( annfontstyle == AnnotationBase::NORMAL ? 0 :
		     annfontstyle == AnnotationBase::BOLD ? Region::BoldText :
		     annfontstyle == AnnotationBase::ITALIC ? Region::ItalicText :
		     annfontstyle == AnnotationBase::ITALIC_BOLD ? (Region::BoldText | Region::ItalicText) : 0 );
	}

	inline Region::LineStyle get_line_style( AnnotationBase::LineStyle annlinestyle ) {
	    return ( annlinestyle == AnnotationBase::SOLID ? Region::SolidLine :
		     annlinestyle == AnnotationBase::DASHED ? Region::DashLine :
		     annlinestyle == AnnotationBase::DOTTED ? Region::DotLine : Region::SolidLine );
	}

	void QtRegionSource::load_crtf_rectangle( WorldCanvas *wc, MDirection::Types cstype, const AnnRectBox *rectangle ) {

	    MDirection::Types ann_cstype = get_coordinate_type( rectangle->getCsys( ) );
	    if ( ann_cstype == MDirection::EXTRA ) return;

	    // get BLC, TRC as quantities... <mdirection>
	    casa::Vector<MDirection> corners = rectangle->getCorners();
	    if ( corners.size() != 2 ) return;

	    // convert to the viewer's world coordinates... <mdirection>
	    MDirection blcmd = MDirection::Convert(MDirection(corners[0].getAngle("rad"),ann_cstype), cstype)();
	    casa::Quantum<casa::Vector<double> > blcq = blcmd.getAngle("rad");
	    MDirection trcmd = MDirection::Convert(MDirection(corners[1].getAngle("rad"),ann_cstype), cstype)();
	    casa::Quantum<casa::Vector<double> > trcq = trcmd.getAngle("rad");
	    std::vector<std::pair<double,double> > pts(2);
	    world_to_linear( wc,blcq.getValue( )(0),blcq.getValue( )(1),trcq.getValue( )(0),trcq.getValue( )(1),
			     pts[0].first, pts[0].second, pts[1].first, pts[1].second );

	    // create the rectangle...
	    const RegionCreator::creator_list_type &rect_creators = RegionCreator::findCreator( Region::RectRegion );
	    if ( rect_creators.size( ) <= 0 ) return;
	    int font_style = get_font_style(rectangle->getFontStyle());
	    Region::LineStyle line_style = get_line_style(rectangle->getLineStyle( ));

	    rect_creators.front( )->create( Region::RectRegion, wc, pts, rectangle->getLabel( ), rectangle->getFont( ),
					    rectangle->getFontSize( ), font_style, rectangle->getLabelColorString( ),
					    rectangle->getColorString( ), line_style, rectangle->isAnnotationOnly( ) );
	}


	void QtRegionSource::load_crtf_ellipse( WorldCanvas *wc, MDirection::Types cstype, const AnnEllipse *ellipse ) {
	    MDirection::Types ann_cstype = get_coordinate_type( ellipse->getCsys( ) );
	    if ( ann_cstype == MDirection::EXTRA ) return;

	    // convert to the viewer's world coordinates... <mdirection>
	    MDirection dir_center = MDirection::Convert(ellipse->getCenter( ), cstype)();
	    casa::Vector<double> center = dir_center.getAngle("rad").getValue( );
	    // 90 deg around 0 & 180 deg
	    const double major_radius = ellipse->getMajorAxis().getValue("rad") / 2.0;
	    const double minor_radius = ellipse->getMinorAxis().getValue("rad") / 2.0;
	    const double pos_angle = ellipse->getPositionAngle( ).getValue("deg");
	    const bool x_is_major = ((pos_angle > 45.0 && pos_angle < 135.0) ||
			       (pos_angle > 225.0 && pos_angle < 315.0));
	    const double xradius = (x_is_major ? major_radius : minor_radius);
	    const double yradius = (x_is_major ? minor_radius : major_radius);

	    std::vector<std::pair<double,double> > pts(2);
	    world_to_linear(wc,center[0]-xradius,center[1]-yradius,center[0]+xradius,center[1]+yradius, pts[0].first, pts[0].second, pts[1].first, pts[1].second);
	    
	    // create the ellipse...
	    const RegionCreator::creator_list_type &ellipse_creators = RegionCreator::findCreator( Region::EllipseRegion );
	    if ( ellipse_creators.size( ) <= 0 ) return;
	    int font_style = get_font_style(ellipse->getFontStyle());
	    Region::LineStyle line_style = get_line_style(ellipse->getLineStyle( ));

	    ellipse_creators.front( )->create( Region::EllipseRegion, wc, pts, ellipse->getLabel( ), ellipse->getFont( ),
					    ellipse->getFontSize( ), font_style, ellipse->getLabelColorString( ),
					    ellipse->getColorString( ), line_style, ellipse->isAnnotationOnly( ) );
	}


	void QtRegionSource::load_crtf_point( WorldCanvas *wc, MDirection::Types cstype, const AnnSymbol *symbol ) {

	    MDirection::Types ann_cstype = get_coordinate_type( symbol->getCsys( ) );
	    if ( ann_cstype == MDirection::EXTRA ) return;

	    // get point
	    MDirection dir_point = MDirection::Convert(symbol->getDirection( ), cstype)();
	    casa::Vector<double> point = dir_point.getAngle("rad").getValue( );
	    
	    std::vector<std::pair<double,double> > pts(2);
	    world_to_linear( wc, point[0], point[1], pts[0].first, pts[0].second );
	    pts[1] = pts[0];	// points also have two corners...

	    // create the point...
	    const RegionCreator::creator_list_type &point_creators = RegionCreator::findCreator( Region::PointRegion );
	    if ( point_creators.size( ) <= 0 ) return;
	    int font_style = get_font_style(symbol->getFontStyle());
	    Region::LineStyle line_style = get_line_style(symbol->getLineStyle( ));

	    point_creators.front( )->create( Region::PointRegion, wc, pts, symbol->getLabel( ), symbol->getFont( ),
					     symbol->getFontSize( ), font_style, symbol->getLabelColorString( ),
					     symbol->getColorString( ), line_style, false );
	}

	void QtRegionSource::load_crtf_polygon( WorldCanvas *wc, MDirection::Types cstype, const AnnPolygon *polygon ) {

	    MDirection::Types ann_cstype = get_coordinate_type( polygon->getCsys( ) );
	    if ( ann_cstype == MDirection::EXTRA ) return;

	    casa::Vector<MDirection> corners = polygon->getCorners();
	    if ( corners.size() < 3 ) return;

	    std::vector<std::pair<double,double> > pts(corners.size());
	    for ( int i=0; i < corners.size( ); ++i ) {
		MDirection corner = MDirection::Convert(corners[i], cstype)();
		casa::Vector<double> point = corner.getAngle("rad").getValue( );
		world_to_linear( wc, point[0], point[1], pts[i].first, pts[i].second );
	    }

	    // create the polygon...
	    const RegionCreator::creator_list_type &poly_creators = RegionCreator::findCreator( Region::PolyRegion );
	    if ( poly_creators.size( ) <= 0 ) return;
	    int font_style = get_font_style(polygon->getFontStyle());
	    Region::LineStyle line_style = get_line_style(polygon->getLineStyle( ));

	    poly_creators.front( )->create( Region::PolyRegion, wc, pts, polygon->getLabel( ), polygon->getFont( ),
					    polygon->getFontSize( ), font_style, polygon->getLabelColorString( ),
					    polygon->getColorString( ), line_style, polygon->isAnnotationOnly( ) );
	}

    }
}

