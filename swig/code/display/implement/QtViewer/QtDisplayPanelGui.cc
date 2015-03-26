//# QtDisplayPanelGui.cc: Qt implementation of main viewer display window.
//# with surrounding Gui functionality
//# Copyright (C) 2005,2009
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
//# $Id: QtDisplayPanelGui.cc,v 1.12 2006/10/10 21:59:19 dking Exp $

#include <algorithm>
#include <string> 
#include <casa/BasicSL/String.h>
#include <display/Utilities/StringUtil.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtViewerPrintGui.qo.h>
#include <display/QtViewer/QtCanvasManager.qo.h>
#include <display/QtViewer/QtRegionManager.qo.h>
#include <display/QtViewer/MakeMask.qo.h>
#include <display/QtViewer/FileBox.qo.h>
#include <display/QtViewer/MakeRegion.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtMouseToolBar.qo.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtPlotter/QtProfile.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDataManager.qo.h>
#include <display/QtViewer/QtExportManager.qo.h>
#include <display/QtViewer/QtDataOptionsPanel.qo.h>
#include <display/RegionShapes/QtRegionShapeManager.qo.h>
#include <display/QtViewer/QtWCBox.h>
#include <display/QtViewer/Preferences.qo.h>
#include <display/region/QtRegionSource.qo.h>

namespace casa { //# NAMESPACE CASA - BEGIN

bool QtDisplayPanelGui::logger_did_region_warning = false;;

QtDisplayPanelGui::QtDisplayPanelGui(QtViewer* v, QWidget *parent, std::string rcstr, const std::list<std::string> &args ) :
		   QtPanelBase(parent), logger(LogOrigin("CASA", "Viewer")), qdm_(0),qem_(0),qdo_(0),
		   colorBarsVertical_(True), v_(v), qdp_(0), qpm_(0), qcm_(0), qap_(0), qfb_(0), qmr_(0), qrm_(0), 
		   qsm_(0), qst_(0),
                   profile_(0), savedTool_(QtMouseToolNames::NONE),
		   profileDD_(0),
		   annotAct_(0), mkRgnAct_(0), fboxAct_(0), rgnMgrAct_(0), shpMgrAct_(0),
		   regionDock_(0), rc(viewer::getrc()), rcid_(rcstr), use_new_regions(true),
		   showdataoptionspanel_enter_count(0),
		   controlling_dd(0), preferences(0), autoDDOptionsShow(True) {

    // initialize the "pix" unit, et al...
    QtWCBox::unitInit( );
    
    setWindowTitle("Viewer Display Panel");
    use_new_regions = std::find(args.begin(),args.end(),"--oldregions") == args.end();
    const char default_dock_location[] = "right";

    std::string apos = rc.get("viewer." + rcid() + ".position.animator");
    std::transform(apos.begin(), apos.end(), apos.begin(), ::tolower);
    if ( apos != "bottom" && apos != "right" && apos != "left" && apos != "top" ) {
	rc.put( "viewer." + rcid() + ".position.animator", default_dock_location );
    }
    std::string tpos = rc.get("viewer." + rcid() + ".position.cursor_tracking");
    std::transform(tpos.begin(), tpos.end(), tpos.begin(), ::tolower);
    if ( tpos != "bottom" && tpos != "right" && tpos != "left" && tpos != "top" ) {
	rc.put( "viewer." + rcid() + ".position.cursor_tracking", default_dock_location );
    }
    std::string rpos = rc.get("viewer." + rcid() + ".position.regions");
    std::transform(rpos.begin(), rpos.end(), rpos.begin(), ::tolower);
    if ( rpos != "bottom" && rpos != "right" && rpos != "left" && rpos != "top" ) {
	rc.put( "viewer." + rcid() + ".position.regions", default_dock_location );
    }

    if ( use_new_regions ) {
	// -----
	// This must be created here, because the process of (a) constructing a QtDisplayPanel,
	// (b) creates a QtRegionCreatorSource, which (c) uses the constructed QtDisplayPanel, to
	// (d) retrieve the QToolBox which is part of this QtRegionDock... should fix... <drs>
	regionDock_  = new viewer::QtRegionDock(this);
	connect( this, SIGNAL(axisToolUpdate(QtDisplayData*)), regionDock_, SLOT(updateRegionState(QtDisplayData*)) );	
	std::string shown = getrc("visible.regiondock");
	std::transform(shown.begin(), shown.end(), shown.begin(), ::tolower);
	if ( shown == "false" ) regionDock_->dismiss( );
    }

    qdp_ = new QtDisplayPanel(this,0,args);
//  qdo_ = new QtDataOptionsPanel(this);
  
    setCentralWidget(qdp_);
  
    setFocusProxy(qdp_);	// Shifts panel kbd focus to qdp_, which
				// in turn shifts it to PixelCanvas.
				// PC/WC respond to some keystrokes
				// (e.g. arrow keys, esc, space...).
				// Note: this will not steal focus from,
				// e.g., a text widget on the panel.

  
    qrm_ = new QtRegionManager(qdp_);
  
    qsm_ = new QtRegionShapeManager(qdp_);
    qsm_->setVisible(false);  
  
    qdp_->setShapeManager(qsm_);
 

    
    // SURROUNDING GUI LAYOUT  

    // Create the widgets (plus a little parenting and properties)
  
    ddMenu_       = menuBar()->addMenu("&Data");
    ddOpenAct_    = ddMenu_->addAction("&Open...");
    ddRegAct_     = ddMenu_->addAction("&Register");
    ddRegMenu_    = new QMenu; ddRegAct_->setMenu(ddRegMenu_);
    ddCloseAct_   = ddMenu_->addAction("&Close");
    ddCloseMenu_  = new QMenu; ddCloseAct_->setMenu(ddCloseMenu_);
    ddAdjAct_     = ddMenu_->addAction("&Adjust...");
    ddSaveAct_    = ddMenu_->addAction("Sa&ve as...");
		    ddMenu_->addSeparator();
    printAct_     = ddMenu_->addAction("&Print...");
		    ddMenu_->addSeparator();
    dpSaveAct_    = ddMenu_->addAction("&Save Panel State...");
    dpRstrAct_    = ddMenu_->addAction("Restore Panel State...");
		    ddMenu_->addSeparator();
    ddPreferencesAct_ = ddMenu_->addAction("Preferences...");
		    ddMenu_->addSeparator();
    dpCloseAct_   = ddMenu_->addAction("&Close Panel");
    dpQuitAct_    = ddMenu_->addAction("&Quit Viewer");

    dpMenu_       = menuBar()->addMenu("D&isplay Panel");
    dpNewAct_     = dpMenu_->addAction("&New Panel");
    dpOptsAct_    = dpMenu_->addAction("Panel &Options...");
                    dpMenu_->addAction(dpSaveAct_);
		    dpMenu_->addAction(dpRstrAct_);
                    dpMenu_->addAction(printAct_);
		    dpMenu_->addSeparator();
                    dpMenu_->addAction(dpCloseAct_);
  
    tlMenu_       = menuBar()->addMenu("&Tools");
    if ( ! use_new_regions ) {
	fboxAct_      = tlMenu_->addAction("&Box in File");
	annotAct_     = tlMenu_->addAction("Region in &File");
	mkRgnAct_     = tlMenu_->addAction("Region in &Image");
//                  annotAct_->setEnabled(False);
	connect(fboxAct_,    SIGNAL(triggered()),  SLOT(showFileBoxPanel()));
	connect(annotAct_,   SIGNAL(triggered()),  SLOT(showAnnotatorPanel()));
	connect(mkRgnAct_,   SIGNAL(triggered()),  SLOT(showMakeRegionPanel()));
    }

    profileAct_   = tlMenu_->addAction("Spectral Profi&le...");
    // rgnMgrAct_    = new QAction("Region Manager...", 0);
                    // rgnMgrAct_->setEnabled(False);

    if ( ! use_new_regions ) {
	shpMgrAct_    = tlMenu_->addAction("Shape Manager...");
	connect(shpMgrAct_,  SIGNAL(triggered()),  SLOT(showShapeManager()));
    }
  
    vwMenu_       = menuBar()->addMenu("&View");
			// (populated after creation of toolbars/dockwidgets).
  
    mainToolBar_  = addToolBar("Main Toolbar");
		    mainToolBar_->setObjectName("Main Toolbar");
		    mainToolBar_->addAction(ddOpenAct_);
		    mainToolBar_->addAction(ddAdjAct_);
    ddRegBtn_     = new QToolButton(mainToolBar_);
		    mainToolBar_->addWidget(ddRegBtn_);
		    ddRegBtn_->setMenu(ddRegMenu_);
    ddCloseBtn_   = new QToolButton(mainToolBar_);
		    mainToolBar_->addWidget(ddCloseBtn_);
		    ddCloseBtn_->setMenu(ddCloseMenu_);
		    mainToolBar_->addAction(ddSaveAct_);
		    mainToolBar_->addSeparator();
		    mainToolBar_->addAction(dpNewAct_);
		    mainToolBar_->addAction(dpOptsAct_);
		    mainToolBar_->addAction(dpSaveAct_);
		    mainToolBar_->addAction(dpRstrAct_);
		    mainToolBar_->addSeparator();
		    mainToolBar_->addAction(profileAct_);
//		    mainToolBar_->addAction(rgnMgrAct_);
		    mainToolBar_->addSeparator();
		    mainToolBar_->addAction(printAct_);
		    mainToolBar_->addSeparator();
    unzoomAct_    = mainToolBar_->addAction("Un&zoom");
    zoomInAct_    = mainToolBar_->addAction("Zoom &In");
    zoomOutAct_   = mainToolBar_->addAction("Zoom O&ut");

  
    std::string mbpos = rc.get("viewer." + rcid() + ".position.mousetools");
    std::transform(mbpos.begin(), mbpos.end(), mbpos.begin(), ::tolower);
    mouseToolBar_ = new QtMouseToolBar(v_->mouseBtns(), qdp_);
		    mouseToolBar_->setObjectName("Mouse Toolbar");
		    mouseToolBar_->setAllowedAreas( (Qt::ToolBarArea) ( Qt::LeftToolBarArea | Qt::TopToolBarArea |
									Qt::RightToolBarArea | Qt::BottomToolBarArea ) );
		    addToolBarBreak();
		    addToolBar( mbpos == "left" ? Qt::LeftToolBarArea :
				mbpos == "right" ? Qt::RightToolBarArea :
				mbpos == "bottom" ? Qt::BottomToolBarArea :
				Qt::TopToolBarArea, mouseToolBar_);

#if 0
//  addToolBarBreak();
    QToolBar *atb =  addToolBar("another toolbar");
    QToolButton *atb_play = new QToolButton(atb);
    atb->addWidget(atb_play);
    const QIcon icon4 = QIcon(QString::fromUtf8(":/icons/Anim4_Play.png"));
    atb_play->setIcon(icon4);
    atb_play->setIconSize(QSize(22, 22));
    atb_play->resize(QSize(36, 36));
    atb_play->setCheckable(True);
    atb_play->setEnabled(True);
    connect(atb_play, SIGNAL(clicked()),           SLOT(fwdPlay_()));
#endif

    // This tool bar is _public_; programmer can add custom interface to it.
    customToolBar    = addToolBar("Custom Toolbar");
		       customToolBar->setObjectName("Custom Toolbar");
		       customToolBar->hide();	// (hidden by default).
		       customToolBar->toggleViewAction()->setVisible(False);
			// This can also be reversed: controls visibility
			// of this toolbar in the 'View' (or rt.-click) menu.

    // Another public tool bar; This one in a separate row, will fit more.
		       addToolBarBreak();
    customToolBar2   = addToolBar("Custom Toolbar 2");
		       customToolBar2->setObjectName("Custom Toolbar 2");
		       customToolBar2->hide();
		       customToolBar2->toggleViewAction()->setVisible(False);
       
    std::string animloc = rc.get("viewer." + rcid() + ".position.animator");
    std::transform(animloc.begin(), animloc.end(), animloc.begin(), ::tolower);
    animDockWidget_  = new QDockWidget();
		       animDockWidget_->setObjectName("Animator");
		       addDockWidget( animloc == "right" ? Qt::RightDockWidgetArea :
				      animloc == "left" ? Qt::LeftDockWidgetArea :
				      animloc == "top" ? Qt::TopDockWidgetArea :
				      Qt::BottomDockWidgetArea, animDockWidget_, Qt::Vertical );
   
    animWidget_      = new QFrame;	
		       animDockWidget_->setWidget(animWidget_);

    Ui::QtAnimatorGui::setupUi(animWidget_);  
	// creates/inserts animator controls into animWidget_.
	// These widgets (e.g. frameSlider_) are protected members
	// of Ui::QtAnimatorGui, accessible to this derived class.
	// They are declared/defined in QtAnimatorGui.ui[.h].
   
    std::string trackloc = rc.get("viewer." + rcid() + ".position.cursor_tracking");
    std::transform(trackloc.begin(), trackloc.end(), trackloc.begin(), ::tolower);
    trkgDockWidget_  = new QDockWidget();
		       trkgDockWidget_->setObjectName("Position Tracking");
		       addDockWidget( trackloc == "right" ? Qt::RightDockWidgetArea :
				      trackloc == "left" ? Qt::LeftDockWidgetArea :
				      trackloc == "top" ? Qt::TopDockWidgetArea :
				      Qt::BottomDockWidgetArea, trkgDockWidget_, Qt::Vertical );
   
    trkgWidget_      = new QWidget;
		       trkgDockWidget_->setWidget(trkgWidget_);
	// trkgDockWidget_->layout()->addWidget(trkgWidget_);  // <-- no!
	// QDockWidgets create their own layout, and setWidget(trkgWidget_)
	// automatically puts trkgWidget_ into it...
	
	// ..._but_: _must_ create layout for trkgWidget_:...
		       new QVBoxLayout(trkgWidget_);
	// ..._and_ explicitly add trkgWidget_'s children to it as needed.
	// Note that parenting a TrackBox with trkgWidget_ does _not_
	// automatically do this....  For generic widgets (which don't have
	// methods like 'setWidget()' above, whose layout you create yourself) 
	// doing genericWidget->layout()->addWidget(childWidget) automatically
	// makes genericWidget the parent of childWidget, but _not_ vice
	// versa.  Also note: technically, the _layout_ is not the child
	// widget's 'parent'.



//  ------------------------------------------------------------------------------------------
    if ( regionDock_ ) {
	std::string rgnloc = rc.get("viewer." + rcid() + ".position.regions");
	std::transform(rgnloc.begin(), rgnloc.end(), rgnloc.begin(), ::tolower);
	addDockWidget( rgnloc == "right" ? Qt::RightDockWidgetArea :
			rgnloc == "left" ? Qt::LeftDockWidgetArea :
			rgnloc == "top" ? Qt::TopDockWidgetArea :
			Qt::BottomDockWidgetArea, regionDock_, Qt::Vertical );
    }

#if 0   
    if ( trackloc == "right" && animloc == "right" && rc.get("viewer." + rcid() + ".rightdock") == "tabbed" ) {
	tabifyDockWidget( animDockWidget_, trkgDockWidget_ );
    } else if ( trackloc == "left" && animloc == "left" && rc.get("viewer." + rcid() + ".leftdock") == "tabbed" ) {
	tabifyDockWidget( animDockWidget_, trkgDockWidget_ );
    } else if ( trackloc == "top" && animloc == "top" && rc.get("viewer." + rcid() + ".topdock") == "tabbed" ) {
	tabifyDockWidget( animDockWidget_, trkgDockWidget_ );
    } else if ( trackloc == "bottom" && animloc == "bottom" && rc.get("viewer." + rcid() + ".bottomdock") == "tabbed" ) {
	tabifyDockWidget( animDockWidget_, trkgDockWidget_ );
    }
#endif

//  ------------------------------------------------------------------------------------------
   
    // (This was going to be used for tracking....  May still be useful for
    // additions, or perhaps they should be in central widget, above QtPC).
    // bottomToolBar_   = new QToolBar();
    // 		         addToolBar(Qt::BottomToolBarArea, bottomToolBar_);
    //  bottomWidget_   = new QWidget;
    // 		         bottomToolBar_->addWidget(bottomWidget_);
    //   new QVBoxLayout(bottomWidget_);
    
  
  

     
    // REST OF THE PROPERTY SETTING 

  
    // The 'View' menu contains hide/show checkboxes for the tool bars
    // and dock widgets.  Thus it is populated here, after those tool
    // bars et. al. have been created.
  
    QMenu* rawVwMenu = createPopupMenu();
  
    if(rawVwMenu!=0) {
	    // (Beware: createPopupMenu() returns 0 rather than an empty
	    //  menu (<--as it should) if the window has no toolbars/dockwidgets.
	    //  For safety only: the menu shouldn't be empty in this case...).

	QList<QAction*> vwActions = rawVwMenu->actions();
    
	    // createPopupMenu() puts the dock widgets ahead of the tool bars by
	    // default, with a separator in between.  Reversing this is more
	    // intuitive for this application, since it puts them in the same order
	    // as the widgets appear on the display panel itself.  (All this could
	    // be replaced with the following if you didn't care about order): 
	    //
	    //   vwMenu_->addActions(createPopupMenu()->actions());
    
	Int nActions = vwActions.size();
	Int sep;	// Index of the separator 'action'.
	for(sep=0; sep<nActions; sep++) if(vwActions[sep]->isSeparator()) break;
    
	for(Int i=sep+1; i<nActions; i++) vwMenu_->addAction(vwActions[i]);
	if (0<sep && sep<nActions-1)      vwMenu_->addSeparator();
	for(Int i=0; i<sep; i++)          vwMenu_->addAction(vwActions[i]);
      
	delete rawVwMenu;
    }
    
  
    if(vwMenu_->actions().size()==0)
	vwMenu_->hide();


    //######################################
    //## Cursor Position Tracking
    //######################################

    trkgDockWidget_->setAllowedAreas((Qt::DockWidgetAreas)( Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea | 
							    Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea ));
  
//  trkgDockWidget_->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	// Prevents closing.  (Now that there's a 'View' menu making
	// it obvious how to get it back, this is no longer needed).
   
//  trkgDockWidget_->setWindowTitle("Position Tracking");
    trkgDockWidget_->toggleViewAction()->setText("Position Tracking");
	// Identifies this widget in the 'View' menu.
	// Preferred to previous line, which also displays the title
	// on the widget itself (not needed).
  
    trkgDockWidget_->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum);

    trkgWidget_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  
    trkgWidget_->layout()->setMargin(0);



    //######################################
    //## Animation
    //######################################
  
    animWidget_->setFrameShadow(QFrame::Plain);
    animWidget_->setFrameShape(QFrame::NoFrame);       // (invisible)
//  animWidget_->setFrameShape(QFrame::StyledPanel);
					// (to outline outer frame)
    animFrame_->setFrameShape(QFrame::StyledPanel);
					// (to outline inner frame (in Ui))
   

    animDockWidget_->setAllowedAreas((Qt::DockWidgetAreas)( Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea | 
							    Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea ));
  
    animDockWidget_->toggleViewAction()->setText("Animator");
  
    animDockWidget_->setSizePolicy( QSizePolicy::Minimum,	  // (horizontal)
				    QSizePolicy::Minimum);	  // (vertical)
    animWidget_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); 
	//#dk  For bug submission, was 'min,fixed' 4 dockWidg; notSet 4 animWidg.
	//# Main problem seems to be that dockWidget itself won't do 'fixed'
	//# correctly -- won't size down (or, sometimes, up) to contained
	//# widget's desired size (Qt task 94288, issue N93095).
  
    revTB_ ->setCheckable(True);
    stopTB_->setCheckable(True);
    playTB_->setCheckable(True);
  
    // rateEdit_ ->setReadOnly(True);
    // 	//#dk~ temporary only: no edits allowed here yet.

//  frameEdit_->setReadOnly(True);
    frameEdit_->setValidator(new QIntValidator(frameEdit_));
	// (Assures only validated Ints will signal editingFinished).
  
    // animAuxButton_->setToolTip( "Press 'Full' for more animation controls.\n"
    // 			      "Press 'Compact' to hide the extra controls." );
	// (...More animator widgets still need these help texts as well...)
  
    
    // Set interface according to the initial state of underlying animator.
  
    updateAnimUi_();
  
    // animAuxButton_->setText("Compact");
	// Puts animator initially in 'Full' configuration.
//  animAuxButton_->setText("Full");
	// (This would put it in 'Compact' configuration).
  
  
    // setAnimExtrasVisibility_();
	// Hides or shows extra animator widgets according to
	// the 'Compact/Full' button.
  
  
    // Trying to make dock area give back space anim. doesn't need.
    // Halleluia, this actually worked (Qt-4.1.3).  
    // (Also needed: setFixedHeight() (or fixed size policies throughout) 
    // in underlying widget(s)).
    // (Un-hallelujia, the Trolls broke it again in Qt-4.2.0.  Awaiting
    // further fixes...).		dk 11/06
    // See   http://www.trolltech.com/developer/task-tracker
    //             /index_html?method=entry&id=102107
    animDockWidget_->setSizePolicy( QSizePolicy::Minimum,
				    QSizePolicy::Fixed);	  // (needed)

    //#dg  animWidget_->setSizePolicy( QSizePolicy::Minimum,
    //#dg                              QSizePolicy::Fixed);  // (unneeded)
    //#dg  QApplication::processEvents();                   // (unneeded)
    //#dg  animDockWidget_->resize( animWidget_->minimumSizeHint());
                                             //#dg  (no luck either...)


/*    //#dg
    cerr <<"anMSzH:"<<animWidget_->minimumSizeHint().width()	    
         <<","<<animWidget_->minimumSizeHint().height()
         <<"   SzH:"<<animWidget_->sizeHint().width()	    
         <<","<<animWidget_->sizeHint().height()<<endl; 
    cerr <<"trMSzH:"<<trkgWidget_->minimumSizeHint().width()	    
         <<","<<trkgWidget_->minimumSizeHint().height()
         <<"   SzH:"<<trkgWidget_->sizeHint().width()	    
         <<","<<trkgWidget_->sizeHint().height()<<endl<<endl; 
//*/  //#dg  


/*    //#dg
    cerr <<"anWszPol:"<<animWidget_->sizePolicy().horizontalPolicy()	    
         <<","<<animWidget_->sizePolicy().verticalPolicy()<<endl; 
    cerr <<"anDszPol:"<<animDockWidget_->sizePolicy().horizontalPolicy() 
         <<","<<animDockWidget_->sizePolicy().verticalPolicy()<<endl;
    cerr <<"trEszPol:"<<trkgEdit_->sizePolicy().horizontalPolicy()	    
         <<","<<trkgEdit_->sizePolicy().verticalPolicy()<<endl; 
    cerr <<"trWszPol:"<<trkgWidget_->sizePolicy().horizontalPolicy()	    
         <<","<<trkgWidget_->sizePolicy().verticalPolicy()<<endl; 
    cerr <<"trDszPol:"<<trkgDockWidget_->sizePolicy().horizontalPolicy() 
         <<","<<trkgDockWidget_->sizePolicy().verticalPolicy()<<endl;
//*/  //#dg  

  
   
    // menus / toolbars
    
//  mainToolBar_->setIconSize(QSize(22,22));
    setIconSize(QSize(22,22));
 
    mainToolBar_->setMovable(False);
  
    ddOpenAct_ ->setIcon(QIcon(":/icons/File_Open.png"));
    ddSaveAct_ ->setIcon(QIcon(":/icons/Save_Img.png"));
    ddRegAct_  ->setIcon(QIcon(":/icons/DD_Register.png"));
    ddRegBtn_  ->setIcon(QIcon(":/icons/DD_Register.png"));
    ddCloseAct_->setIcon(QIcon(":/icons/File_Close.png"));
    ddCloseBtn_->setIcon(QIcon(":/icons/File_Close.png"));
    ddAdjAct_  ->setIcon(QIcon(":/icons/DD_Adjust.png"));
    dpNewAct_  ->setIcon(QIcon(":/icons/DP_New.png"));
    dpOptsAct_ ->setIcon(QIcon(":/icons/DP_Options.png"));
    dpSaveAct_ ->setIcon(QIcon(":/icons/Save_Panel.png"));
    dpRstrAct_ ->setIcon(QIcon(":/icons/Restore_Panel.png"));
    profileAct_->setIcon(QIcon(":/icons/Spec_Prof.png"));
    // rgnMgrAct_ ->setIcon(QIcon(":/icons/Region_Save.png"));
    printAct_  ->setIcon(QIcon(":/icons/File_Print.png"));
    unzoomAct_ ->setIcon(QIcon(":/icons/Zoom0_OutExt.png"));
    zoomInAct_ ->setIcon(QIcon(":/icons/Zoom1_In.png"));
    zoomOutAct_->setIcon(QIcon(":/icons/Zoom2_Out.png"));
    dpCloseAct_->setIcon(QIcon(":/icons/File_Close.png"));
    dpQuitAct_ ->setIcon(QIcon(":/icons/File_Quit.png"));
  
    ddOpenAct_ ->setToolTip("Open Data...");
    ddRegBtn_  ->setToolTip("[Un]register Data");
    ddCloseBtn_->setToolTip("Close Data");
    ddAdjAct_  ->setToolTip("Data Display Options");
    ddSaveAct_ ->setToolTip("Save as...");
    dpNewAct_  ->setToolTip("New Display Panel");
    dpOptsAct_ ->setToolTip("Panel Display Options");
    dpSaveAct_ ->setToolTip("Save Display Panel State to File");
    profileAct_->setToolTip("Open the Spectrum Profiler");
    dpRstrAct_ ->setToolTip("Restore Display Panel State from File");
    // rgnMgrAct_ ->setToolTip("Save/Control Regions");
    if ( shpMgrAct_ ) shpMgrAct_ ->setToolTip("Load/Control Region Shapes");
    printAct_  ->setToolTip("Print...");
    unzoomAct_ ->setToolTip("Zoom Out to Entire Image");
    zoomInAct_ ->setToolTip("Zoom In");
    zoomOutAct_->setToolTip("Zoom Out");
    dpQuitAct_ ->setToolTip("Close All Windows and Exit");


    ddRegBtn_  ->setPopupMode(QToolButton::InstantPopup);
    ddRegBtn_  ->setAutoRaise(True);
    ddRegBtn_  ->setIconSize(QSize(22,22));

    ddCloseBtn_->setPopupMode(QToolButton::InstantPopup);
    ddCloseBtn_->setAutoRaise(True);
    ddCloseBtn_->setIconSize(QSize(22,22));
  
//  bottomToolBar_->setMovable(False);
//  bottomToolBar_->hide();
//  (disabled unless/until something for it to contain).


    //######################################
    //## CONNECTIONS
    //######################################


    //## Direct reactions to user interface.

    connect(ddOpenAct_,  SIGNAL(triggered()),  SLOT(showDataManager()));
    connect(ddSaveAct_,  SIGNAL(triggered()),  SLOT(showExportManager()));
    connect(ddPreferencesAct_, SIGNAL(triggered()), SLOT(showPreferences()));
    // connect(dpNewAct_,   SIGNAL(triggered()),  v_, SLOT(createDPG()));
    connect(dpNewAct_,   SIGNAL(triggered()),  SLOT(createNewPanel()));
    connect(dpOptsAct_,  SIGNAL(triggered()),  SLOT(showCanvasManager()));
    connect(dpSaveAct_,  SIGNAL(triggered()),  SLOT(savePanelState_()));
    connect(dpRstrAct_,  SIGNAL(triggered()),  SLOT(restorePanelState_()));
    connect(dpCloseAct_, SIGNAL(triggered()),  SLOT(close()));
    connect(dpQuitAct_,  SIGNAL(triggered()),  SLOT(quit()));
    connect(profileAct_, SIGNAL(triggered()),  SLOT(showImageProfile()));
    // connect(rgnMgrAct_,  SIGNAL(triggered()),  SLOT(showRegionManager()));
    connect(ddAdjAct_,   SIGNAL(triggered()),  SLOT(showDataOptionsPanel()));
    connect(printAct_,   SIGNAL(triggered()),  SLOT(showPrintManager()));
    connect(unzoomAct_,  SIGNAL(triggered()),  qdp_, SLOT(unzoom()));
    connect(zoomInAct_,  SIGNAL(triggered()),  qdp_, SLOT(zoomIn()));
    connect(zoomOutAct_, SIGNAL(triggered()),  qdp_, SLOT(zoomOut()));
    // connect(animAuxButton_, SIGNAL(clicked()),    SLOT(toggleAnimExtras_()));

    //## docking changes
    connect( trkgDockWidget_, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(trackingMoved(Qt::DockWidgetArea)) );
    connect( animDockWidget_, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(animatorMoved(Qt::DockWidgetArea)) );
    if ( regionDock_ ) connect( regionDock_, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(regionMoved(Qt::DockWidgetArea)) );
#if QT_VERSION >= 0x040600
    connect( mouseToolBar_, SIGNAL(topLevelChanged(bool)), SLOT(mousetoolbarMoved(bool)) );
#endif

    //## user interface to animator
  
    connect(frameSlider_, SIGNAL(valueChanged(int)), qdp_, SLOT(goTo(int)));
    connect(frameEdit_, SIGNAL(editingFinished()),  SLOT(frameNumberEdited_()));
    // connect(rateSlider_,  SIGNAL(valueChanged(int)), qdp_, SLOT(setRate(int)));
    connect(rateEdit_,  SIGNAL(valueChanged(int)), qdp_, SLOT(setRate(int)));
    connect(normalRB_,    SIGNAL(toggled(bool)),     qdp_, SLOT(setMode(bool)));
  
    connect(toStartTB_, SIGNAL(clicked()),  qdp_, SLOT(toStart()));
    connect(revStepTB_, SIGNAL(clicked()),  qdp_, SLOT(revStep()));
    connect(revTB_, SIGNAL(clicked()),            SLOT(revPlay_()));
    connect(stopTB_, SIGNAL(clicked()),     qdp_, SLOT(stop()));
    connect(playTB_, SIGNAL(clicked()),           SLOT(fwdPlay_()));
    connect(fwdStep_, SIGNAL(clicked()),    qdp_, SLOT(fwdStep()));
    connect(toEndTB_, SIGNAL(clicked()),    qdp_, SLOT(toEnd()));
    
  
    // Reaction to signals from the basic graphics panel, qdp_. 
    // (qdp_ doesn't know about, and needn't necessarily use, this gui).
  
    // For tracking
  
    connect( qdp_, SIGNAL(trackingInfo(Record)),
		   SLOT(displayTrackingData_(Record)) );
  
    connect( this, SIGNAL(ddRemoved(QtDisplayData*)),
		   SLOT(deleteTrackBox_(QtDisplayData*)) );
    
    
    // From animator
  
    connect( qdp_, SIGNAL(animatorChange()),  SLOT(updateAnimUi_()) );
  
    
    // From registration

    connect( qdp_, SIGNAL(registrationChange()),  SLOT(ddRegChange_()), Qt::QueuedConnection );
		// (Important to queue this one, since the slot alters
		//  menus that may have triggered the signal).


    // From save-restore
  
    connect( qdp_, SIGNAL(creatingRstrDoc(QDomDocument*)),
                   SLOT(addGuiState_(QDomDocument*)) );
		// Adds gui state to QDomDocument qdp has created.
		// (Recall that qdp_ is unaware of this gui).
  
    connect( qdp_, SIGNAL(restoring(QDomDocument*)),
                   SLOT(restoreGuiState_(QDomDocument*)) );
		// Sets gui state from QDomDocument (window size, esp.)

    // FINAL INITIALIZATIONS

    connect( qdp_, SIGNAL(newRegisteredDD(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );
    connect( qdp_, SIGNAL(oldDDUnregistered(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );
    connect( qdp_, SIGNAL(oldDDRegistered(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );
    connect( qdp_, SIGNAL(RegisteredDDRemoved(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );
  
    connect(qdp_,  SIGNAL(registrationChange()),
		   SLOT(hideImageMenus()));
    updateDDMenus_();


    bool dimension_set = false;
    std::string dim = rc.get("viewer." + rcid() + ".dimensions");
    if ( dim != "" ) {
	std::stringstream ins(dim);
	int width, height;
	ins >> width >> height;
	if ( ins.fail() ) {
	    fprintf( stderr, "setting dimension failed...\n" );
	} else if ( width > 2400 || height > 2400 ) {
	    fprintf( stderr, "unreasonably large dimensions...\n" );
	} else {
	    dimension_set = true;
	    resize( QSize( width, height ).expandedTo(minimumSizeHint()) );
	}
    }

    if ( dimension_set == false ) {
	if ( animloc == "right" || trackloc == "right" ||
	     animloc == "left" || animloc == "left" ) {
	    if ( trackloc != "right" && trackloc != "left" ) {
		resize( QSize(1000, 700).expandedTo(minimumSizeHint()) );
	    } else if ( animloc != "right" && animloc != "left" ) {
		resize( QSize(1000, 760).expandedTo(minimumSizeHint()) );
	    } else {
		resize( QSize(1000, 640).expandedTo(minimumSizeHint()) );
	    }
	} else {
	    resize( QSize(600, 770).expandedTo(minimumSizeHint()) );
	}
		// To do: use a QSetting, to preserve size.
    }
}


QtDisplayPanelGui::~QtDisplayPanelGui() {

    removeAllDDs();  
    delete qdp_;	// (probably unnecessary because of Qt parenting...)
			// (possibly wrong, for same reason?...).
			// (indeed was wrong as the last deletion [at least] because the display panel also reference the qsm_)

    if(qpm_!=0) delete qpm_;
    if(qrm_!=0) delete qrm_;
    if(qsm_!=0) delete qsm_;
    if(qdm_!=0) delete qdm_;
    if(qdo_!=0) delete qdo_;
}

bool QtDisplayPanelGui::supports( SCRIPTING_OPTION ) const {
    return false;
}

QVariant QtDisplayPanelGui::start_interact( const QVariant &, int ) {
    return QVariant(QString("*error* unimplemented (by design)"));
}
QVariant QtDisplayPanelGui::setoptions( const QMap<QString,QVariant> &, int ) {
    return QVariant(QString("*error* nothing implemented yet"));
}

//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//# DisplayData functionality brought down from QtViewerBase
//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

QtDisplayData* QtDisplayPanelGui::createDD( String path, String dataType, String displayType,
					    Bool autoRegister, const viewer::DisplayDataOptions &ddo ) {

  QtDisplayData* qdd = new QtDisplayData( this, path, dataType, displayType, ddo );
  
  return processDD( path, dataType, displayType, autoRegister, qdd );
}
QtDisplayData* QtDisplayPanelGui::processDD( String path, String dataType, String displayType,
		Bool autoRegister, QtDisplayData* qdd){
  if(qdd->isEmpty()) {
    errMsg_ = qdd->errMsg();
    emit createDDFailed(errMsg_, path, dataType, displayType);
    return 0;  }
    
  // Be sure name is unique by adding numerical suffix if necessary.
  
  String name=qdd->name();
  for(Int i=2; dd(name)!=0; i++) {
    name=qdd->name() + " <" + viewer::to_string( i ) + ">";  }
  qdd->setName(name);
  
  ListIter<QtDisplayData* > qdds(qdds_);
  qdds.toEnd();
  qdds.addRight(qdd);
  
  emit ddCreated(qdd, autoRegister);

  return qdd;  }

void QtDisplayPanelGui::addDD(String path, String dataType, String displayType, Bool autoRegister, Bool tmpData, ImageInterface<Float>* img) {
	// create a new DD
	QtDisplayData* dd = NULL;
	if ( img == NULL ){
		createDD(path, dataType, displayType, autoRegister);
	}
	else {
		dd =new QtDisplayData( this, path, dataType, displayType);
		dd->setImage( img );
		dd->initImage();
		dd->init();
		dd = processDD( path, dataType, displayType, autoRegister, dd );
	}

	// set flagg if requested
	if (tmpData && dd != NULL ){
		dd->setDelTmpData(True);
	}
}

void QtDisplayPanelGui::doSelectChannel( const Vector<float> &zvec, float zval ) {
    unsigned int size = zvec.size( );
    if ( size == 0 ) return;
    unsigned int channel_num = static_cast<unsigned int>(zvec[0]);
    bool forward = zvec[0] < zvec[size-1];
    if ( forward ) {
	for ( unsigned int i=0; i < size; ++i ) {
	    if ( zval > zvec[i] ) channel_num = i;
	}
	if ( channel_num < (size-1) ) {
	    if ( (zval-zvec[channel_num]) > (zvec[channel_num+1]-zval) ) {
		channel_num += 1;
	    }
	}
    } else {
	for ( unsigned int i=0; i < size; ++i ) {
	    if ( zval < zvec[i] ) channel_num = i;
	}
	if ( channel_num > 0 ) {
	    if ( (zval-zvec[channel_num]) > (zvec[channel_num-1]-zval) ) {
		channel_num -= 1;
	    }
	}
    }

    qdp_->goTo((int)channel_num);
}

void QtDisplayPanelGui::removeAllDDs() {
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); ) {
    QtDisplayData* qdd = qdds.getRight();
    
    qdds.removeRight();
    emit ddRemoved(qdd);
    qdd->done();
    delete qdd;  }  }
  
    

Bool QtDisplayPanelGui::removeDD(QtDisplayData* qdd) {
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
    if(qdd == qdds.getRight()) {
    
      qdds.removeRight();
      emit ddRemoved(qdd);
      qdd->done();
      delete qdd;
      return True;  }  }
  
  return False;  }
      

  
Bool QtDisplayPanelGui::ddExists(QtDisplayData* qdd) {
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
    if(qdd == qdds.getRight()) return True;  }
  return False;  }

void QtDisplayPanelGui::loadRegions( const std::string &path, const std::string &datatype, const std::string &displaytype ) {
    if ( logger_did_region_warning == false ) {
	logger << LogIO::WARN
	       << "currently only supports rectangle, ellipse, symbol (somewhat), and polygon region shapes"
	       << LogIO::POST;
	logger_did_region_warning = true;
    }

    qdp_->loadRegions( path, datatype, displaytype );
}

QtDisplayData* QtDisplayPanelGui::dd(const std::string& name) {
  // retrieve DD with given name (0 if none).
  QtDisplayData* qdd;
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
    if( (qdd=qdds.getRight())->name() == name ) return qdd;  }
  return 0;  }

  
  
List<QtDisplayData*> QtDisplayPanelGui::registeredDDs() {
  // return a list of DDs that are registered on some panel.
  List<QtDisplayData*> rDDs(qdds_);
#if 0
  List<QtDisplayPanelGui*> DPs(viewer()->openDPs());
  
  for(ListIter<QtDisplayData*> rdds(rDDs); !rdds.atEnd();) {
    QtDisplayData* dd = rdds.getRight();
    Bool regd = False;
    
    for(ListIter<QtDisplayPanel*> dps(DPs); !dps.atEnd(); dps++) {
      QtDisplayPanel* dp = dps.getRight();
      if(dp->isRegistered(dd)) { regd = True; break;  }  }
    
    if(regd) rdds++;
    else rdds.removeRight();  }
#endif
  return rDDs;  }

  
List<QtDisplayData*> QtDisplayPanelGui::unregisteredDDs(){
  // return a list of DDs that exist but are not registered on any panel.
  List<QtDisplayData*> uDDs(qdds_);
  List<QtDisplayPanelGui*> DPs(viewer()->openDPs());
  
  for(ListIter<QtDisplayData*> udds(uDDs); !udds.atEnd(); ) {
    QtDisplayData* dd = udds.getRight();
    Bool regd = False;
    
    for(ListIter<QtDisplayPanelGui*> dps(DPs); !dps.atEnd(); dps++) {
      QtDisplayPanelGui* dp = dps.getRight();
      if(dp->displayPanel()->isRegistered(dd)) { regd = True; break;  }  }
    
    if(regd) udds.removeRight();
    else udds++;  }
  
  return uDDs;  }
  
//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//# DisplayData functionality brought down from QtViewerBase
//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


void QtDisplayPanelGui:: addedData( QString /*type*/, QtDisplayData * ) { }

// Animation slots.

void QtDisplayPanelGui::updateAnimUi_() {
  // This slot monitors the QtDisplayPanel::animatorChange() signal, to
  // keep the animator user interface ('view') in sync with that 'model'
  // (QtDisplayPanel's animator state).  It assumes that the animator
  // model is in a valid state (and this routine should not emit signals
  // that would cause state-setting commands to be fed back to that model).
  // Prevent the signal-feedback recursion mentioned above.
  // (The signal used from text boxes is only emitted on user edits).
  Bool nrbSav = normalRB_->blockSignals(True),
       brbSav = blinkRB_->blockSignals(True),
       // rslSav = rateSlider_->blockSignals(True),
       fslSav = frameSlider_->blockSignals(True);
  
  // Current animator state.
  Int  frm   = qdp_->frame();
  Int len  = qdp_->nFrames();
  qdp_->startFrame();
  qdp_->lastFrame();
  qdp_->step();
  Int     rate = qdp_->animRate();
  Int     minr  = qdp_->minRate();
  Int maxr = qdp_->maxRate();
  Int play  = qdp_->animating();
  Bool modez = qdp_->modeZ();
  

  frameEdit_->setText(QString::number(frm));
  nFrmsLbl_ ->setText(QString::number(len));
  
  if(modez) normalRB_->setChecked(True);
  else blinkRB_->setChecked(True);
	// NB: QRadioButton::setChecked(false)  doesn't work
	// (not what we want here anyway).
  
  // rateSlider_->setMinimum(minr);
  // rateSlider_->setMaximum(maxr);
  // rateSlider_->setValue(rate);
  rateEdit_->setMinimum(minr);
  rateEdit_->setMaximum(maxr);
  rateEdit_->setValue(rate);
  
  frameSlider_->setMinimum(0);
  frameSlider_->setMaximum(len-1);
  //frameSlider_->setMinimum(strt);
  //frameSlider_->setMaximum(lst);
  frameSlider_->setValue(frm);
  
  // stFrmEdit_ ->setText(QString::number(strt));
  // lstFrmEdit_->setText(QString::number(lst));
  // stepEdit_  ->setText(QString::number(stp));
  
  
  
  // Enable interface according to number of frames.
  
  // enabled in any case:
  modeGB_->setEnabled(True);		// Blink mode
  // animAuxButton_->setEnabled(True);	// 'Compact/Full' button.
  
  // Enabled only if there is more than 1 frame to animate:
  Bool multiframe = (len > 1);
  
  rateLbl_->setEnabled(multiframe);	// 
  // rateSlider_->setEnabled(multiframe);	// Rate controls.
  rateEdit_->setEnabled(multiframe);	//
  // perSecLbl_->setEnabled(multiframe);	//
  
  toStartTB_->setEnabled(multiframe);	//
  revStepTB_->setEnabled(multiframe);	//
  revTB_->setEnabled(multiframe);	//
  stopTB_->setEnabled(multiframe);	// Tape deck controls.
  playTB_->setEnabled(multiframe);	//
  fwdStep_->setEnabled(multiframe);	//
  toEndTB_->setEnabled(multiframe);	//
  frameEdit_->setEnabled(multiframe);	// Frame number entry.
  nFrmsLbl_->setEnabled(multiframe);	// Total frames label.
  // curFrmLbl_->setEnabled(multiframe);	//
  frameSlider_->setEnabled(multiframe);	// Frame number slider.
  // stFrmLbl_->setEnabled(multiframe);	//
  // stFrmEdit_->setEnabled(multiframe);	// first and last frames
  // lstFrmLbl_->setEnabled(multiframe);	// to include in animation
  // lstFrmEdit_->setEnabled(multiframe);	// and animation step.
  // stepLbl_->setEnabled(multiframe);	//
  // stepEdit_->setEnabled(multiframe);	//
  
  
  revTB_ ->setChecked(play<0);
  stopTB_->setChecked(play==0);
  playTB_->setChecked(play>0);
  
  
  //#dk  (For now, always disable the following animator
  //      interface, because it is not yet fully supported).
 
  // stFrmLbl_->setEnabled(False);		// 
  // stFrmEdit_->setEnabled(False);	// 
  // lstFrmLbl_->setEnabled(False);	// first and last frames
  // lstFrmEdit_->setEnabled(False);	// to include in animation,
  // stepLbl_->setEnabled(False);		// and animation step.
  // stepEdit_->setEnabled(False);		// 
  
  
  // restore signal-blocking state (to 'unblocked', in all likelihood).
  
  normalRB_->blockSignals(nrbSav),
  blinkRB_->blockSignals(brbSav),
  // rateSlider_->blockSignals(rslSav),
  frameSlider_->blockSignals(fslSav);  

//cout << "updataAni============" << endl;
}


void QtDisplayPanelGui::frameNumberEdited_() {
  // User has entered a frame number (text already Int-validated).
  qdp_->goTo(frameEdit_->text().toInt());  }


// Public slots: may be safely operated programmatically (i.e.,
// scripted, when available), or via gui actions.

void QtDisplayPanelGui::hideImageMenus() {
   List<QtDisplayData*> rdds = qdp_->registeredDDs();
   for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
      QtDisplayData* pdd = qdds.getRight();
      if(pdd != 0){
         ImageInterface<float>* img = pdd->imageInterface();
         //cout << "img=" << img << endl;
         //cout << "dataType=" << pdd->dataType() << endl;
         PanelDisplay* ppd = qdp_->panelDisplay();
         //cout << "ppd->isCSmaster=" 
         //      << ppd->isCSmaster(pdd->dd()) << endl;
         if (ppd != 0 && ppd->isCSmaster(pdd->dd())) { 
            if (pdd->dataType() == "image" && img !=0) {
	       if ( fboxAct_ ) fboxAct_->setEnabled(True);
               if ( mkRgnAct_ ) mkRgnAct_->setEnabled(True);
               if ( annotAct_ ) annotAct_->setEnabled(True);
               profileAct_->setEnabled(True);
               if ( shpMgrAct_ ) shpMgrAct_->setEnabled(True);
               setUseRegion(False);
               break;
            }
            if (pdd->dataType() == "ms" || img ==0) {
               
               hideRegionManager();
               hideAnnotatorPanel();
               hideFileBoxPanel();
               hideMakeRegionPanel();
               hideImageProfile();  
               hideShapeManager();
               hideStats();

               if ( fboxAct_ ) fboxAct_->setEnabled(False);
               if ( mkRgnAct_ ) mkRgnAct_->setEnabled(False);
               if ( annotAct_ ) annotAct_->setEnabled(False);
               profileAct_->setEnabled(False);
               if ( shpMgrAct_ ) shpMgrAct_->setEnabled(False);
               setUseRegion(False);
               //cout << "hide image menus" << endl;
               break;
            }
          }
       }
    }
}

void QtDisplayPanelGui::hideAllSubwindows() {
  hidePrintManager();
  hideCanvasManager();
  hideRegionManager();
  hideAnnotatorPanel();
  hideFileBoxPanel();
  hideMakeRegionPanel();
  hideImageProfile();  
  hideDataManager();
  hideExportManager();
  hideDataOptionsPanel();
  hideStats();
}

void QtDisplayPanelGui::createNewPanel( ) {
    v_->createDPG( )->show( );
}

void QtDisplayPanelGui::showDataManager() {
  if(qdm_==0) qdm_ = new QtDataManager(this);
  qdm_->showNormal();
  qdm_->raise();  }

void QtDisplayPanelGui::hideDataManager() {
  if(qdm_==0) return;
  qdm_->hide();  }

void QtDisplayPanelGui::showExportManager() {
  if(qem_==0) {
	  qem_ = new QtExportManager(this);
	  connect(this, SIGNAL(ddRemoved(QtDisplayData*)), qem_, SLOT(updateEM(QtDisplayData*)));
	  connect(this, SIGNAL(ddCreated(QtDisplayData*, Bool)), qem_, SLOT(updateEM(QtDisplayData*, Bool)));
  }
  qem_->updateEM();
  qem_->showNormal();
  qem_->raise();  }

void QtDisplayPanelGui::hideExportManager() {
  if(qem_==0) return;
  qem_->hide();  }


void QtDisplayPanelGui::showPreferences( ) {
    if ( preferences == 0 ) 
	preferences = new viewer::Preferences( );
    preferences->showNormal( );
    preferences->raise( );
}

    
void QtDisplayPanelGui::showDataOptionsPanel() {
    //  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---
    // prevent infinate recursion, due to loop:
    //		#1321 0x00e37264 in casa::QtDataOptionsPanel::createDDTab_ ()
    //		#1322 0x00e375b9 in casa::QtDataOptionsPanel::QtDataOptionsPanel ()
    //		#1323 0x00eda8bf in casa::QtDisplayPanelGui::showDataOptionsPanel ()
    //		#1324 0x00e37264 in casa::QtDataOptionsPanel::createDDTab_ ()
    //		#1325 0x00e375b9 in casa::QtDataOptionsPanel::QtDataOptionsPanel ()
    //		#1326 0x00eda8bf in casa::QtDisplayPanelGui::showDataOptionsPanel ()
    //  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---
    if ( showdataoptionspanel_enter_count == 0 ) {
	++showdataoptionspanel_enter_count;
	if(qdo_==0) qdo_ = new QtDataOptionsPanel(this);
	if(qdo_!=0) {  // (should be True, barring exceptions above).
	    qdo_->showNormal();
	    qdo_->raise();
	}
	--showdataoptionspanel_enter_count;
    }
}

void QtDisplayPanelGui::hideDataOptionsPanel() {
  if(qdo_==0) return;
  qdo_->hide();  }
  
  
void QtDisplayPanelGui::showPrintManager() {
  if(qpm_==0) qpm_ = new QtViewerPrintGui(qdp_);
  qpm_->showNormal();	// (Magic formula to bring a window up,
  qpm_->raise();  }	// normal size, whether 'closed' (hidden),
			// iconified, or merely obscured by other 
			// windows.  (Found through trial-and-error).

void QtDisplayPanelGui::hidePrintManager() {
  if(qpm_==0) return;
  qpm_->hide();  }

  
    
void QtDisplayPanelGui::showCanvasManager() {
  if(qcm_==0) qcm_ = new QtCanvasManager(qdp_);
  qcm_->showNormal();
  qcm_->raise();  }

void QtDisplayPanelGui::hideCanvasManager() {
  if(qcm_==0) return;
  qcm_->hide();  }

void QtDisplayPanelGui::showStats(const String& stats) {
  cout << stats << endl;
  
  /* this print stats on to a panel
  QFont font("Monospace");
  if(qst_==0) {
     qst_ = new QTextEdit;
     qst_->setWindowTitle(QObject::tr("Image Statistics"));
     //qst_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
     font.setStyleHint(QFont::TypeWriter);
     qst_->setCurrentFont(font);
     qst_->setReadOnly(1);
     qst_->setMinimumWidth(600);
  }
  QString s;
  s=stats.c_str();
  QFontMetrics fm(font);
  int len=s.length();
  int last = s.lastIndexOf('\n', len - 3);
  //cout << "length=" << len << " last=" << last << endl;
  QString lastLine=s.right(max(len - last,0));
  //cout << "lastLine=" << lastLine.toStdString() 
  //     << "<<<==========\n" << endl;
  int width=fm.width(lastLine);
  //cout << "width=" << width << endl;
  qst_->resize(width, qst_->size().height());
  qst_->append(s);
  qst_->showNormal();
  qst_->raise();  
  */
}

void QtDisplayPanelGui::trackingMoved(Qt::DockWidgetArea area) {
    if ( area == Qt::RightDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.cursor_tracking", "right" );
    } else if ( area == Qt::BottomDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.cursor_tracking", "bottom" );
    } else if ( area == Qt::LeftDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.cursor_tracking", "left" );
    } else if ( area == Qt::TopDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.cursor_tracking", "top" );
    }
}

void QtDisplayPanelGui::animatorMoved(Qt::DockWidgetArea area) {
    if ( area == Qt::RightDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.animator", "right" );
    } else if ( area == Qt::BottomDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.animator", "bottom" );
    } else if ( area == Qt::LeftDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.animator", "left" );
    } else if ( area == Qt::TopDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.animator", "top" );
    }
}

void QtDisplayPanelGui::regionMoved(Qt::DockWidgetArea area) {
    if ( area == Qt::RightDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.regions", "right" );
    } else if ( area == Qt::BottomDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.regions", "bottom" );
    } else if ( area == Qt::LeftDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.regions", "left" );
    } else if ( area == Qt::TopDockWidgetArea ) {
	rc.put( "viewer." + rcid() + ".position.regions", "top" );
    }
}

void QtDisplayPanelGui::mousetoolbarMoved(bool) {
    if ( toolBarArea(mouseToolBar_) == Qt::TopToolBarArea ) {
	rc.put( "viewer." + rcid() + ".position.mousetools", "top" );
    } else if ( toolBarArea(mouseToolBar_) == Qt::LeftToolBarArea ) {
	rc.put( "viewer." + rcid() + ".position.mousetools", "left" );
    } else if ( toolBarArea(mouseToolBar_) == Qt::RightToolBarArea ) {
	rc.put( "viewer." + rcid() + ".position.mousetools", "right" );
    } else if ( toolBarArea(mouseToolBar_) == Qt::BottomToolBarArea ) {
	rc.put( "viewer." + rcid() + ".position.mousetools", "bottom" );
    }
}

std::string QtDisplayPanelGui::getrc( const std::string &key ) {
    return rc.get( "viewer." + rcid() + "." + key );
}
void QtDisplayPanelGui::putrc( const std::string &key, const std::string &val ) {
    rc.put( "viewer." + rcid() + "." + key, val );
}


void QtDisplayPanelGui::hideStats() {
  if(qst_==0) return;
  qst_->hide();  }

void QtDisplayPanelGui::showRegionManager() {
  if(qrm_==0) return;
  List<QtDisplayData*> rdds = qdp_->registeredDDs();
  for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
     QtDisplayData* pdd = qdds.getRight();
     if(pdd != 0 && pdd->dataType() == "image") {
            
        ImageInterface<float>* img = pdd->imageInterface();
        PanelDisplay* ppd = qdp_->panelDisplay();
        if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {
           connect(pdd, 
                   SIGNAL(axisChanged(String, String, String, std::vector<int> )),
                   qrm_, 
                   SLOT(changeAxis(String, String, String, std::vector<int> )));
        }
      }
  }
  qrm_->showNormal();
  qrm_->raise();  }

void QtDisplayPanelGui::hideRegionManager() {
  if(qrm_==0) return;
  qrm_->hide();  }



void QtDisplayPanelGui::showShapeManager() {
  if(qsm_==0) qsm_ = new QtRegionShapeManager(qdp_);
  List<QtDisplayData*> rdds = qdp_->registeredDDs();
  for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
     QtDisplayData* pdd = qdds.getRight();
     if(pdd != 0 && pdd->dataType() == "image") {
            
        ImageInterface<float>* img = pdd->imageInterface();
        PanelDisplay* ppd = qdp_->panelDisplay();
        if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {
           qsm_->showNormal();
           qsm_->raise();  
        }
      }
  }
}

void QtDisplayPanelGui::hideShapeManager() {
  if(qsm_==0) return;
  qsm_->hide();  }

void QtDisplayPanelGui::showFileBoxPanel() {

  if (qfb_ == 0) 
     qfb_ = new FileBox(qdp_);

  List<QtDisplayData*> rdds = qdp_->registeredDDs();
  for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
     QtDisplayData* pdd = qdds.getRight();
     if(pdd != 0 && pdd->dataType() == "image") {
            
        ImageInterface<float>* img = pdd->imageInterface();
        PanelDisplay* ppd = qdp_->panelDisplay();
        if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {
           connect(qfb_,  SIGNAL(hideFileBox()),
                          SLOT(hideFileBoxPanel()));
           connect(pdd, 
                   SIGNAL(axisChanged(String, String, String, std::vector<int> )),
                   qfb_, 
                   SLOT(changeAxis(String, String, String, std::vector<int> )));
        }
      }
  }
  qfb_->showNormal();
  qfb_->raise();  
  if ( annotAct_ ) annotAct_->setEnabled(False);
  if ( mkRgnAct_ ) mkRgnAct_->setEnabled(False);
  setUseRegion(True);

}

void QtDisplayPanelGui::hideFileBoxPanel() {
  if (qfb_==0) 
     return;
  qfb_->hide();  
  if ( annotAct_ ) annotAct_->setEnabled(True);
  if ( mkRgnAct_ ) mkRgnAct_->setEnabled(True);
  setUseRegion(False);
}
    
void QtDisplayPanelGui::showAnnotatorPanel() {

  if (qap_ == 0) 
     qap_ = new MakeMask(qdp_);

  List<QtDisplayData*> rdds = qdp_->registeredDDs();
  for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
     QtDisplayData* pdd = qdds.getRight();
     if(pdd != 0 && pdd->dataType() == "image") {
            
        ImageInterface<float>* img = pdd->imageInterface();
        PanelDisplay* ppd = qdp_->panelDisplay();
        if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {
           connect(qap_,  SIGNAL(hideRegionInFile()),
                          SLOT(hideAnnotatorPanel()));
           connect(pdd, 
                   SIGNAL(axisChanged(String, String, String, std::vector<int> )),
                   qap_, 
                   SLOT(changeAxis(String, String, String, std::vector<int> )));
        }
      }
  }
  qap_->showNormal();
  qap_->raise();  
  if ( fboxAct_ ) fboxAct_->setEnabled(False);
  if ( mkRgnAct_ ) mkRgnAct_->setEnabled(False);
  setUseRegion(True);

}

void QtDisplayPanelGui::hideAnnotatorPanel() {
  //cout << "hide--------region in image" << endl;
  if (qap_==0) 
     return;
  qap_->hide();  
  if ( fboxAct_ ) fboxAct_->setEnabled(True);
  if ( mkRgnAct_ ) mkRgnAct_->setEnabled(True);
  setUseRegion(False);
}

void QtDisplayPanelGui::showMakeRegionPanel() {

  if (qmr_ == 0) 
     qmr_ = new MakeRegion(qdp_);

  List<QtDisplayData*> rdds = qdp_->registeredDDs();
  for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
     QtDisplayData* pdd = qdds.getRight();
     if(pdd != 0 && pdd->dataType() == "image") {
            
        ImageInterface<float>* img = pdd->imageInterface();
        PanelDisplay* ppd = qdp_->panelDisplay();
        if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {
           connect(qmr_,  SIGNAL(hideRegionInImage()),
                          SLOT(hideMakeRegionPanel()));
           qmr_->showNormal();
           qmr_->raise();  
           break;
        }
      }
  }
  if ( fboxAct_ ) fboxAct_->setEnabled(False);
  if ( annotAct_ ) annotAct_->setEnabled(False);
  setUseRegion(True);

}

void QtDisplayPanelGui::hideMakeRegionPanel() {
  //cout << "hide--------region in file" << endl;
  if (qmr_==0) 
     return;
  qmr_->hide();  
  if ( fboxAct_ ) fboxAct_->setEnabled(True);
  if ( annotAct_ ) annotAct_->setEnabled(True);
  setUseRegion(False);
}

void QtDisplayPanelGui::showImageProfile() {

    List<QtDisplayData*> rdds = qdp_->registeredDDs();
    QHash<QString, ImageInterface<float>*> overlap;
    for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {

	QtDisplayData* pdd = qdds.getRight();
	if(pdd != 0 && pdd->dataType() == "image") {
            
	    ImageInterface<float>* img = pdd->imageInterface();
	    PanelDisplay* ppd = qdp_->panelDisplay();
	    if (ppd != 0 && img != 0) {
 
		if (ppd->isCSmaster(pdd->dd())) { 

		    // pdd is a suitable QDD for profiling.
              
		    if (!profile_) {
			// Set up profiler for first time.
	        
		    	profile_ = new QtProfile(img, pdd->name().c_str());
		    	profile_->setPath(QString(pdd->path().c_str()) );
		    	connect( profile_, SIGNAL(hideProfile()), SLOT(hideImageProfile()));
		    	connect( qdp_, SIGNAL(registrationChange()), SLOT(refreshImageProfile()));
		    	connect( pdd, SIGNAL(axisChangedProfile(String, String, String, std::vector<int> )),
		    			profile_, SLOT(changeAxis(String, String, String, std::vector<int> )));
		    	connect( pdd, SIGNAL(spectrumChanged(String, String, String )),
		    			profile_, SLOT(changeSpectrum(String, String, String )));
		    	connect(profile_, SIGNAL(showCollapsedImg(String, String, String, Bool, Bool, ImageInterface<Float>*)),
		    			this, SLOT(addDD(String, String, String, Bool, Bool, ImageInterface<Float>*)));
		    	connect(profile_, SIGNAL(channelSelect(const Vector<float>&,float)),
		    			this, SLOT(doSelectChannel(const Vector<float>&,float)));
		    	 connect( pdd, SIGNAL(pixelsChanged(int,int)), profile_,
		    			   SLOT(pixelsChanged(int,int)) );
			{
			    QtCrossTool *pos = dynamic_cast<QtCrossTool*>(ppd->getTool(QtMouseToolNames::POSITION));
			    if (pos) {

				connect( pos, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
							       const Vector<Double>, const Vector<Double>, const ProfileType)),
					 profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								   const Vector<Double>, const Vector<Double>, const ProfileType)));
				connect( profile_, SIGNAL(coordinateChange(const String&)),
					 pos, SLOT(setCoordType(const String&)));

				// one region source is shared among all of the tools...
				// so there is no need to connect these signals for all of the tools...
				std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(pos->getRegionSource( )->kernel( ));

				if ( qrs ) {
				    connect( qrs.get( ), SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &,
										const QList<double> &, const QList<int> &, const QList<int> &,
										const QString &, const QString &, const QString &, int, int ) ),
					     profile_, SLOT( newRegion( int, const QString &, const QString &, const QList<double> &,
									const QList<double> &, const QList<int> &, const QList<int> &,
									const QString &, const QString &, const QString &, int, int ) ) );
				    connect( qrs.get( ), SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &,
									       const QList<int> &, const QList<int> & ) ),
					     profile_, SLOT( updateRegion( int, const QList<double> &, const QList<double> &,
									   const QList<int> &, const QList<int> & ) ) );
				    connect( qrs.get( ), SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &,
										       const QList<double> &, const QList<int> &, const QList<int> &,
										       const QString &, const QString &, const QString &, int, int ) ),
					     profile_, SLOT( newRegion( int, const QString &, const QString &, const QList<double> &,
									const QList<double> &, const QList<int> &, const QList<int> &,
									const QString &, const QString &, const QString &, int, int ) ) );

				}

			    } else {
				QtOldCrossTool *pos = dynamic_cast<QtOldCrossTool*>(ppd->getTool(QtMouseToolNames::POSITION));
				if (pos) {
				    connect( pos, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
								   const Vector<Double>, const Vector<Double>, const ProfileType)),
					     profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								       const Vector<Double>, const Vector<Double>, const ProfileType)));
				    connect( profile_, SIGNAL(coordinateChange(const String&)),
					     pos, SLOT(setCoordType(const String&)));

				}
			    }
			}
			
			{
			    QtRectTool *rect = dynamic_cast<QtRectTool*>(ppd->getTool(QtMouseToolNames::RECTANGLE));
			    if (rect) {
				connect( rect, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
								const Vector<Double>, const Vector<Double>, const ProfileType)),
					 profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								   const Vector<Double>, const Vector<Double>, const ProfileType )));
				connect( profile_, SIGNAL(coordinateChange(const String&)),
					 rect, SLOT(setCoordType(const String&)));

			    } else { 
				QtOldRectTool *rect = dynamic_cast<QtOldRectTool*>(ppd->getTool(QtMouseToolNames::RECTANGLE));
				if (rect) {
				    connect( rect, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
								    const Vector<Double>, const Vector<Double>, const ProfileType)),
					     profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								       const Vector<Double>, const Vector<Double>, const ProfileType )));
				    connect( profile_, SIGNAL(coordinateChange(const String&)),
					     rect, SLOT(setCoordType(const String&)));
				}
			    }
			}

			{
			    QtEllipseTool *ellipse = dynamic_cast<QtEllipseTool*>(ppd->getTool(QtMouseToolNames::ELLIPSE));
			    if (ellipse) {
				connect( ellipse, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
								   const Vector<Double>, const Vector<Double>, const ProfileType )),
					 profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								   const Vector<Double>, const Vector<Double>, const ProfileType )));
				connect( profile_, SIGNAL(coordinateChange(const String&)),
					 ellipse, SLOT(setCoordType(const String&)));

			    } else {
				QtOldEllipseTool *ellipse = dynamic_cast<QtOldEllipseTool*>(ppd->getTool(QtMouseToolNames::ELLIPSE));
				if (ellipse) {
				    connect( ellipse, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
								       const Vector<Double>, const Vector<Double>, const ProfileType )),
					     profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								       const Vector<Double>, const Vector<Double>, const ProfileType )));
				    connect( profile_, SIGNAL(coordinateChange(const String&)),
					     ellipse, SLOT(setCoordType(const String&)));
				}
			    }
			}

			{
			    QtPolyTool *poly = dynamic_cast<QtPolyTool*>(ppd->getTool(QtMouseToolNames::POLYGON));
			    if (poly) {
				connect( poly, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
								const Vector<Double>, const Vector<Double>, const ProfileType )),
					 profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								   const Vector<Double>, const Vector<Double>, const ProfileType )));
				connect( profile_, SIGNAL(coordinateChange(const String&)),
					 poly, SLOT(setCoordType(const String&)));

			    } else {
				QtOldPolyTool *poly = dynamic_cast<QtOldPolyTool*>(ppd->getTool(QtMouseToolNames::POLYGON));
				if (poly) {
				    connect( poly, SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
								    const Vector<Double>, const Vector<Double>, const ProfileType )),
					     profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
								       const Vector<Double>, const Vector<Double>, const ProfileType )));
				    connect( profile_, SIGNAL(coordinateChange(const String&)),
					     poly, SLOT(setCoordType(const String&)));
				}
			    }
			}
		    } else {
			if (profileDD_ != pdd) {

			    // [Re-]orient pre-existing profiler to pdd
			    profile_->resetProfile(img, pdd->name().c_str());
			    profileDD_ = pdd;
			} else {
			    pdd->checkAxis();
			}
		    }

		    if (pdd->getAxisIndex(String("Spectral")) == -1) {
			profileDD_ = 0;
			hideImageProfile();  
		    } else {
			profileDD_ = pdd;
			profile_->show();
			pdd->checkAxis();
		    }
	      
		    //break;
		} else {
		    if (pdd->getAxisIndex(String("Spectral")) != -1) 
		      overlap[pdd->name().c_str()] = img;
		}
	    }
	}
    }

    if (profile_) {
	connect( this, SIGNAL(overlay(QHash<QString, ImageInterface<float>*>)),
		 profile_, SLOT(overplot(QHash<QString, ImageInterface<float>*>)));
	emit overlay(overlap);
    }
    PanelDisplay* ppd = qdp_->panelDisplay();
    QtRectTool *rect = dynamic_cast<QtRectTool*>(ppd->getTool(QtMouseToolNames::RECTANGLE));
    if (rect) {
	// this is the *new* region implementation... all events come from region source...
	std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(rect->getRegionSource( )->kernel( ));
	qrs->generateExistingRegionUpdates( );
    }

}


void QtDisplayPanelGui::hideImageProfile() {
    
    if(profile_) {    
      profile_->hide();
      delete profile_;
      profile_ = 0;
    }
    profileDD_ = 0;
    
}


void QtDisplayPanelGui::refreshImageProfile() {
    if(profile_) {
	List<QtDisplayData*> rdds = qdp_->registeredDDs();
	if ( rdds.len() > 0 ) {
	    showImageProfile( );
	    profile_->redraw( );
	} else {
	    profile_->hide();
	    delete profile_;
	    profile_ = 0;
	    profileDD_ = 0;
	}
    }
}




// Internal slots and methods.
  

// Position Tracking

  
void QtDisplayPanelGui::arrangeTrackBoxes_() {
  // Reacts to QDP registration change signal.  If necessary, changes
  // the set of cursor position tracking boxes being displayed in
  // trkgWidget_ (creating new TrackBoxes as necessary).  A TrackBox
  // will be shown for each qdd in qdp_->registeredDDs() where
  // qdd->usesTracking() (in the same order).

  // Hide track boxes whose dd has been unregistered and remove them
  // from the trkgWidget_'s layout.  (They remain parented to trkgWidget_
  // until deleted, though).
  QList<TrackBox*> trkBoxes = trkgWidget_->findChildren<TrackBox*>();
  for(Int i=0; i<trkBoxes.size(); i++) {
    TrackBox* trkBox = trkBoxes[i];
    if( trkBox->isVisibleTo(trkgWidget_) && 
        !qdp_->isRegistered(trkBox->dd()) ) {
      trkgWidget_->layout()->removeWidget(trkBox);
      trkBox->hide();  }  }

  // Assure that all applicable registered QDDs are showing track boxes.
  List<QtDisplayData*> rDDs = qdp_->registeredDDs();
  for(ListIter<QtDisplayData*> rdds(&rDDs); !rdds.atEnd(); rdds++) {
    showTrackBox_(rdds.getRight());  }  }
    
    

TrackBox* QtDisplayPanelGui::showTrackBox_(QtDisplayData* qdd) {
  // If qdd->usesTracking(), this method assures that a TrackBox for qdd
  // is visible in the trkgWidget_'s layout (creating the TrackBox if it
  // didn't exist).  Used by arrangeTrackBoxes_() above.  Returns the
  // TrackBox (or 0 if none, i.e., if !qdd->usesTracking()).
  
  if(!qdd->usesTracking()) return 0;	// (track boxes N/A to qdd)
  
  TrackBox* trkBox = trkBox_(qdd);
  Bool notShown = trkBox==0 || trkBox->isHidden();
  
  if(trkBox==0) trkBox = new TrackBox(qdd);
  else if(notShown) trkBox->clear();	// (Clear old, hidden trackbox).

  if(notShown) {
    trkgWidget_->layout()->addWidget(trkBox);
    trkBox->show();  }
	// (trkBox will be added to the _bottom_ of trkgWidget_, assuring
	// that track boxes are displayed in registration order).
  
  return trkBox;  }
 
  
 
   
void QtDisplayPanelGui::deleteTrackBox_(QtDisplayData* qdd) {
  // Deletes the TrackBox for the given QDD if it exists.  Deletion
  // automatically removes it from the gui (trkgWidget_ and its layout). 
  // Connected to the ddRemoved() signal of QtViewerBase.
  if(hasTrackBox_(qdd)) delete trkBox_(qdd);  }




void QtDisplayPanelGui::displayTrackingData_(Record trackingRec) {
  // Display tracking data gathered by underlying panel.
  
  for(uInt i=0; i<trackingRec.nfields(); i++) {
    TrackBox* trkBox = trkBox_(trackingRec.name(i));
    if(trkBox!=0) trkBox->setText(trackingRec.asString(i));  }  }

  

TrackBox* QtDisplayPanelGui::trkBox_(QtDisplayData* qdd) {
  return trkBox_(qdd->name());  }
  
TrackBox* QtDisplayPanelGui::trkBox_(String ddname) {
  return trkgWidget_->findChild<TrackBox*>(ddname.chars());  }

    
    
TrackBox::TrackBox(QtDisplayData* qdd, QWidget* parent) :
                   QGroupBox(parent), qdd_(qdd) {

  trkgEdit_ = new QTextEdit;
  
  new QVBoxLayout(this);
  layout()->addWidget(trkgEdit_);
  layout()->setMargin(1);
  
  connect( this, SIGNAL(toggled(bool)),  trkgEdit_, SLOT(setVisible(bool)) );
	// (User can hide edit area with a checkbox by the track box title).
  
  
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); 
  
  // (TrackBox as QroupBox)
  setFlat(True);
  setObjectName(qdd_->name().c_str());
  setTitle(objectName());
  setCheckable(True);
  setChecked(True);
  // setAlignment(Qt::AlignHCenter);
  // setAlignment(Qt::AlignRight);
  String tltp="Uncheck if you do not need to see position tracking data for\n"
              + name() + "  (it will remain registered).";
  setToolTip(tltp.chars());
    
  
  trkgEdit_->setMinimumWidth(355);
  trkgEdit_->setFixedHeight( qdd->dataType() == "ms" ? 84 : 47 );
  // trkgEdit_->setFixedHeight(81);	// (obs.)
  //trkgEdit_->setPlainText("\n  ");	// (Doesn't work on init,
  //setTrackingHeight_();		// for some reason...).
  
  QFont trkgFont;
  trkgFont.setFamily(QString::fromUtf8("Courier"));
  trkgFont.setBold(True);
  trkgEdit_->setFont(trkgFont);
    
  trkgEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  trkgEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  trkgEdit_->setLineWrapMode(QTextEdit::NoWrap);
  trkgEdit_->setReadOnly(True);
  trkgEdit_->setAcceptRichText(True);  }


    
void TrackBox::setText(String trkgString) {
  trkgEdit_->setPlainText(trkgString.chars());
  setTrackingHeight_();  }

  
    
void TrackBox::setTrackingHeight_() {
  // Set tracking edit height according to contents.
  // Note: setting a 'fixed' height is necessary to cause Dock Area
  // to return unneeded space.  'Fixed' size policy alone is _not_
  // adequate, apparently because [min.] size hints cannot be relied
  // upon to be recalculated correctly (or at least in a way that
  // makes sense to me...).  Issues outstanding with Trolltech (Qt).
  //
  // (Even the working behavior was broken again in Qt-4.2.0;  awaiting
  // further fixes before upgrading beyond 4.1.3).
  //
  // (Broken even in 4.1.3 is ability to use this routine to set the
  // initial trackbox height, before the widget is ever shown...).
  //
  // I don't understand all these issues.  As usual with Qt sizing,
  // this kludgy routine was arrived at by laborious trial-and-error....
  // (dk  11/06)
   
  trkgEdit_->setUpdatesEnabled(False);
	// temporarily disables widget painting, avoiding flicker
	// from next statement.
  
  trkgEdit_->setFixedHeight(1000);	// (More than is ever needed.
	// Necessary so that the cursorRect() call below will return the
	// proper height needed for the text, without truncating to the
	// widget's current height...).
  
  QTextCursor c = trkgEdit_->textCursor();
  c.movePosition(QTextCursor::End);
  trkgEdit_->setTextCursor(c);
  
  trkgEdit_->setFixedHeight(trkgEdit_->cursorRect().bottom()+10);
  
  c.movePosition(QTextCursor::Start);
  trkgEdit_->setTextCursor(c);		// (assures left edge is visible
  trkgEdit_->ensureCursorVisible();	//  when tracking is updated).
  
  trkgEdit_->setUpdatesEnabled(True);  }






// Etc.

void QtDisplayPanelGui::close( ) {
	// shit down the DD's
	removeAllDDs();

	// shut down the window
	QtPanelBase::closeMainPanel();
}

void QtDisplayPanelGui::quit( ) {
	removeAllDDs();
    emit closed( this );

    if ( v_->server( ) ) {
	close( );
    } else {
	v_->quit( );
    }
}

// void QtDisplayPanelGui::toggleAnimExtras_() {

//   if(animAuxButton_->text()=="Full") animAuxButton_->setText("Compact");
//   else				     animAuxButton_->setText("Full");  
//   setAnimExtrasVisibility_();  }

   
       
// void QtDisplayPanelGui::setAnimExtrasVisibility_() {
    
// //#dg  ...of failure of dockWidgets/areas to downsize when needed.
// // (also, of improper dependencies of szHints on event processing).
// // (Leave these until promised Qt fixes arrive...).
    
// /*  //#dg
// cerr<<"anMSzHb:"<<animWidget_->minimumSizeHint().width()	    
//           <<","<<animWidget_->minimumSizeHint().height()
//     <<"   SzHb:"<<animWidget_->sizeHint().width()	    
//           <<","<<animWidget_->sizeHint().height()<<endl; 
// cerr<<"trMSzHb:"<<trkgWidget_->minimumSizeHint().width()	    
//           <<","<<trkgWidget_->minimumSizeHint().height()
//     <<"   SzHb:"<<trkgWidget_->sizeHint().width()	    
//           <<","<<trkgWidget_->sizeHint().height()<<endl<<endl; 
// //*/  //#dg  

  
  
//   if(animAuxButton_->text()=="Full") {
//     animAuxFrame_->hide(); modeGB_->hide();  }
//   else {
//     animAuxFrame_->show(); modeGB_->show();
//     animAuxButton_->setText("Compact");  }  
    
    
/*  //#dg
cerr<<"anMSzHa:"<<animWidget_->minimumSizeHint().width()	    
          <<","<<animWidget_->minimumSizeHint().height()
    <<"   SzHa:"<<animWidget_->sizeHint().width()	    
          <<","<<animWidget_->sizeHint().height()<<endl; 
cerr<<"trMSzHa:"<<trkgWidget_->minimumSizeHint().width()	    
          <<","<<trkgWidget_->minimumSizeHint().height()
    <<"   SzHa:"<<trkgWidget_->sizeHint().width()	    
          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl; 
//*/  //#dg  


/*  //#dg
  repaint();    //#dg 
cerr<<"anMSzHr:"<<animWidget_->minimumSizeHint().width()	    
          <<","<<animWidget_->minimumSizeHint().height()
    <<"   SzHr:"<<animWidget_->sizeHint().width()	    
          <<","<<animWidget_->sizeHint().height()<<endl; 
cerr<<"trMSzHr:"<<trkgWidget_->minimumSizeHint().width()	    
          <<","<<trkgWidget_->minimumSizeHint().height()
    <<"   SzHr:"<<trkgWidget_->sizeHint().width()	    
          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl; 
//*/  //#dg  


/*  //#dg
  update();    //#dg 
cerr<<"anMSzHu:"<<animWidget_->minimumSizeHint().width()	    
          <<","<<animWidget_->minimumSizeHint().height()
    <<"   SzHu:"<<animWidget_->sizeHint().width()	    
          <<","<<animWidget_->sizeHint().height()<<endl; 
cerr<<"trMSzHu:"<<trkgWidget_->minimumSizeHint().width()	    
          <<","<<trkgWidget_->minimumSizeHint().height()
    <<"   SzHu:"<<trkgWidget_->sizeHint().width()	    
          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl; 
//*/  //#dg  


/*  //#dg
  QtApp::app()->processEvents();  //#dg
	// (NB: anSzH's don't usually decrease until this point(?!)...)
cerr<<"anMSzHp:"<<animWidget_->minimumSizeHint().width()	    
          <<","<<animWidget_->minimumSizeHint().height()
    <<"   SzHp:"<<animWidget_->sizeHint().width()	    
          <<","<<animWidget_->sizeHint().height()<<endl; 
cerr<<"trMSzHp:"<<trkgWidget_->minimumSizeHint().width()	    
          <<","<<trkgWidget_->minimumSizeHint().height()
    <<"   SzHp:"<<trkgWidget_->sizeHint().width()	    
          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl; 
//*/  //#dg  


// }




// Save-Restore methods.



void QtDisplayPanelGui::savePanelState_() {
  // Brings up dialog for saving display panel state: reg'd DDs, their
  // options, etc.  Triggered by dpSaveAct_.
  
  QString         savedir = selectedDMDir.chars();
  if(savedir=="") savedir = QDir::currentPath();
  
  QString suggpath = savedir + "/" + qdp_->suggestedRestoreFilename().chars();
  
  String filename = QFileDialog::getSaveFileName(
                    this, "Save State As", suggpath,  
		    ( "viewer restore files (*."
                      + v_->cvRestoreFileExt
		      + ");;all files (*.*)" ).chars() ).toStdString();
  
  if(filename=="") return;
  
  if(!qdp_->savePanelState(filename)) {
    
    cerr<<endl<<"**Unable to save "<<filename<<endl
              <<"  (check permissions)**"<<endl<<endl;  }
  
  syncDataDir_(filename);  }
	// Keeps current data directory in sync between
	// DataManager window and save-restore dialogs.

   
  
  
void QtDisplayPanelGui::restorePanelState_() {
  // Brings up dialog for restore file, attempts restore.
  // Triggered by dpRstrAct_.

  QString            restoredir = selectedDMDir.chars();
  if(restoredir=="") restoredir = QDir::currentPath();
  
  String filename = QFileDialog::getOpenFileName(
                    this, "Select File for Viewer Restore", restoredir,
                    ( "viewer restore files (*."
		      +v_->cvRestoreFileExt
		      +");;all files (*.*)" ).chars() ).toStdString();
  
  if(filename=="") return;
    
  syncDataDir_(filename);  
	// Keeps current data directory in sync between
	// DataManager window and save-restore dialogs.
    
  restorePanelState(filename);  }

  


Bool QtDisplayPanelGui::restorePanelState(String filename) {
  // (Attempts to) restore panel state from named file (or from
  // 'clipboard' if filename=="").

  return qdp_->restorePanelState(filename);  }
 
 
  
        
Bool QtDisplayPanelGui::syncDataDir_(String filename) {
  // Keeps current data directory in sync between
  // DataManager window and save-restore dialogs.

  QDir datadir = QFileInfo(filename.chars()).dir();
  if(!datadir.exists()) return False;
  
  QString datadirname = datadir.path();
  
  if(dataMgr()!=0) dataMgr()->updateDirectory(datadirname);
  else selectedDMDir = datadirname.toStdString();
  return True;  }



void QtDisplayPanelGui::addGuiState_(QDomDocument* restoredoc) {
  // Responds to qdp_->creatingRstrDoc(QDomDocument*) signal.
  // (Recall that qdp_ is unaware of this gui).
  // Adds gui state to the QDomDocument qdp_ has created.
  
  QDomElement restoreElem = 
              restoredoc->firstChildElement(v_->cvRestoreID.chars());
  if(restoreElem.isNull()) return;  // invalid rstr doc (shouldn't happen).
  
  QDomElement dpgSettings = restoredoc->createElement("gui-settings");
  restoreElem.appendChild(dpgSettings);
  
  
  // QMainWindow settings (dock status of toolbars and dockwidgets, etc.)
  
  QDomElement dockSettings = restoredoc->createElement("dock-settings");
  dpgSettings.appendChild(dockSettings);
  QByteArray docksettings = saveState().toBase64();
  
  dockSettings.setAttribute("data", docksettings.constData());
  
  
  // Overall window size.
  
  QDomElement winsize = restoredoc->createElement("window-size");
  dpgSettings.appendChild(winsize);
  
  winsize.setAttribute("width",  size().width());
  winsize.setAttribute("height", size().height());
    
  // RegionShapes
  qsm_->saveState(*restoredoc);  }
  
  
  
  
void QtDisplayPanelGui::restoreGuiState_(QDomDocument* restoredoc) { 
  // Responds to qdp_->creatingRstrDoc(QDomDocument*) signal.
  // Sets gui-specific state (most notably, overall window size).
  
  QDomElement dpgSettings =
              restoredoc->firstChildElement(v_->cvRestoreID.chars())
			 .firstChildElement("gui-settings");
  
  if(dpgSettings.isNull()) return;

  
  // QMainWindow settings (dock status of toolbars and dockwidgets, etc.)
  
  QDomElement dockSettings = dpgSettings.firstChildElement("dock-settings");
  QString dockstring = dockSettings.attribute("data");
  if(dockstring!="") {
    QByteArray docksettings;		// (Why is it so damn hard to convert
    docksettings.append(dockstring);	// between ascii QByteArrays and
					// QStrings?...)
    restoreState(QByteArray::fromBase64(docksettings));  }
  
  
  // Overall window size
  
  QDomElement winsize = dpgSettings.firstChildElement("window-size");
  QString wd = winsize.attribute("width", "#"),
          ht = winsize.attribute("height", "#");
  Bool w_ok = False, h_ok = False;
  Int w = wd.toInt(&w_ok); 
  Int h = ht.toInt(&h_ok);
  
  if(h_ok && w_ok) resize(w, h);
  
  
  // Other items (anim full/compact, chkboxes, trkgfontsize, etc...).
    
    // (to implement).
  
  
  // RegionShapes state
    
  qsm_->restoreState(*restoredoc);  }  



  
// Reactors to QDP registration status changes.



void QtDisplayPanelGui::updateDDMenus_(Bool /*doCloseMenu*/) {
  // Re-populates ddRegMenu_ with actions.  If doCloseMenu is
  // True (on DD create/close), also recreates ddCloseMenu_.
  // (For now, both menus are always recreated).

  ddRegMenu_->clear();  ddCloseMenu_->clear();
  
  List<QtDisplayData*> regdDDs   = qdp_->registeredDDs();
  List<QtDisplayData*> unregdDDs = qdp_->unregisteredDDs();
  
  Bool anyRdds = regdDDs.len()>0u,   anyUdds = unregdDDs.len()>0u,
       manydds = regdDDs.len() + unregdDDs.len() > 1u; 

  QAction* action = 0;

  // The following allows slots to distinguish the dd associated with
  // triggered actions (Qt actions and signals are somewhat deficient in
  // their ability to make distinctions of this sort, imo).
  // Also note the macro at the end of QtDisplayData.qo.h, which enables
  // QtDisplayData* to be a QVariant's value.
  QVariant ddv;		// QVariant wrapper for a QtDisplayData pointer.
    
  
  // For registered DDs:...
  
  for(ListIter<QtDisplayData*> rdds(regdDDs); !rdds.atEnd(); rdds++) {
    QtDisplayData* rdd = rdds.getRight();
    
    ddv.setValue(rdd);
    
    
    // 'Unregister' menu item for dd.
    
    // Note: the explicit parenting means that the Action will
    // be deleted on the next ddRegMenu_->clear().
    
    action = new QAction(rdd->name().c_str(), ddRegMenu_);
    
    action->setCheckable(True);
    action->setChecked(True);
    action->setData(ddv);	// Associate the dd with the action.
    ddRegMenu_->addAction(action);
    connect(action, SIGNAL(triggered()), SLOT(ddUnregClicked_()));

    
    // 'Close' menu item.
    
    action = new QAction( ("Close "+rdd->name()).c_str(), ddCloseMenu_ );
    action->setData(ddv);
    ddCloseMenu_->addAction(action);
    connect(action, SIGNAL(triggered()), SLOT(ddCloseClicked_()));  }

  
  if(anyRdds && anyUdds) {
    ddRegMenu_->addSeparator();
    ddCloseMenu_->addSeparator();  }  

  // Enable/disable shape manager
  // That is not right, if dd is ms, crashes!
  //shpMgrAct_->setEnabled(anyRdds);
  //if(qsm_->isVisible() && !anyRdds) qsm_->close();  

  
  // For unregistered DDs:...
  
  for(ListIter<QtDisplayData*> udds(unregdDDs); !udds.atEnd(); udds++) {
    QtDisplayData* udd = udds.getRight();
    
    ddv.setValue(udd);
    
    
    // 'Unregister' menu item.
    
    action = new QAction(udd->name().c_str(), ddRegMenu_);
    action->setCheckable(True);
    action->setChecked(False);
    action->setData(ddv);
    ddRegMenu_->addAction(action);
    connect(action, SIGNAL(triggered()), SLOT(ddRegClicked_()));
    
    
    // 'Close' menu item.
    
    action = new QAction(("Close "+udd->name()).c_str(), ddCloseMenu_);
    action->setData(ddv);
    ddCloseMenu_->addAction(action);
    connect(action, SIGNAL(triggered()), SLOT(ddCloseClicked_()));  }
  
  
  // '[Un]Register All' / 'Close All'  menu items.
  
  if(manydds) {
    
    ddRegMenu_->addSeparator();

    if(anyUdds) {
      action = new QAction("Register All", ddRegMenu_);
      ddRegMenu_->addAction(action);
      connect(action, SIGNAL(triggered()),  qdp_, SLOT(registerAll()));  }

    if(anyRdds) {
      action = new QAction("Unregister All", ddRegMenu_);
      ddRegMenu_->addAction(action);
      connect(action, SIGNAL(triggered()),  qdp_, SLOT(unregisterAll()));  }

    
    ddCloseMenu_->addSeparator();
    
    action = new QAction("Close All", ddCloseMenu_);
    ddCloseMenu_->addAction(action);
    connect(action, SIGNAL(triggered()), SLOT(removeAllDDs()));  }  }
    




void QtDisplayPanelGui::closeEvent(QCloseEvent *event) {

    // save viewer dimensions
    QSize ending_size(size());
    char buf[256];
    sprintf( buf, "%d %d", ending_size.width(), ending_size.height() );
    rc.put( "viewer." + rcid() + ".dimensions", buf );

#if QT_VERSION >= 0x040500
    QList<QDockWidget*> tabbeddocks = tabifiedDockWidgets( trkgDockWidget_ );

    int rcnt = 0, lcnt = 0, tcnt = 0, bcnt = 0;
    for ( QList<QDockWidget*>::const_iterator i = tabbeddocks.begin(); i !=  tabbeddocks.end(); ++i ) {
	if ( dockWidgetArea(*i) == Qt::RightDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".rightdock", "tabbed" );
	    ++rcnt;
	}
	if ( dockWidgetArea(*i) == Qt::BottomDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".bottomdock", "tabbed" );
	    ++bcnt;
	}
	if ( dockWidgetArea(*i) == Qt::LeftDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".leftdock", "tabbed" );
	    ++lcnt;
	}
	if ( dockWidgetArea(*i) == Qt::TopDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".topdock", "tabbed" );
	    ++tcnt;
	}
    }

    if ( trkgDockWidget_->isVisible( ) && animDockWidget_->isVisible( ) &&
	 tabbeddocks.indexOf(trkgDockWidget_) < 0 && tabbeddocks.indexOf(animDockWidget_) < 0 ) {
	// ... both right/bottom dock widgets are available & untabbed
	// ... if they are both on the same dock location it means that location is untabbed
	if ( dockWidgetArea(trkgDockWidget_) == Qt::RightDockWidgetArea &&
	     dockWidgetArea(animDockWidget_) == Qt::RightDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".rightdock", "untabbed" );
	}
	if ( dockWidgetArea(trkgDockWidget_) == Qt::BottomDockWidgetArea &&
	     dockWidgetArea(animDockWidget_) == Qt::BottomDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".bottomdock", "untabbed" );
	}
	if ( dockWidgetArea(trkgDockWidget_) == Qt::LeftDockWidgetArea &&
	     dockWidgetArea(animDockWidget_) == Qt::LeftDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".leftdock", "untabbed" );
	}
	if ( dockWidgetArea(trkgDockWidget_) == Qt::TopDockWidgetArea &&
	     dockWidgetArea(animDockWidget_) == Qt::TopDockWidgetArea ) {
	    rc.put( "viewer." + rcid() + ".topdock", "untabbed" );
	}
    }
#endif

    QtPanelBase::closeEvent(event);
}

// Slots to respond to registration/close menu clicks.


void QtDisplayPanelGui::ddRegClicked_() {

  // Retrieve the dd associated with the signal.
  
  QAction* action = dynamic_cast<QAction*>(sender());
  if(action==0) return;		// (shouldn't happen).
  QtDisplayData* dd = action->data().value<QtDisplayData*>();
  
  qdp_->registerDD(dd);  }  


void QtDisplayPanelGui::ddUnregClicked_() {
  QAction* action = dynamic_cast<QAction*>(sender());
  if(action==0) return;		// (shouldn't happen).
  QtDisplayData* dd = action->data().value<QtDisplayData*>();
  
  qdp_->unregisterDD(dd);  }  


void QtDisplayPanelGui::ddCloseClicked_() {
  QAction* action = dynamic_cast<QAction*>(sender());
  if(action==0) return;		// (shouldn't happen).
  QtDisplayData* dd = action->data().value<QtDisplayData*>();

  removeDD(dd);  }  

 
void QtDisplayPanelGui::setColorBarOrientation(Bool vertical) {    
  // At least for now, colorbars can only be placed horizontally or vertically,
  // identically for all display panels.  This sets that state for everyone.
  // Sends out colorBarOrientationChange signal when the state changes.
  
  if(colorBarsVertical_ == vertical) return;	// (already there).
  
  colorBarsVertical_ = vertical;
  
  // Tell QDPs and QDDs to rearrange color bars as necessary.
  
  v_->hold();	// (avoid unnecessary extra refreshes).
  emit colorBarOrientationChange();
  v_->release();  }

  void QtDisplayPanelGui::controlling_dd_update(QtDisplayData*) {
	// manage controlling_dd so that we can generate updateAxis( ) events
	// in response to user visible axis changes...

	QtDisplayData *ctrld = 0;
	List<QtDisplayData*> rdds = qdp_->registeredDDs();
	for ( ListIter<QtDisplayData*> iter(&rdds); ! iter.atEnd(); ++iter ) {
	    QtDisplayData* pdd = iter.getRight();
	    if ( pdd != 0 && pdd->dataType() == "image" ) {
		ImageInterface<float>* img = pdd->imageInterface( );
		PanelDisplay* ppd = qdp_->panelDisplay( );
		if ( ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0 ) { ctrld = pdd; }
	    }
	}

	if ( ctrld != controlling_dd ) {
	    if ( controlling_dd != 0 )
		disconnect( controlling_dd, SIGNAL(axisChanged(String, String, String, std::vector<int>)),
			    this, SLOT(controlling_dd_axis_change(String, String, String, std::vector<int> )) );
	    controlling_dd = ctrld;
	    emit axisToolUpdate( controlling_dd );
	    if ( controlling_dd != 0 )
		connect( controlling_dd, SIGNAL(axisChanged(String, String, String, std::vector<int>)),
			 SLOT(controlling_dd_axis_change(String, String, String, std::vector<int> )) );
	}

  }

  void QtDisplayPanelGui::controlling_dd_axis_change(String, String, String, std::vector<int> ) {
	emit axisToolUpdate( controlling_dd );
  }

}