//# PlotMS.cc: Main controller for plotms.
//# Copyright (C) 2008
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
//#
//# $Id: $
#include <plotms/PlotMS/PlotMS.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMSDBusApp.h>
#include <plotms/Client/ClientFactory.h>
#include <plotms/Actions/ActionFactory.h>
#include <plotms/Actions/ActionCacheLoad.h>

#include <QDebug>

namespace casa {

// TODO PlotMSAction: iteration, release cache.  action for new plots.  update
//      hold/release action/button text.
// TODO PlotMSCache: multi-region locate
// TODO PlotMSLogger: log source (std out, text widget, casapy logger), better
//      locate message, log message for parameters updated and action
//      execution, log event flag for flag/unflag
// TODO PlotMSParameters: canvas background, fonts, grids, spacing, cartesian
//      axes, limit zoom/pan cache size
// TODO PlotMSPlot: different colors within one plot, different types, shared
//      caches
// TODO PlotMSPlotter: range padding, customize toolbars/tabs
// TODO PlotMSThread: background, pause/resume
// TODO PlotMSWidgets: label creator
// TODO PlotTool: set tracker font

////////////////////////
// PLOTMS DEFINITIONS //
////////////////////////

// Constructors/Destructors //

PlotMSApp::PlotMSApp(bool connectToDBus, bool userGui) : itsDBus_(NULL) {
    initialize(connectToDBus, userGui);
}

PlotMSApp::PlotMSApp(const PlotMSParameters& params, bool connectToDBus,
		bool userGui) :
        itsPlotter_(NULL), itsParameters_(params), itsDBus_(NULL) {
    initialize(connectToDBus, userGui);
}

PlotMSApp::~PlotMSApp() {
    if(itsDBus_ != NULL) delete itsDBus_;
}


// Public Methods //

//PlotMSPlotter* PlotMSApp::getPlotter() { return itsPlotter_; }
void PlotMSApp::showGUI(bool show) { itsPlotter_->showGUI(show); }
bool PlotMSApp::guiShown() const { return itsPlotter_->guiShown(); }
int PlotMSApp::execLoop() { return itsPlotter_->execLoop(); }
int PlotMSApp::showAndExec(bool show) { return itsPlotter_->showAndExec(show); }
void PlotMSApp::close() { itsPlotter_->close(); }

void PlotMSApp::showError(const String& message, const String& title,
        bool isWarning) { itsPlotter_->showError(message, title, isWarning); }
void PlotMSApp::showWarning(const String& message, const String& title) {
    itsPlotter_->showError(message, title, true); }
void PlotMSApp::showMessage(const String& message, const String& title) {
    itsPlotter_->showMessage(message, title); }

PlotMSParameters& PlotMSApp::getParameters() { return itsParameters_; }
void PlotMSApp::setParameters(const PlotMSParameters& params) {
    itsParameters_ = params; }

void PlotMSApp::parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag) {
	(void)updateFlag;
    // We only care about PlotMS's parameters.
    if(&params == &itsParameters_) {
        itsLogger_->setSinkLocation(itsParameters_.logFilename());
        itsLogger_->setFilterEventFlags(itsParameters_.logEvents());
        itsLogger_->setFilterMinPriority(itsParameters_.logPriority());
        
        pair<int, int> cis = itsParameters_.cachedImageSize();
        /*if(itsPlotter_ != NULL && !itsPlotter_->getPlotter().null() &&
           !itsPlotter_->getPlotter()->canvasLayout().null()) {
            vector<PlotCanvasPtr> canv = itsPlotter_->getPlotter()
                                     ->canvasLayout()->allCanvases();
            for(unsigned int i = 0; i < canv.size(); i++)
                if(!canv[i].null())
                    canv[i]->setCachedAxesStackImageSize(cis.first,cis.second);
        }*/
        if ( itsPlotter_ != NULL ){
        	itsPlotter_->setCanvasCachedAxesStackImageSize( cis.first, cis.second );
        }
    }
}

PlotSymbolPtr PlotMSApp::createSymbol( const PlotSymbolPtr& copy ){
	PlotSymbolPtr symbol;
	if ( itsPlotter_ != NULL ){
		symbol = itsPlotter_->createSymbol( copy );
	}
	return symbol;
}

PlotSymbolPtr PlotMSApp::createSymbol (const String& descriptor,
    		Int size, const String& color,
        	const String& fillPattern, bool outline ){
	PlotSymbolPtr symbol;
	if ( itsPlotter_ != NULL ){
		symbol = itsPlotter_->createSymbol( descriptor, size, color, fillPattern, outline );
	}
	return symbol;
}

PlotLoggerPtr PlotMSApp::getLogger() { return itsLogger_; }
PlotMSPlotManager& PlotMSApp::getPlotManager() { return itsPlotManager_; }

/*PlotMSPlot* PlotMSApp::addSinglePlot(const PlotMSPlotParameters* p) {
    return itsPlotManager_.addSinglePlot(p); }
*/
/*PlotMSPlot* PlotMSApp::addMultiPlot(const PlotMSPlotParameters* p) {
    return itsPlotManager_.addMultiPlot(p); }
*/
/*PlotMSPlot* PlotMSApp::addIterPlot(const PlotMSPlotParameters* p) {
    return itsPlotManager_.addIterPlot(p); }
*/
PlotMSOverPlot* PlotMSApp::addOverPlot(const PlotMSPlotParameters* p) {
    return itsPlotManager_.addOverPlot(p); }

bool PlotMSApp::isDrawing() const {
	return itsPlotter_->isDrawing();
}

void PlotMSApp::setAnnotationModeActive( PlotMSAction::Type type, bool active ){
	itsPlotter_->setAnnotationModeActive( type, active );
}

bool PlotMSApp::isClosed() const {
	return itsPlotter_ == NULL ||
               itsPlotter_->isClosed();
}

bool PlotMSApp::save(const PlotExportFormat& format, const bool interactive) {
	return itsPlotter_->exportPlot(format, interactive, false);
}

PlotFactoryPtr PlotMSApp::getPlotFactory(){
	return itsPlotter_->getPlotFactory();
}

void PlotMSApp::quitApplication(){
	CountedPtr<PlotMSAction> quitAction = ActionFactory::getAction( PlotMSAction::QUIT, NULL );
	quitAction->doAction( this );
}

PlotMSFlagging PlotMSApp::getFlagging() const {
	return itsPlotter_->getFlagging();
}

void PlotMSApp::setFlagging(PlotMSFlagging flag){
	itsPlotter_->setFlagging( flag );
}

void PlotMSApp::canvasAdded( PlotCanvasPtr canvas ){
	itsPlotter_->canvasAdded( canvas );
}

bool PlotMSApp::exportToFormat(const PlotExportFormat& format){
	return itsPlotter_->exportToFormat( format );
}

bool PlotMSApp::isVisible(PlotCanvasPtr& canvas ) {
	return itsPlotter_->isVisible( canvas );
}

Record PlotMSApp::locateInfo( Bool& success, String& errorMessage ){
	CountedPtr<PlotMSAction> action = ActionFactory::getAction(PlotMSAction::SEL_INFO, NULL );
	Record retval;
	success = action->doActionWithResponse( this, retval );
	if(! success ) {
		errorMessage = action->doActionResult();
	}
	return retval;
}

PlotterPtr PlotMSApp::getPlotter(){
	return itsPlotter_->getPlotter();
}

bool PlotMSApp::updateCachePlot( PlotMSPlot* plot,
		void (*f)(void*, bool), bool setupPlot){
	 ActionCacheLoad loadCacheAction( itsPlotter_, plot, f);
	 bool threading = itsPlotter_->isInteractive();

	 loadCacheAction.setUseThreading( threading );
	 loadCacheAction.setSetupPlot( setupPlot );
	 bool result = loadCacheAction.doAction( this );
	 return result;
}
// Private Methods //

void PlotMSApp::initialize(bool connectToDBus, bool userGui ) {

	its_want_avoid_popups=false;
	
    itsParameters_.addWatcher(this);
    ClientFactory::ClientType clientType = ClientFactory::GUI;
    if ( !userGui ){
    	clientType = ClientFactory::SCRIPT;
    }

    itsPlotter_ = ClientFactory::makeClient( clientType, this );
    //itsPlotter_->showIterationButtons(true);
    itsLogger_ = itsPlotter_->getLogger();
    
    itsPlotManager_.setParent(this);
    
    // Update internal state to reflect parameters.
    parametersHaveChanged(itsParameters_,
            PlotMSWatchedParameters::ALL_UPDATE_FLAGS());
    
    if(connectToDBus) {
        itsDBus_ = new PlotMSDBusApp(*this);
        itsDBus_->connectToDBus();
    }
}



}

