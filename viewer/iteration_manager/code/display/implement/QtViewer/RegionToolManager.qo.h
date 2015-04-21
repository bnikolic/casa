//# RegionToolManager.qo.h: class designed to unify the behavior of all of the mouse tools
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
//# $Id$

#ifndef DISPLAY_TOOLMANAGER_H__
#define DISPLAY_TOOLMANAGER_H__

#include <QObject>
#include <display/Display/PanelDisplay.h>
#include <display/region/QtRegionSourceFactory.h>

#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/WCRefreshEH.h>

#include <display/QtPlotter/QtMWCTools.qo.h>

namespace casa {

    /* class QtMWCTool; */
    class QtMouseTool;
    class QtRectTool;

    namespace viewer {

	// the RegionCreator class now has static members which allow for the
	// retrieval of lists of RegionCreator objects for creating each type
	// of region (currently rectangle, polygon, ellipse, and point).
	class RegionToolManager : public QObject,
			    public WCPositionEH,
	                    public WCMotionEH,
	                    public WCRefreshEH {
	    Q_OBJECT
	    public:

		enum ToolTypes { RectTool, PointTool, EllipseTool, PolyTool };

		RegionToolManager( QtRegionSourceFactory *rsf, PanelDisplay *pd );
		~RegionToolManager( );

		// Required operators for event handling - these are called when
		// events occur, and distribute the events to the "user-level"
		// methods
		// <group>
		void operator()(const WCPositionEvent& ev);
		void operator()(const WCMotionEvent& ev);
		void operator()(const WCRefreshEvent& ev);
		// </group>

		void loadRegions( const std::string &path, const std::string &datatype, const std::string &displaytype );

	    private:
		typedef std::set<viewer::Region*> region_list_type;
		typedef std::pair<double,double> linear_point_type;
		PanelDisplay *pd;
		typedef std::map<ToolTypes,RegionTool*> tool_map;
		tool_map tools;

		// members for keeping track of marked (sticky-selected) regions...
		region_list_type marked_regions;

		// members for state when moving selected region...
		bool inDrawArea( WorldCanvas *wc, const linear_point_type &new_blc, const linear_point_type &new_trc ) const;
		region_list_type moving_regions;
		linear_point_type moving_ref_point;
		linear_point_type moving_blc;
		linear_point_type moving_trc;

		// members for state when moving a region handle...
		bool moving_handle;
		viewer::Region::PointInfo moving_handle_info;
		viewer::Region *moving_handle_region;

		bool add_mark_select( RegionTool::State &state );
		void clear_mark_select( RegionTool::State &state );
		bool setup_moving_regions( RegionTool::State &state );
		void setup_moving_regions_state( double linx, double liny );
		void translate_moving_regions( WorldCanvas *wc, double dx, double dy );
	};
    }
}


#endif