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
#include "SearchMoleculesWidget.qo.h"
#include <casa/System/Aipsrc.h>

#include <display/QtPlotter/Util.h>
#include <display/Display/Options.h>
#include <spectrallines/Splatalogue/SplatalogueTable.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDoppler.h>
#include <casa/Quanta/MVDoppler.h>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QDebug>
#include <assert.h>
#include <iostream>
using namespace std;

namespace casa {

const QString SearchMoleculesWidget::SPLATALOGUE_UNITS="GHz";
const double SearchMoleculesWidget::SPLATALOGUE_DEFAULT_MIN = 84;
const double SearchMoleculesWidget::SPLATALOGUE_DEFAULT_MAX = 90;

QString SearchMoleculesWidget::initialReferenceStr = "LSRK";

void SearchMoleculesWidget::setInitialReferenceFrame( QString frameStr ){

	initialReferenceStr = frameStr;
}


SearchMoleculesWidget::SearchMoleculesWidget(QWidget *parent)
    : QWidget(parent), unitStr( SPLATALOGUE_UNITS ), searchThread( NULL ),
      progressBar( this ), resultDisplay( NULL )
{
	ui.setupUi(this);

	//Progress Bar
	progressBar.setWindowTitle( "Line Search");
	progressBar.setLabelText( "Searching for molecular lines ...");
	progressBar.setWindowModality( Qt::WindowModal );
	progressBar.setCancelButton( 0 );
	progressBar.setMinimum(0);
	progressBar.setMaximum(0);

	//Range initialization
	QList<QString> frequencyUnitList;
	frequencyUnitList << "Hz" << "KHz" << "MHz" << "GHz" << "A" << "nm"
			<< "um" << "mm" << "cm" << "m";
	for ( int i = 0; i < frequencyUnitList.size(); i++ ){
		ui.rangeUnitComboBox->addItem( frequencyUnitList[i]);
	}
	ui.rangeUnitComboBox->setCurrentIndex(frequencyUnitList.indexOf(SPLATALOGUE_UNITS));
	this->unitStr = SPLATALOGUE_UNITS;
	connect( ui.rangeUnitComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT( searchUnitsChanged(const QString&)));

	//Astronomical Filters
	QList<QString> astronomicalFiltersList;
	astronomicalFiltersList<<"None"<<"Top 20 list"<<"Planetary Atmosphere"<<"Hot Cores"
			<<"Dark Clouds"<<"Diffuse Clouds"<<"Comets"<<"AGB/PPN/PN"<<"Extragalactic";
	for ( int i = 0; i < astronomicalFiltersList.size(); i++ ){
		ui.astronomicalFilterComboBox->addItem( astronomicalFiltersList[i] );
	}

	//Validators for the LineEdits
	QDoubleValidator* validator = new QDoubleValidator( this );
	ui.rangeMinLineEdit->setValidator( validator );
	ui.rangeMaxLineEdit->setValidator( validator );
	ui.dopplerLineEdit->setValidator( validator );
	ui.dopplerLineEdit->setText( QString::number(0) );

	radialVelocityTypeMap.insert("LSRK", MRadialVelocity::LSRK);
	radialVelocityTypeMap.insert("LSRD", MRadialVelocity::LSRD);
	radialVelocityTypeMap.insert("BARY", MRadialVelocity::BARY);
	radialVelocityTypeMap.insert("TOPO", MRadialVelocity::TOPO);
	QList<QString> keys = radialVelocityTypeMap.keys();
	for ( int i = 0; i < keys.size(); i++ ){
		ui.referenceFrameCombo->addItem( keys[i] );
	}
	velocityUnitsList<<"m/s"<<"km/s";
	for ( int i = 0; i < velocityUnitsList.size(); i++ ){
		ui.dopplerUnitsComboBox->addItem( velocityUnitsList[i] );
	}
	dopplerTypeMap.insert( "Radio", MDoppler::RADIO);
	dopplerTypeMap.insert( "Optical", MDoppler::OPTICAL);
	dopplerTypeMap.insert( "Relativistic", MDoppler::RELATIVISTIC );
	int radioIndex = 0;
	QList<QString> dopplerKeys = dopplerTypeMap.keys();
	for ( int i = 0; i < dopplerKeys.size(); i++ ){
		ui.dopplerTypeCombo->addItem( dopplerKeys[i] );
		if ( dopplerTypeMap[dopplerKeys[i]]  == MDoppler::RADIO ){
			radioIndex = i;
		}
	}
	ui.dopplerTypeCombo->setCurrentIndex( radioIndex );

	//Trigger the ui to update based on the default redshift
	ui.redshiftRadio->setChecked( true );
	dopplerInVelocity = true;
	this->dopplerShiftChanged();


	//Signal/Slot
	connect( ui.searchButton, SIGNAL(clicked()), this, SLOT(search()));
	connect( ui.browseButton, SIGNAL(clicked()), this, SLOT(openCatalog()));
	connect( ui.velocityRadio, SIGNAL(clicked()), this, SLOT(dopplerShiftChanged()));
	connect( ui.redshiftRadio, SIGNAL(clicked()), this, SLOT(dopplerShiftChanged()));
	connect( ui.dopplerUnitsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(dopplerVelocityUnitsChanged()));

	//Database initialization
	Bool foundDatabase = Aipsrc::find(defaultDatabasePath, "user.ephemerides.SplatDefault.tbl");
	if( !foundDatabase ){
		foundDatabase = Aipsrc::findDir(defaultDatabasePath, "data/ephemerides/SplatDefault.tbl");
	}
	if ( foundDatabase ){
		ui.catalogueLineEdit->setText( "Splatalogue");
	}
}

//-------------------------------------------------------------------------------------
//                        Initialization
//-------------------------------------------------------------------------------------


void SearchMoleculesWidget::setRange( float min, float max, QString units ){
	if ( unitStr != units ){
		Converter* converter = Converter::getConverter( units, unitStr );
		min = converter->convert( min );
		max = converter->convert( max );
		delete converter;
	}
	ui.rangeMinLineEdit->setText( QString::number( min ));
	ui.rangeMaxLineEdit->setText( QString::number( max ));
}

void SearchMoleculesWidget::setResultDisplay( SearchMoleculesResultDisplayer* resultDisplay ){
	this->resultDisplay = resultDisplay;
}

void SearchMoleculesWidget::updateReferenceFrame(){
	QList<QString> keys = radialVelocityTypeMap.keys();
	for ( int i = 0; i < keys.size(); i++ ){
		if ( keys[i]  == initialReferenceStr ){
			ui.referenceFrameCombo->setCurrentIndex(i);
			break;
		}
	}
}
//--------------------------------------------------------------------------------------
//                        Signal/Slot
//--------------------------------------------------------------------------------------

void SearchMoleculesWidget::openCatalog(){
	QString dPath = QFileDialog::getExistingDirectory( this,
			"Select Spectral Line Catalog",
			"/home");
	databasePath = dPath.toStdString();
	ui.catalogueLineEdit->setText( dPath );
}

void SearchMoleculesWidget::convertRangeLineEdit( QLineEdit* lineEdit, Converter* converter ){
	QString editText = lineEdit->text();
	if ( editText != NULL && !editText.isEmpty()){
		if ( converter != NULL ){
			double val = editText.toDouble();
			val = converter->convert( val );
			lineEdit->setText( QString::number( val ));
		}
	}
}


void SearchMoleculesWidget::searchUnitsChanged( const QString& newUnits ){
	//If the units have changed
	if ( unitStr != newUnits ){
		//Readjust the text fields
		Converter* converter = Converter::getConverter( unitStr, newUnits );
		convertRangeLineEdit(ui.rangeMinLineEdit, converter);
		convertRangeLineEdit(ui.rangeMaxLineEdit, converter);
		delete converter;
	}
	unitStr = newUnits;
}

void SearchMoleculesWidget::dopplerShiftChanged(){
	//Redshift is unitless so we hide the unit combo if the doppler shift
	//is specified with a redshift.  We also hide the reference frame.
	bool unitsVelocity = ui.velocityRadio->isChecked();
	ui.dopplerUnitsComboBox->setVisible( unitsVelocity );
	ui.referenceFrameLabel->setVisible( unitsVelocity );
	ui.referenceFrameCombo->setVisible( unitsVelocity );

	if ( unitsVelocity != dopplerInVelocity ){
		//We need to convert a value in doppler text field to the new units.
		//Make sure there is a value to convert:
		QString valStr = ui.dopplerLineEdit->text();
		if ( valStr.length() > 0 ){
			QString unitStr = ui.dopplerUnitsComboBox->currentText();
			String unitString = unitStr.toStdString();
			double val = valStr.toDouble();
			QString referenceStr = ui.referenceFrameCombo->currentText();
			MRadialVelocity::Types referenceType = radialVelocityTypeMap.value( referenceStr );
			QString dopplerTypeStr = ui.dopplerTypeCombo->currentText();
			MDoppler::Types dopplerType = dopplerTypeMap.value( dopplerTypeStr );
			Bool valid = true;
			if ( unitsVelocity ){
				//From doppler to velocity
				MVDoppler mvDoppler( val );
				MDoppler doppler( mvDoppler, dopplerType );
				MRadialVelocity mVelocity = MRadialVelocity::fromDoppler(doppler, referenceType);
				Quantity velQuantity = mVelocity.get( unitString );
				val = velQuantity.getValue();
			}
			else {
				//From velocity to doppler
				MDoppler doppler = MDoppler::Convert ( MRadialVelocity( Quantity(val, unitString),
							                 referenceType).toDoppler(),dopplerType)();
				val = doppler.getValue ();
				if ( isnan( val )){
					valid = false;
				}
			}
			if ( valid ){
				ui.dopplerLineEdit->setText( QString::number( val ));
			}
			else {
				ui.dopplerLineEdit->setText( "" );
			}
		}
		dopplerInVelocity = unitsVelocity;
	}
}

void SearchMoleculesWidget::dopplerVelocityUnitsChanged(){
	QString currentUnits = ui.dopplerUnitsComboBox->currentText();
	if ( dopplerVelocityUnitStr != currentUnits ){
		QString dopplerStr = ui.dopplerLineEdit->text();
		if (dopplerStr.length() > 0 ){
			double dopplerVal = dopplerStr.toDouble();
			Converter* converter = Converter::getConverter( dopplerVelocityUnitStr, currentUnits );
			dopplerVal = converter->convert( dopplerVal );
			ui.dopplerLineEdit->setText( QString::number( dopplerVal ));
			delete converter;
		}
		dopplerVelocityUnitStr = currentUnits;
	}
}



//------------------------------------------------------------------------------------
//                 Performing the search and displaying the results
//-----------------------------------------------------------------------------------

void SearchMoleculesWidget::initializeSearchRange( QLineEdit* lineEdit, Double& value/*, MDoppler redShift*/ ){
	QString valueStr = lineEdit->text();
	if ( !valueStr.isEmpty() ){
		value = valueStr.toDouble();
		//Convert the range value to Splatalogue Units
		if ( unitStr != SPLATALOGUE_UNITS ){
			Converter* converter = Converter::getConverter(unitStr, SPLATALOGUE_UNITS);
			value = converter->convert( value );
			delete converter;
		}

		//Factor in the specified redshift.
		value = getRedShiftedValue( true, value );
	}
}

double SearchMoleculesWidget::getRedShiftedValue( bool reverseRedshift, double value ) const{
	Vector<Double> inputValues(1);
	inputValues[0] = value;
	Unit splatalogueUnit( SPLATALOGUE_UNITS.toStdString());
	Quantum< Vector<Double> > quantum( inputValues, splatalogueUnit );
	MDoppler doppler = getRedShiftAdjustment( reverseRedshift );
	Quantum< Vector<Double> > outputQuantum = doppler.shiftFrequency(inputValues);
	Vector<Double> outputValues = outputQuantum.getValue();
	double result = outputValues[0];
	return result;
}

void SearchMoleculesWidget::setAstronomicalFilters( Searcher* searcher ){
	int filterIndex = ui.astronomicalFilterComboBox->currentIndex();
	if ( filterIndex > 0 ){
		switch( filterIndex ){
		case TOP_20:
			searcher->setAstroFilterTop20();
			break;
		case PLANETARY_ATMOSPHERE:
			searcher->setAstroFilterPlanetaryAtmosphere();
			break;
		case HOT_CORES:
			searcher->setAstroFilterHotCores();
			break;
		case DARK_CLOUDS:
			searcher->setAstroFilterDarkClouds();
			break;
		case DIFFUSE_CLOUDS:
			searcher->setAstroFilterDiffuseClouds();
			break;
		case COMETS:
			searcher->setAstroFilterComets();
			break;
		case AGB_PPN_PN:
			searcher->setAstroFilterAgbPpnPn();
			break;
		case EXTRAGALACTIC:
			searcher->setAstroFilterExtragalactic();
			break;
		default:
			assert( false );
		}
	}
}

void SearchMoleculesWidget::search(){

	//First decide if we have a database to search
	//We will try to use a user specified one.  Otherwise, we
	//will try the default.
	bool localSearcher = isLocal();
	String dPath = getDatabasePath();

	//Acquire the searcher that will do the search for us.
	Searcher* searcher = SearcherFactory::getSearcher( localSearcher );
	searcher->setDatabasePath( dPath );

	//Get the search parameters
	QString searchList = ui.searchLineEdit->text();
	QList<QString> moleculeList;
	if ( ! searchList.isEmpty() ){
		moleculeList = searchList.split(",");
	}
	Vector<String> chemNames( moleculeList.size());
	for ( int i = 0; i < moleculeList.size(); i++ ){
		chemNames[i] = moleculeList[i].trimmed().toStdString();
	}
	searcher->setChemicalNames( chemNames );

	//Create a temporary file for the search results
	String resultTableName = viewer::options.temporaryPath("SearcheMoleculesResults");
	searcher->setResultFile( resultTableName );

	//Set the range for the search
	Double minValue = SPLATALOGUE_DEFAULT_MIN;
	Double maxValue = SPLATALOGUE_DEFAULT_MAX;
	//MDoppler redShift = getRedShiftAdjustment();
	initializeSearchRange( ui.rangeMinLineEdit, minValue/*, redShift*/ );
	initializeSearchRange( ui.rangeMaxLineEdit, maxValue/*, redShift*/ );
	if ( minValue > maxValue ){
		double tmp = minValue;
		minValue = maxValue;
		maxValue = tmp;
	}
	searcher->setSearchRangeFrequency( minValue, maxValue );

	//Set any astronomical filters for the search
	setAstronomicalFilters( searcher );

	//Start the background thread that will do the search
	delete searchThread;
	searchThread = new SearchThread( searcher );
	connect( searchThread, SIGNAL( finished() ), this, SLOT(searchFinished()));
	searchThread->start();
	progressBar.show();
}


void SearchMoleculesWidget::searchFinished(){

	String errorMsg = searchThread->getErrorMessage();
	if ( errorMsg.length() == 0 ){
		searchResults = searchThread->getResults();
		bool noResults = searchResults.empty();
		if ( noResults ){
			progressBar.hide();
		}
		resultDisplay ->displaySearchResults( searchResults );
		emit searchCompleted();
	}
	else {
		progressBar.hide();
		QString errorMessage = errorMsg.c_str();
		Util::showUserMessage( errorMessage, this );
	}
	progressBar.hide();
}


MDoppler SearchMoleculesWidget::getRedShiftAdjustment( bool reverseDirection ) const{
	QString redShiftStr = ui.dopplerLineEdit->text();
	MDoppler::Types dopplerType = getDopplerType();
	MVDoppler defaultValue( 0 );
	MDoppler doppler( defaultValue, dopplerType );
	if ( ! redShiftStr.isEmpty() ){
		double redShift = redShiftStr.toDouble();
		if ( ui.redshiftRadio->isChecked() ){
			//Set the value of the doppler
			if ( reverseDirection ){
				redShift = redShift * -1;
			}
			MVDoppler mvDoppler( redShift);
			doppler.set(mvDoppler);
		}
		else {
			//Velocity units:  convert to a doppler
			QString unitString = ui.dopplerUnitsComboBox->currentText();
			MRadialVelocity::Types referenceType = getReferenceFrame();
			String unitStr( unitString.toStdString());
			MDoppler baseDoppler = MDoppler::Convert ( MRadialVelocity( Quantity(redShift, unitStr ),
										                 referenceType).toDoppler(),dopplerType)();
			//Reverse the sign
			MVDoppler mvValue = baseDoppler.getValue();
			double redshiftVal = mvValue.getValue();
			if ( reverseDirection ){
				redshiftVal = redshiftVal * -1;
			}
			MVDoppler reverseMvValue( redshiftVal );
			doppler.set( reverseMvValue );
		}
	}
	return doppler;
}
//-----------------------------------------------------------------------------
//                        Accessors
//-----------------------------------------------------------------------------

QString SearchMoleculesWidget::getUnit() const {
	return unitStr;
}

Record SearchMoleculesWidget::getSearchResults() const {
	return searchResults;
}

bool SearchMoleculesWidget::isLocal() const {
	//Note:  this will need to be reimplemented when we have access
	//to the real splatalogue.
	return true;
}

String SearchMoleculesWidget::getDatabasePath() const {
	String dPath = defaultDatabasePath;
	if ( databasePath.length() > 0 ){
		dPath = databasePath;
	}
	return dPath;
}

MDoppler::Types SearchMoleculesWidget::getDopplerType() const {
	QString dopplerStr = ui.dopplerTypeCombo->currentText();
	MDoppler::Types dopplerType = dopplerTypeMap[dopplerStr];
	return dopplerType;
}
MRadialVelocity::Types SearchMoleculesWidget::getReferenceFrame() const{
	QString referenceStr = ui.referenceFrameCombo->currentText();
	MRadialVelocity::Types velocityType = radialVelocityTypeMap[referenceStr ];
	return velocityType;
}

SearchMoleculesWidget::~SearchMoleculesWidget()
{
	delete searchThread;
}
}
