//# QtRegionDock.cc: dockable Qt implementation of viewer region management
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


#include <fstream>
#include <iostream>
#include <algorithm>
#include <display/region/QtRegionDock.qo.h>
#include <display/region/QtRegionState.qo.h>
#include <display/region/QtRegion.qo.h>
#include <display/ds9/ds9writer.h>
#include <display/DisplayErrors.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>

namespace casa {
    namespace viewer {
	QtRegionDock::QtRegionDock( QtDisplayPanelGui *d, QWidget* parent ) :
					QDockWidget(parent), Ui::QtRegionDock( ),
					dpg(d), current_dd(0), current_tab_state(-1,-1),
					current_color_index(6), /*** magenta ***/
					dismissed(false) {
	    setupUi(this);

	    // there are two standard Qt, dismiss icons...
	    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
	    // dismissRegion->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
	    dismiss_region->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
	    reset_regions->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

	    // Qt Designer will not allow you to create an empty QStackedWidget (qt4.7)... all created
	    // stacked widgets seem to have two elements... here we remove that elements (if they
	    // exists) because we want the elements to appear as they are created by the user...
	    while ( region_stack->count( ) > 0 ) {
		QWidget *c = region_stack->currentWidget( );
		region_stack->removeWidget(c);
		delete c;
	    }

	    connect( region_stack, SIGNAL(currentChanged(int)), SLOT(stack_changed(int)) );
	    connect( region_stack, SIGNAL(widgetRemoved(int)), SLOT(stack_changed(int)) );

	    connect( region_scroll, SIGNAL(valueChanged(int)), SLOT(change_stack(int)) );

	    connect( dismiss_region, SIGNAL(clicked(bool)), SLOT(delete_current_region(bool)) );
	    connect( reset_regions, SIGNAL(clicked(bool)), SLOT(delete_all_regions(bool)) );
	    connect( this, SIGNAL(visibilityChanged(bool)), SLOT(handle_visibility(bool)) );
	    connect( region_stack, SIGNAL(currentChanged(int)), SLOT(emit_region_stack_change(int)) );

	}
  
	QtRegionDock::~QtRegionDock() {  }

	// void QtRegionDock::showStats( const QString &stats ) { }

	void QtRegionDock::addRegion(QtRegion *r, QtRegionState *state, int index) {

	    // book keeping for DataManager region access...
	    region_list.push_back(r);
	    region_map.insert(region_map_t::value_type(state,r));

	    if ( index >= 0 ) {
		region_stack->insertWidget( index, state );
	    } else {
		region_stack->addWidget(state);
	    }
	    region_stack->setCurrentWidget(state);

	    emit regionChange( r, "created" );

	    connect( r, SIGNAL(regionChange(viewer::QtRegion*,std::string)), this, SIGNAL(regionChange(viewer::QtRegion*,std::string)));
	    connect( state, SIGNAL(outputRegions(const QString &,const QString &,const QString&,const QString&)), SLOT(output_region_event(const QString &,const QString &,const QString&,const QString&)) );
	    connect( state, SIGNAL(loadRegions(bool&,const QString &,const QString &)), SIGNAL(loadRegions(bool&,const QString &,const QString&)) );
	    connect( this, SIGNAL(region_stack_change(QWidget*)), state, SLOT(stackChange(QWidget*)) );

	    // not needed if the dock starts out as visible (or in user control)
	    // if ( ! isVisible( ) && dismissed == false ) {
	    // 	show( );
	    // }
	}

	int QtRegionDock::indexOf(QtRegionState *state) const {
	    return region_stack->indexOf(state);
	}

	void QtRegionDock::removeRegion(QtRegionState *state) {

	    QtRegion *gonner=0;
	    // clean up book keeping for DataManager region access...
	    region_map_t::iterator map_it = region_map.find(state);
	    if ( map_it != region_map.end( ) ) {
		region_list_t::iterator list_it = std::find(region_list.begin( ), region_list.end( ), map_it->second);
		if ( list_it != region_list.end( ) ) {
		    region_list.erase(list_it);
		}
		gonner = map_it->second;
		region_map.erase(map_it);
	    }

	    if ( region_stack->indexOf(state) != -1 ) {
		// disconnect signals from 'state' to this object...
		disconnect( state, 0, this, 0 );
	    }
	    region_stack->removeWidget(state);
	    if ( gonner ) emit regionChange( gonner, "deleted" );
	}

	void QtRegionDock::selectRegion(QtRegionState *state) {
	    region_stack->setCurrentWidget(state);
	    state->nowVisible( );
	}

	void QtRegionDock::dismiss( ) {
	    hide( );
	    dismissed = true;
	}

	// zero length string indicates OK!
	std::string QtRegionDock::outputRegions( std::list<viewer::QtRegionState*> regionstate_list, std::string file, std::string format, std::string csys ) {
	    if ( regionstate_list.size( ) > 0 ) {
		if ( format == "crtf" ) {
		    AnnRegion::unitInit();
		    RegionTextList annotation_list;
		    try { emit saveRegions(regionstate_list,annotation_list); } catch (...) { return "encountered error"; }
		    ofstream sink;
		    sink.open(file.c_str( ));
		    annotation_list.print(sink);
		    sink.close( );
		} else if ( format == "ds9" ) {
		    ds9writer writer(file.c_str( ),csys.c_str( ));
		    try { emit saveRegions(regionstate_list,writer); } catch (...) { return "encountered error"; }
		} else {
		    return "invalid format";
		}
		return "";
	    } else {
		return "no regions to write out";
	    }
	}

	void QtRegionDock::change_stack( int index ) {
	    int size = region_stack->count( );
	    if ( index >= 0 && index <= size - 1 )
		region_stack->setCurrentIndex( index );
	}

	void QtRegionDock::updateRegionState( QtDisplayData *dd ) {
	    if ( dd == 0 && current_dd == 0 ) return;
	    if ( current_dd != 0 && dd == 0 )
		region_stack->hide( );
	    else if ( current_dd == 0 && dd != 0 )
		region_stack->show( );

	    for ( int i = 0; i < region_stack->count( ); ++i ) {
		QWidget *widget = region_stack->widget( i );
		QtRegionState *state = dynamic_cast<QtRegionState*>(widget);
		if ( state != 0 ) state->updateCoord( );
	    }

	    current_dd = dd;
	}

	void QtRegionDock::stack_changed( int index ) {

	    static int last_index = -1;
	    int size = region_stack->count( );
	    region_scroll->setMaximum( size > 0 ? size - 1 : 0 );
	    if ( size <= 0 ) {
		region_scroll->setEnabled(false);
		dismiss_region->setEnabled(false);
		reset_regions->setEnabled(false);
	    } else if ( size == 1 ) {
		region_scroll->setEnabled(false);
		dismiss_region->setEnabled(true);
		reset_regions->setEnabled(true);
	    } else {
		region_scroll->setEnabled(true);
		dismiss_region->setEnabled(true);
		reset_regions->setEnabled(true);
	    }
	    QWidget *current_widget = region_stack->currentWidget( );
	    if ( current_widget == 0 ) {
		// box is empty... e.g. after being deleted while updating
		region_scroll->setEnabled(false);
		dismiss_region->setEnabled(false);
		last_index = 0;
		return;
	    }

	    int current_index = region_stack->currentIndex( );
	    if ( current_index >= 0 )
		region_scroll->setValue(current_index);

	    QtRegionState *state = dynamic_cast<QtRegionState*>(current_widget);
	    if ( state == 0 )
		throw internal_error("region state corruption");
	    state->justExposed( );
	    last_index = index;
	}

	void QtRegionDock::delete_current_region(bool) {
	    emit deleteRegion(dynamic_cast<QtRegionState*>(region_stack->currentWidget( )));
	}

	void QtRegionDock::delete_all_regions(bool) {
	    emit deleteAllRegions( );
	}

      void QtRegionDock::output_region_event( const QString &what, const QString &where, const QString &type, const QString &csys ) {
	    std::list<QtRegionState*> regionstate_list;
	    if ( what == "current" ) {
		// current region, only...
		QWidget *current_widget = region_stack->currentWidget( );
		QtRegionState *current = dynamic_cast<QtRegionState*>(current_widget);
		if ( current != 0 )
		    regionstate_list.push_back(current);
	    } else if ( what == "marked" ) {
		// all marked regions...
		for ( int i = 0; i < region_stack->count( ); ++i ) {
		    QWidget *widget = region_stack->widget( i );
		    QtRegionState *state = dynamic_cast<QtRegionState*>(widget);
		    if ( state != 0 && state->marked( ) )
			regionstate_list.push_back(state);
		}
	    } else {
		// ("all")... all regions...
		for ( int i = 0; i < region_stack->count( ); ++i ) {
		    QWidget *widget = region_stack->widget( i );
		    QtRegionState *state = dynamic_cast<QtRegionState*>(widget);
		    if ( state != 0 )
			regionstate_list.push_back(state);
		}
	    }

	    if ( regionstate_list.size( ) > 0 ) {
		if ( type == "CASA region file" ) {
		    AnnRegion::unitInit();
		    RegionTextList annotation_list;
		    try { emit saveRegions(regionstate_list,annotation_list); } catch (...) { return; }
		    ofstream sink;
		    sink.open(where.toAscii( ).constData( ));
		    annotation_list.print(sink);
		    sink.close( );
		} else if ( type == "DS9 region file" ) {
		    ds9writer writer(where.toAscii( ).constData( ),csys.toAscii( ).constData( ));
		    try { emit saveRegions(regionstate_list,writer); } catch (...) { return; }

		}
	    } else {
		QWidget *current_widget = region_stack->currentWidget( );
		QtRegionState *current = dynamic_cast<QtRegionState*>(current_widget);
		if ( current != 0 )
		    current->noOutputNotify( );
	    }
	}

	void QtRegionDock::handle_visibility( bool visible ) {
	    if ( visible && dismissed ) {
		dismissed = false;
		dpg->putrc( "visible.regiondock", "true" );
	    }
	}

	void QtRegionDock::emit_region_stack_change( int index ) {
	    emit region_stack_change(region_stack->widget(index));
	}

	void QtRegionDock::closeEvent ( QCloseEvent * event ) {
	    dismissed = true;
	    QDockWidget::closeEvent(event);
	    dpg->putrc( "visible.regiondock", "false" );
	}
    }
}
