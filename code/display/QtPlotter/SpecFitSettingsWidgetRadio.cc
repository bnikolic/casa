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
#include "SpecFitSettingsWidgetRadio.qo.h"
#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>
#include <images/Images/ImageFit1D.h>
#include <components/SpectralComponents/PCFSpectralElement.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/SpecFitLogDialog.qo.h>
#include <display/QtPlotter/SpecFitSettingsFixedTableCell.qo.h>
#include <display/QtPlotter/SpecFit.h>
#include <display/QtPlotter/SpecFitPolynomial.h>
#include <display/QtPlotter/SpecFitGaussian.h>
#include <display/QtPlotter/Util.h>
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <display/QtPlotter/conversion/ConverterChannel.h>
#include <display/Display/Options.h>

#include <QFileDialog>
#include <QTemporaryFile>
#include <QMutableMapIterator>
#include <QList>
#include <QDebug>
#include <QThread>
#include <sys/time.h>




namespace casa {

/**
 * Responsible for running the spectral fitting calculations in
 * the background so that we don't freeze the GUI.
 */
class SpecFitThread : public QThread {
public:
	SpecFitThread( ImageProfileFitter* profileFitter );
	void run();
	void setStartValue( float val );
	void setEndValue( float val );
	float getStartValue() const;
	float getEndValue() const;
	String getErrorMessage() const;
	const Record& getResults() const;
private:
	ImageProfileFitter* fitter;
	Record results;
	float startValue;
	float endValue;
	String errorMsg;
};

const Record& SpecFitThread::getResults() const {
	return results;
}

String SpecFitThread::getErrorMessage() const {
	return errorMsg;
}

SpecFitThread::SpecFitThread( ImageProfileFitter* profileFitter ):
		fitter( profileFitter ), errorMsg(""){
}

void SpecFitThread::setStartValue( float val ){
	startValue = val;
}
void SpecFitThread::setEndValue( float val ){
	endValue = val;
}

float SpecFitThread::getStartValue() const {
	return startValue;
}
float SpecFitThread::getEndValue() const {
	return endValue;
}
void SpecFitThread::run(){
	try {
		results = fitter->fit();
	}
	catch( AipsError aipsError ){
		errorMsg = aipsError.getMesg();
	}
}


//-------------------------------------------------------------------------------


SpecFitSettingsWidgetRadio::SpecFitSettingsWidgetRadio(QWidget *parent)
    : QWidget(parent), fitter( NULL ), specFitThread( NULL ),
      progressDialog("Calculating fit(s)...", "Cancel", 0, 100, this),
      plotDialog( this ), gaussEstimateDialog( this ),
      POINT_COUNT(20), SUM_FIT_INDEX(-1)
{
	ui.setupUi(this);


	progressDialog.setModal( true );
	connect( &progressDialog, SIGNAL(canceled()), this, SLOT(cancelFit()));

	//Until we get the code written
	ui.loadButton->setVisible( false );
	ui.plotButton->setVisible( false );

	//Text fields
	QValidator *validator = new QDoubleValidator(-1.0e+32, 1.0e+32,10,this);
	ui.minLineEdit->setValidator(validator);
	ui.maxLineEdit->setValidator(validator);

	ui.estimateTable->setColumnCount( END_COLUMN );
	QStringList tableHeaders =(QStringList()<< "  Peak  " << " Center " << "  FWHM  " << "          Fixed          ");
	ui.estimateTable->setHorizontalHeaderLabels( tableHeaders );
	ui.estimateTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.estimateTable->setSelectionMode( QAbstractItemView::SingleSelection );

	int state = ui.polyFitCheckBox->checkState();
	polyFitChanged( state );
	int saveOutput = ui.saveOutputCheckBox->checkState();
	saveOutputChanged( saveOutput );

	connect( ui.polyFitCheckBox, SIGNAL(stateChanged(int)), this, SLOT(polyFitChanged(int)) );
	connect( ui.gaussCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(gaussCountChanged(int)));
	connect( ui.fitButton, SIGNAL(clicked()), this, SLOT(specLineFit()));
	connect( ui.plotButton, SIGNAL(clicked()), this, SLOT(showPlots()));
	connect( ui.cleanButton, SIGNAL(clicked()), this, SLOT(clean()));
	connect( ui.saveButton, SIGNAL(clicked()), this, SLOT(setOutputLogFile()));
	connect( ui.viewButton, SIGNAL(clicked()), this, SLOT(viewOutputLogFile()));
	connect( ui.saveOutputCheckBox, SIGNAL(stateChanged(int)), this, SLOT(saveOutputChanged(int)));
	connect( ui.advancedGaussianEstimatesButton, SIGNAL(clicked()), this, SLOT(specifyGaussianEstimates()));
	connect( &gaussEstimateDialog, SIGNAL( accepted()), this, SLOT(gaussianEstimatesChanged()));
	//connect( &gaussEstimateDialog, SIGNAL( gaussEstimateUnitsChanged(QString)), this, SLOT( gaussEstimateUnitsChanged(QString)));

	ui.viewButton->setEnabled( false );
	ui.plotButton->setEnabled( false );
	ui.advancedGaussianEstimatesButton->setEnabled( false );
}

void SpecFitSettingsWidgetRadio::setCanvas( QtCanvas* canvas ){
	ProfileTaskFacilitator::setCanvas( canvas );
	connect( pixelCanvas, SIGNAL(specFitEstimateSpecified(double,double,bool)),
			this, SLOT(specFitEstimateSpecified(double,double,bool)));
}

void SpecFitSettingsWidgetRadio::setEstimateValue( int row, int col, double val ){
	QTableWidgetItem* peakItem = new QTableWidgetItem();
	peakItem -> setText( QString::number( val ) );
	ui.estimateTable->setItem( row, col, peakItem );
}


void SpecFitSettingsWidgetRadio::specFitEstimateSpecified(double xValue,
		double yValue, bool centerPeak ){

	QList<QTableWidgetSelectionRange> selectionRanges = ui.estimateTable->selectedRanges();
	int selectionCount = selectionRanges.length();
	if ( selectionCount == 0 ){
		if ( ui.estimateTable->isVisible()){
			QString msg( "Please select a row in the initial Gaussian estimates table before specifying the estimate.");
			Util::showUserMessage( msg, this );
		}
	}
	else if ( selectionCount == 1 ){
		QTableWidgetSelectionRange selectionRange = selectionRanges[0];
		//We should have a single row selected
		int rowCount = selectionRange.rowCount();
		if ( rowCount == 1 ){
			int row = selectionRange.bottomRow();
			if ( centerPeak ){
				setEstimateValue( row, PEAK, yValue );
				setEstimateValue( row, CENTER, xValue );
				pixelCanvas->setProfileFitMarkerCenterPeak( row, xValue, yValue );
			}
			else {
				//Take the distance between the center and xValue, then
				//double it to get the fwhm.
				QTableWidgetItem* centerItem = ui.estimateTable->item( row, CENTER );
				QString centerStr = centerItem->text();
				bool validNum;
				double val = centerStr.toDouble( &validNum );
				if ( validNum ){
					double fwhm = 2 * abs(val - xValue);
					setEstimateValue( row, FWHM, fwhm);
					pixelCanvas -> setProfileFitMarkerFWHM( row, fwhm, yValue );
				}
			}
			//Update the profile fit marker in this row.
		}

	}

}

void SpecFitSettingsWidgetRadio::reset(){
	if ( fitter != NULL ){
		delete fitter;
		fitter = NULL;
	}
	if ( specFitThread != NULL ){
		delete specFitThread;
		specFitThread = NULL;
	}

	//Empty the curves and their associated data.
	while (!curveList.isEmpty()) {
	     QList<SpecFit*> curves = curveList.takeFirst();
	     while( !curves.isEmpty()){
	     	SpecFit* specFit = curves.takeFirst();
	     	delete specFit;
	     }
	 }

}

void SpecFitSettingsWidgetRadio::clean(){
	plotMainCurve();

}

void SpecFitSettingsWidgetRadio::clear(){
	ui.minLineEdit->setText("");
	ui.maxLineEdit->setText("");
	clearEstimates();
}


void SpecFitSettingsWidgetRadio::setUnits( QString units ){
	int bracketIndex = units.indexOf( "[]");
	if ( bracketIndex > 0 ){
		units = "";
	}
	QString oldUnits = ui.rangeGroupBox->title();
	if ( oldUnits != units ){
		clear();
		int startIndex = units.indexOf( "[");
		int endIndex = units.indexOf( "]");
		if ( startIndex > 0 && endIndex > 0 ){
			units = units.mid(startIndex, endIndex + 1 - startIndex);
		}
		ui.rangeGroupBox->setTitle( units );
	}
}

void SpecFitSettingsWidgetRadio::gaussCountChanged( int count ){
	int oldRowCount = ui.estimateTable->rowCount();
	//We are adding rows.
	if ( oldRowCount < count ){
		for ( int i = oldRowCount; i < count; i++ ){
			ui.estimateTable->insertRow( i );
			SpecFitSettingsFixedTableCell* cellWidget = new SpecFitSettingsFixedTableCell(ui.estimateTable);
			ui.estimateTable->setCellWidget( i, FIXED, cellWidget );
			ui.estimateTable->resizeRowToContents( i );
		}
	}
	else if ( oldRowCount > count ){
		for ( int i = oldRowCount-1; i>=count; i--){
			SpecFitSettingsFixedTableCell* cellWidget = dynamic_cast<SpecFitSettingsFixedTableCell*>(ui.estimateTable->item(i, FIXED ));
			delete cellWidget;
			ui.estimateTable->removeRow(i);
		}
	}
	ui.estimateTable->resizeColumnToContents( FIXED );

	//Enable/disable the advanced gaussian estimate button based
	//on whether there will be any Gaussians to fit.
	if ( count > 0 ){
		ui.advancedGaussianEstimatesButton->setEnabled( true );
	}
	else {
		ui.advancedGaussianEstimatesButton->setEnabled( false );
	}
	gaussEstimateDialog.setGaussCount( ui.gaussCountSpinBox->value() );
}

void SpecFitSettingsWidgetRadio::setRange(float xmin, float xmax ){
	if (xmax < xmin){
		ui.minLineEdit->clear();
		ui.maxLineEdit->clear();
	}
	else {
		QString startStr;
		QString endStr;
		startStr.setNum(xmin);
		endStr.setNum(xmax);
		ui.minLineEdit->setText(startStr);
		ui.maxLineEdit->setText(endStr);
	}
}

void SpecFitSettingsWidgetRadio::polyFitChanged( int state ){
	if ( state == Qt::Checked ){
		ui.polyOrderSpinBox -> setEnabled( true );
	}
	else {
		ui.polyOrderSpinBox -> setEnabled( false );
	}
}

void SpecFitSettingsWidgetRadio::saveOutputChanged( int state ){
	bool saveOutput = false;
	if ( state == Qt::Checked ){
		saveOutput = true;
	}
	else {
		outputLogPath="";
	}
	ui.saveButton->setEnabled( saveOutput );
}

bool SpecFitSettingsWidgetRadio::isValidFitSpecification( int gaussCount, bool polyFit ){
	bool valid = true;
	if (gaussCount <= 0 && !polyFit){
		String msg("Please indicate the type of fit:  Gaussian and/or Polynomial.");
		logWarning( msg );
		postStatus( msg );
		QString userMsg(msg.c_str());
		Util::showUserMessage(userMsg, this);
		valid = false;
	}
	return valid;
}




bool SpecFitSettingsWidgetRadio::isValidEstimate( QString& peakStr,
		QString& centerStr, QString& fwhmStr,
		QString& fixedStr, int rowIndex ) {
	bool validEstimate = true;
	QDoubleValidator validator;
	int pos = 0;
	QString msg( "Row "+QString::number(rowIndex)+" of the initial estimate table is invalid.\n");
	if ( ! validator.validate(peakStr, pos) == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "Please check that the following numbers are valid:\n");
		msg.append( "  peak");
	}
	if ( !validator.validate(centerStr, pos) == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "\n  center");
	}
	if ( !validator.validate( fwhmStr, pos) == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "\n  FWHM");
	}
	//Check that the fixed string contains
	//zero or one instance of the characters "pcf" in any order.
	QRegExp fixedRe("p?c?f?|p?f?c?|c?p?f?|c?f?p?|f?c?p?|f?p?c?");
	QRegExpValidator validatorFixed(fixedRe, 0 );
	if ( !validatorFixed.validate(fixedStr, pos)  == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "\nPlease check that the fixed string contains only the characters 'p', 'c', and 'f' at most once in any order.");
	}
	if ( !validEstimate ){
		Util::showUserMessage( msg, this );
	}

	return validEstimate;
}

void SpecFitSettingsWidgetRadio::getEstimateStrings( int index, QString& peakStr, QString& centerStr, QString& fwhmStr ) const {
	QTableWidgetItem* peakItem = ui.estimateTable->item(index, PEAK );
	if ( peakItem != NULL ){
		peakStr = peakItem->text();
	}
	QTableWidgetItem* centerItem = ui.estimateTable->item( index, CENTER );
	if ( centerItem != NULL ){
		centerStr = centerItem->text();
	}
	QTableWidgetItem* fwhmItem = ui.estimateTable->item( index, FWHM );
	if ( fwhmItem != NULL ){
		fwhmStr = fwhmItem->text();
	}
}

SpectralList SpecFitSettingsWidgetRadio::buildSpectralList( int nGauss,
		Bool& validEstimates ){
	SpectralList spectralList;
	validEstimates = true;
	if ( nGauss > 0 ){
		int potentialEstimateCount = ui.estimateTable->rowCount();
		if ( nGauss == potentialEstimateCount ){
			for ( int i = 0; i < potentialEstimateCount; i++ ){
			    QString peakStr;
			    QString centerStr;
			    QString fwhmStr;
			    getEstimateStrings( i, peakStr, centerStr, fwhmStr );
				SpecFitSettingsFixedTableCell* fixedItem = dynamic_cast<SpecFitSettingsFixedTableCell*>(ui.estimateTable->cellWidget(i,FIXED));
				QString fixedStr;
				if ( fixedItem != NULL ){
					fixedStr = fixedItem->getFixedStr();
				}
				if ( isValidEstimate( peakStr, centerStr, fwhmStr, fixedStr, i+1 ) ){
					if ( !peakStr.isEmpty() && !centerStr.isEmpty() && !fwhmStr.isEmpty()){
						double peakVal = peakStr.toDouble();
						double centerVal = centerStr.toDouble();
						double fwhmVal = fwhmStr.toDouble();

						//Gaussian Estimate must be in pixels.
						double centerValPix = toPixels( centerVal );
						double fwhmValPtPix = toPixels( centerVal - fwhmVal );
						double fwhmValPix = fabs(centerValPix - fwhmValPtPix);

						//Peak values must be in the same units as the image.
						Unit imageUnit = this->getImage()->units();
						const String newUnits = imageUnit.getName();
						QString newUnitStr( newUnits.c_str() );
						QString fitCurveName = ui.curveComboBox->currentText();
						CanvasCurve curve = pixelCanvas->getCurve( fitCurveName );
						peakVal = curve.convertValue( peakVal, displayYUnits, newUnitStr );

						GaussianSpectralElement* estimate = new GaussianSpectralElement( peakVal, centerValPix, fwhmValPix);
						estimate->fixByString( fixedStr.toStdString());
						spectralList.add( *estimate );
					}

				}
				else {
					validEstimates = false;
					break;
				}
			}
		}

	}
	return spectralList;
}

SpectralCoordinate SpecFitSettingsWidgetRadio::getSpectralCoordinate() const {
	const ImageInterface<Float>* img = this->getImage();
	CoordinateSystem cSys = img->coordinates();
	SpectralCoordinate spectralCoordinate = cSys.spectralCoordinate();
	return spectralCoordinate;
}


double SpecFitSettingsWidgetRadio::toPixels( Double val) const {
	String xAxisUnit = getXAxisUnit();
	double pixelValue = val;
	//We are not already in pixels = channels
	if ( xAxisUnit.length() > 0 ){
		QString unitStr( xAxisUnit.c_str());
		Converter* converter = Converter::getConverter( unitStr, unitStr );
		pixelValue = converter->toPixel( val );
		delete converter;
	}
	return pixelValue;
}

void SpecFitSettingsWidgetRadio::doFit( float startVal, float endVal, uint nGauss, bool fitPoly, int polyN ){

	reset();
	QString fitCurveName = ui.curveComboBox->currentText();
	const ImageInterface<float>* image = getImage( fitCurveName );
	const String pixelBox = getPixelBox();

	Bool validSpectralList;
	SpectralList spectralList = buildSpectralList( nGauss, validSpectralList );
	if ( !validSpectralList ){
		return;
	}

	int startChannelIndex = -1;
	int endChannelIndex = -1;
	CanvasCurve curve = pixelCanvas->getCurve( fitCurveName );
	Vector<Float> z_xval = curve.getXValues();
	findChannelRange( startVal, endVal, z_xval, startChannelIndex, endChannelIndex );
	if ( startChannelIndex >= 0 && endChannelIndex >= 0 ){

		//Get the channels
		const String channelStr = String::toString( startChannelIndex )+ "~"+String::toString( endChannelIndex);

		//Get the fit axis, which is right now hard-coded to be the
		//spectral axis.
		CoordinateSystem cSys = image -> coordinates();
		Int spectralAxisNumber = cSys.spectralAxisNumber();

		//Initialize the fitter
		fitter = new ImageProfileFitter( image, "", 0, pixelBox,
				channelStr, "", "", spectralAxisNumber, static_cast<uInt>(nGauss), "",
				spectralList);

		//Tell the fitter if we want to include a polynomial
		if ( fitPoly ){
			fitter->setPolyOrder( polyN );
		}

		//Output logging will always be done even if the user doesn't
		//specify a log file.
		if ( outputLogPath.isEmpty() ){
			resolveOutputLogFile();
		}
		fitter->setLogfile( outputLogPath.toStdString() );

		//Tell the fitter if it is a multifit.
		if ( ui.multiFitCheckBox->isChecked() ){
			fitter->setDoMultiFit( true );
		}

		//Do the fit.
		specFitThread = new SpecFitThread( fitter );
		connect( specFitThread, SIGNAL(finished()), this, SLOT(fitDone()));
		specFitThread->setStartValue( startVal );
		specFitThread->setEndValue( endVal );
		specFitThread->start();
		fitCancelled = false;
		progressDialog.setValue( 0 );
		progressDialog.show();
	}
}

QString SpecFitSettingsWidgetRadio::settingsToString() const {
	QString logMessage = "\n\n\n-------------------------------------Viewer Settings-------------------------------------\n\n";
	logMessage.append( "Range:  Min="+ui.minLineEdit->text()+" Max="+ui.maxLineEdit->text()+" "+ this->getXAxisUnit().c_str()+"\n" );
	logMessage.append( "Multifit: ");
	const QString trueStr( "true" );
	const QString falseStr( "false" );
	if( ui.multiFitCheckBox->isChecked() ){
		logMessage.append( trueStr + "\n");
	}
	else {
		logMessage.append( falseStr +"\n");
	}
	logMessage.append( "Polynomial Fit:");
	if (ui.polyFitCheckBox->isChecked()){
		logMessage.append( trueStr );
		logMessage.append(" Order="+QString::number(ui.polyOrderSpinBox->value())+"\n");
	}
	else {
		logMessage.append( falseStr );
		logMessage.append( "\n");
	}
	logMessage.append( "Gaussian Fit Count: "+ QString::number(ui.gaussCountSpinBox->value())+"\n");
	int estimateCount = ui.estimateTable->rowCount();
	for ( int i = 0; i < estimateCount; i++ ){
		QString peakStr;
		QString centerStr;
		QString fwhmStr;
		QString fixedStr;
		getEstimateStrings( i, peakStr, centerStr, fwhmStr );
		SpecFitSettingsFixedTableCell* fixedItem = dynamic_cast<SpecFitSettingsFixedTableCell*>(ui.estimateTable->cellWidget(i,FIXED));
		if ( fixedItem != NULL ){
			fixedStr = fixedItem->getFixedStr();
		}
		logMessage.append( "  Estimate "+QString::number((i+1))+":  Peak="+peakStr+" "+getYUnitPrefix()+getYUnit()+" Center="+centerStr+" "+
				getXAxisUnit().c_str()+" FWHM="+fwhmStr+" "+getXAxisUnit().c_str() );
		if ( !fixedStr.isNull() && !fixedStr.isEmpty()){
			logMessage.append(" Fixed="+fixedStr);
		}
		logMessage.append( "\n");
	}
	return logMessage;
}

void SpecFitSettingsWidgetRadio::fitDone(){
	float finishedProgress = 10;
	const int PROGRESS_END = 100;
	progressDialog.setValue( static_cast<int>(finishedProgress) );

	String errorMsg = specFitThread->getErrorMessage();
	if ( errorMsg.length() == 0 ){
		const Record& results = specFitThread->getResults();

		//Decide if we got any valid fits
		Array<Bool> succeeded = results.asArrayBool(ImageProfileFitter::_SUCCEEDED );
		Array<Bool> valid = results.asArrayBool( ImageProfileFitter::_VALID );
		float progressIncrement = 90.0f / ntrue( succeeded );
		if ( ntrue( succeeded ) > 0 && ntrue( valid ) ){

			//Tell the user basic information about the fit.
			Array<Bool> converged = results.asArrayBool( ImageProfileFitter::_CONVERGED );
			String msg( "Fit(s) succeeded: "+String::toString(ntrue(succeeded))+"   \n");
			msg.append( "Fit(s) converged: "+String::toString(ntrue(converged))+"   \n");
			msg.append( "Fit(s) valid: "+String::toString(ntrue(valid))+"   \n");
			postStatus( msg );


			//Initialize the x-values.  In the case of a polynomial fit, the
			//x values need to be in pixels.
			Vector<Float> xValues( POINT_COUNT );
			Vector<Float> xValuesPix( POINT_COUNT );
			float startVal = specFitThread->getStartValue();
			float endVal = specFitThread->getEndValue();
			float dx = (endVal - startVal) / POINT_COUNT;
			for( int i = 0; i < POINT_COUNT; i++ ){
				xValues[i] = startVal + i * dx;
				xValuesPix[i] = toPixels( xValues[i] );
			}

			//Go through each of the fits.  This could be 1 if multifit is not
			//checked.  On the other hand it could be hundreds if multifit is checked.
			std::vector<bool> successVector;
			succeeded.tovector( successVector );
			for ( int i = 0; i < static_cast<int>(successVector.size()); i++ ){
				if ( fitCancelled ){
					break;
				}
				//The fit succeeded.
				if ( successVector[i] ){
					//Initialize a SpecFit curve for the fit.
					processFitResults( xValues, xValuesPix );
				}
				finishedProgress = finishedProgress + progressIncrement;

				progressDialog.setValue( static_cast<int>(finishedProgress) );
			}

			progressDialog.setValue( PROGRESS_END );
			if ( !fitCancelled ){
				if ( !ui.multiFitCheckBox->isChecked() ){
					drawCurves(SUM_FIT_INDEX,SUM_FIT_INDEX);
				}
				else {
					QString msg( "Hover the mouse over the point\n in the selected rectangle of the Viewer Display Panel\n to see the fitted profile at each point");
					Util::showUserMessage( msg, this );
				}
			}
			viewOutputLogFile();
		}
		else {
			progressDialog.setValue( PROGRESS_END );
			String msg = String("Data could not be fitted!");
			QString msgStr(msg.c_str());
			postStatus(msg);
			Util::showUserMessage( msgStr, this);
		}
	}
	else {
		progressDialog.setValue( PROGRESS_END );
		QString msg( "Data could not be fitted!\n");
		msg.append( errorMsg.c_str());
		Util::showUserMessage( msg, this );
	}
}

void SpecFitSettingsWidgetRadio::drawCurves( int pixelX, int pixelY ){
	//Make sure we are in our selected rectangular region before
	//going to the trouble of trying to find the curves to draw.
	if ( pixelX != SUM_FIT_INDEX  && pixelY != SUM_FIT_INDEX ){
		Vector<double> xPixels;
		Vector<double> yPixels;
		taskMonitor->getPixelBounds( xPixels, yPixels );
		if ( xPixels.size() < 2 || yPixels.size() < 2 ){
			return;
		}
		if ( pixelX < xPixels[0] || pixelX > xPixels[1] ){
			return;
		}
		if ( pixelY < yPixels[0] || pixelY > yPixels[1] ){
			return;
		}
	}

	//Compute the yValues for the fit.
	Vector<Float> yCumValues(POINT_COUNT, 0);

	for ( int k = 0; k < curveList.size(); k++ ){

		QList<SpecFit*> curves = curveList[k];
		/*if ( !curves[0]->isSpecFitFor(pixelX, pixelY)){
			continue;
		}
		else {
			//Clear off the previous fit, if there was one.
			clean();
		}*/

		Vector<Float> xValues = curves[0]->getXValues();
		for ( int i = 0; i < curves.size(); i++ ){
			curves[i]->evaluate( xValues );

			Vector<Float> yValues = curves[i]->getYValues();
			QString curveName = curves[i]->getCurveName();

			//Send the curve to the canvas for plotting.
			pixelCanvas->addPolyLine( xValues, yValues, curveName, QtCanvas::CURVE_COLOR_PRIMARY);
			for ( int i = 0; i < POINT_COUNT; i++ ){
				yCumValues[i] = yCumValues[i]+yValues[i];
			}
		}

		//Add a curve that represents the sum of all the individual fits
		if ( curves.size() > 1 ){
			QString curveName = taskMonitor->getFileName() +"_Sum_FIT";
			pixelCanvas->addPolyLine( xValues, yCumValues, curveName, QtCanvas::CURVE_COLOR_SECONDARY);
		}
	}
}

void SpecFitSettingsWidgetRadio::processFitResults(
		Vector<float>& xValues, Vector<float>& xValuesPix){

	//Iterate through all the fits and post the results
	Array<ImageFit1D<Float> > image1DFitters = fitter-> getFitters();
	Array<ImageFit1D<Float> >::iterator iterend( image1DFitters.end());
	uint fitIndex = 0;
	for ( Array<ImageFit1D<Float> >::iterator iter = image1DFitters.begin(); iter != iterend; ++iter ){
		ImageFit1D<Float> image1DFitter = *iter;

		SpectralList solutions = image1DFitter.getList();

		//Find the center of the fit in pixels.
		int successCount = 0;
		QList<SpecFit*> curves;
		int centerX = SUM_FIT_INDEX;
		int centerY = SUM_FIT_INDEX;
		if ( ui.multiFitCheckBox->isChecked() ){
			Vector<Double> fitCenter = fitter->getPixelCenter(fitIndex);
			centerX = static_cast<int>(fitCenter[0]);
			centerY = static_cast<int>(fitCenter[1]);
		}
		fitIndex++;

		//Get the solutions for each fit
		for ( int j = 0; j < static_cast<int>(solutions.nelements()); j++  ){
			SpectralElement::Types fitType = solutions[j]->getType();

			bool successfulFit = false;
			if ( fitType == SpectralElement::GAUSSIAN ){
				successfulFit = processFitResultGaussian( solutions[j], j, curves );
			}
			else if (fitType == SpectralElement::POLYNOMIAL ){
				successfulFit = processFitResultPolynomial( solutions[j], curves );
			}

			//Add the fit values into the cumulative fit value vector.
			if ( successfulFit ){
				if ( curves[successCount] ->isXPixels() ){
					curves[successCount]->evaluate( xValuesPix );
					curves[successCount]->setXValues( xValues );
				}
				else {
					curves[successCount]->evaluate( xValues );
				}

				QString fitCurveName = ui.curveComboBox->currentText();
				QString curveName = /*taskMonitor->getFileName()*/fitCurveName +curves[successCount]->getSuffix();
				curves[successCount]->setCurveName( curveName );
				curves[successCount]->setFitCenter( centerX, centerY );
				successCount++;
			}
		}
		//store the curves in the curve map
		curveList.append( curves );

		if ( successCount == 0 ){
			QString msg( "Fit returned invalid value(s).");
			Util::showUserMessage( msg, this );
		}
	}
    ui.viewButton->setEnabled( !ui.multiFitCheckBox->isChecked() );
	ui.plotButton->setEnabled( !ui.multiFitCheckBox->isChecked() );
}


bool SpecFitSettingsWidgetRadio::processFitResultPolynomial( const SpectralElement* solution,
			QList<SpecFit*>& curves){

	bool successfulFit = true;
	const PolynomialSpectralElement* poly = dynamic_cast<const PolynomialSpectralElement*>(solution);

	//Get the coefficients of the polynomial in pixels
	Vector<Double> coefficients;
	poly ->get( coefficients );
	SpecFit* polyFit = new SpecFitPolynomial( coefficients );

	curves.append( polyFit );
	return successfulFit;
}



bool SpecFitSettingsWidgetRadio::processFitResultGaussian( const SpectralElement* solution,
		int index, QList<SpecFit*>& curves){

	//Get the center, peak, and fwhm from the estimate.  Make sure they are
	//valid values.
	const PCFSpectralElement *pcf = dynamic_cast<const PCFSpectralElement*>(solution);
	Double centerValPix = pcf->getCenter();
	double fwhmValPix = pcf->getFWHM();
	float peakVal = static_cast<float>(pcf->getAmpl());
	if ( isnan( fwhmValPix) || isnan( centerValPix ) || isnan( peakVal ) ||
			isinf( fwhmValPix) || isinf( centerValPix) || isinf( peakVal)){
		return false;
	}

	//The peak value will be in whatever units the
	//fit image is using.  We need to convert it to the image units that the canvas
	//knows about (which may not be the same thing if we are fitting a different
	//image than the canvas is using for the main display.
	const Unit imageUnit = this->getImage()->units();
	String imageUnits = imageUnit.getName();
	QString imageUnitsStr( imageUnits.c_str());
	if ( imageUnitsStr != imageYUnits ){
		QString fitCurveName = ui.curveComboBox->currentText();
		CanvasCurve fitCurve = pixelCanvas->getCurve( fitCurveName );
		double convertedPeakVal = fitCurve.convertValue( peakVal, imageUnitsStr, imageYUnits );
		peakVal = convertedPeakVal;
	}

	//Convert the center and fwhm from pixels to whatever units the canvas is
	//using.
	String xAxisUnit = getXAxisUnit();
	ImageInterface<float>* img = const_cast<ImageInterface<float>* >(taskMonitor->getImage());
	CoordinateSystem cSys = img->coordinates();
	int axisCount = cSys.nPixelAxes();
	IPosition imPos(axisCount);
	Bool velocityUnits;
	Bool wavelengthUnits;
	getConversion( xAxisUnit, velocityUnits, wavelengthUnits );
	float centerVal = static_cast<float>(fitter->getWorldValue(centerValPix, imPos, xAxisUnit,velocityUnits, wavelengthUnits));
	float fwhmValX = static_cast<float>(fitter->getWorldValue(fwhmValPix/2+centerValPix, imPos, xAxisUnit,velocityUnits, wavelengthUnits));
	float fwhmVal = 2 * abs(fwhmValX - centerVal);
	if ( isnan( centerVal ) || isnan( fwhmVal) || isinf(fwhmVal) || isinf(centerVal) ){
		return false;
	}

	SpecFit* gaussFit = new SpecFitGaussian( peakVal, centerVal, fwhmVal, index );
	curves.append( gaussFit );
	return true;
}



void SpecFitSettingsWidgetRadio::resolveOutputLogFile( ){
	if (outputLogPath.isEmpty()){
		//QString baseName = taskMonitor->getFileName();
		QString baseName = ui.curveComboBox->currentText();
		std::string filePath = viewer::options.temporaryPath( baseName.toStdString()+"LOG" );
		outputLogPath = filePath.c_str();
	}
}



void SpecFitSettingsWidgetRadio::cancelFit(){
	fitCancelled = true;
}

void SpecFitSettingsWidgetRadio::specLineFit(){
	*logger << LogOrigin("SpecFitOptical", "specLineFit");
	bool validMin = isValidChannelRangeValue( ui.minLineEdit->text(), "Start" );
	bool validMax = isValidChannelRangeValue( ui.maxLineEdit->text(), "End" );
	if ( validMin && validMax ){
		// convert input values to Float
		float startVal=ui.minLineEdit->text().toFloat();
		float endVal  =ui.maxLineEdit->text().toFloat();
		if ( endVal < startVal ){
			//Switch them around - the code expects the startVal
			//to be less than the endVal;
			float tempVal = startVal;
			startVal = endVal;
			endVal = tempVal;
		}

		//Determine what combination of Gauss & Polynomial fits we
		//are doing.
		int gaussCount = ui.gaussCountSpinBox->value();
		bool polyFit = false;
		if ( ui.polyFitCheckBox->isChecked() ){
			polyFit = true;
		}
		// make sure something should be fitted at all
		bool valid = isValidFitSpecification( gaussCount, polyFit);
		if ( valid ){
			int polyN = 0;
			if ( polyFit ){
				polyN = ui.polyOrderSpinBox->value();
			}
			doFit( startVal, endVal, gaussCount, polyFit, polyN );
		}
	}
	else {
		QString msg="Please specify a valid range.";
		if ( !validMin ){
			msg.append( "\nMinimum value "+ui.minLineEdit->text()+" was not valid.");
		}
		if ( !validMax ){
			msg.append( "\nMaximum value "+ui.maxLineEdit->text()+" was not valid.");
		}
		Util::showUserMessage( msg, this );
	}
}

void SpecFitSettingsWidgetRadio::setOutputLogFile(){
	string homedir = getenv("HOME");
	QFileDialog fd( this, tr("Specify File for Output Logging"),
			QString(homedir.c_str()), "Text Files(*.txt)");
	fd.setFileMode( QFileDialog::AnyFile );
	if ( fd.exec() ){
		QStringList fileNames = fd.selectedFiles();
		if ( fileNames.size() > 0 ){
			outputLogPath = fileNames[0];
		}
	}
}

void SpecFitSettingsWidgetRadio::viewOutputLogFile(){

	if ( !outputLogPath.isEmpty()){
		QFile outputLogFile(outputLogPath);
		if ( !outputLogFile.exists() ){
			QString msg("The output file did not exist.\n  Please check it has been specified correctly.");
			Util::showUserMessage ( msg, this);
		}
		else {
			//If possible add the viewer settings to the output log.
			QString viewerSettings = settingsToString();
			if ( !outputLogFile.open( QIODevice::Append | QIODevice::Text)){
				qDebug() << "Could not write to the output log file";
			}
			else {
				//Write the viewer settings to the log file
				QTextStream out(&outputLogFile );
				out << viewerSettings;
				out.flush();
				outputLogFile.close();
			}

			if ( !outputLogFile.open( QIODevice::ReadOnly) ){
				QString msg("Could not open output file for reading: "+outputLogPath+"\n  Please check it has proper permissions.");
				Util::showUserMessage( msg, this);
			}
			else {
				//Now read in the contents of the file to the viewer.
				QTextStream ts( &outputLogFile );
				QString logResults;
				while ( ! ts.atEnd() ){
					QString line = ts.readLine();
					line.append( "\n");
					logResults.append( line );
				}
				SpecFitLogDialog* dialog = new SpecFitLogDialog( this );
				dialog -> setLogResults( logResults );
				dialog -> exec();
			}
		}
	}
	else {
		QString msg("Please specify a file for output logging.");
		Util::showUserMessage( msg, this);
	}
}

void SpecFitSettingsWidgetRadio::clearEstimates(){
	int rowCount = ui.estimateTable->rowCount();
	int colCount = ui.estimateTable->columnCount();
	for ( int i = 0; i < rowCount; i++ ){
		for ( int j = 0; j < colCount; j++ ){
			if ( j != FIXED ){
				QTableWidgetItem* tableItem = ui.estimateTable->item(i,j);
				if ( tableItem != NULL ){
					tableItem->setText("");
				}
			}
			else {
				SpecFitSettingsFixedTableCell* cellWidget =
						dynamic_cast<SpecFitSettingsFixedTableCell*>(ui.estimateTable->item(i,j));
				if ( cellWidget != NULL ){
					cellWidget->clear();
				}
			}
		}
	}
	ui.estimateTable->resizeColumnToContents( FIXED );
}

void SpecFitSettingsWidgetRadio::pixelsChanged( int pixX, int pixY ){
	drawCurves( pixX, pixY);
}

void SpecFitSettingsWidgetRadio::showPlots(){
	plotDialog.show();
}

//Called when the Gauss Estimate dialog is shown.  We populate it with the
//current information.
void SpecFitSettingsWidgetRadio::specifyGaussianEstimates(){

	//Let the dialog know the x axis units we are using
	String axisUnitStr = getXAxisUnit();
	QString unitStr( axisUnitStr.c_str());
	gaussEstimateDialog.setSpecFitUnits( unitStr );
	gaussEstimateDialog.setDisplayYUnits( displayYUnits );

	//Get the yvalues from the curve so they are already in the
	//correct units.
	CanvasCurve curve = pixelCanvas->getCurve( ui.curveComboBox->currentText());
	Vector<float> yValues = curve.getYValues();

	gaussEstimateDialog.setCurveData( getXValues(), yValues );
	gaussEstimateDialog.setCurveColor( pixelCanvas->getCurveColor(0));

	//If there are any estimates specified, tell the dialog about them.
	int estimateCount = ui.estimateTable->rowCount();
	QList<SpecFitGaussian> estimates;
	for ( int i = 0; i < estimateCount; i++ ){
		QString peakStr;
		QString centerStr;
		QString fwhmStr;
		getEstimateStrings( i, peakStr, centerStr, fwhmStr );
		float peak = static_cast<float>(peakStr.toDouble());
		float center = static_cast<float>(centerStr.toDouble());
		float fwhm = static_cast<float>(fwhmStr.toDouble());

		estimates.append(SpecFitGaussian(peak,center,fwhm,i) );
		SpecFitSettingsFixedTableCell* cellWidget =
					dynamic_cast<SpecFitSettingsFixedTableCell*>(ui.estimateTable->cellWidget(i,FIXED));
		if ( cellWidget != NULL ){
			estimates[i].setPeakFixed( cellWidget->isPeakFixed());
			estimates[i].setCenterFixed( cellWidget->isCenterFixed());
			estimates[i].setFwhmFixed( cellWidget->isFWHMFixed());
		}
	}
	gaussEstimateDialog.setEstimates( estimates );
	gaussEstimateDialog.show();
}

void SpecFitSettingsWidgetRadio::gaussianEstimatesChanged(){
	int count = ui.gaussCountSpinBox->value();
	QString dialogUnits = gaussEstimateDialog.getUnits();
	QString xAxisUnit(this->getXAxisUnit().c_str());
	Converter* converter = Converter::getConverter( dialogUnits, xAxisUnit );

	//If our display y-values don't match those of the gaussian estimate dialog,
	//we need to convert.
	QString estimateDisplayYUnits = gaussEstimateDialog.getDisplayYUnits();
	QString fitCurveName = ui.curveComboBox->currentText();
	CanvasCurve curve = pixelCanvas->getCurve( fitCurveName );
	for ( int i = 0; i < count; i++ ){
		SpecFitGaussian estimate = gaussEstimateDialog.getEstimate( i );
		float peakVal = estimate.getPeak();
		float centerVal = estimate.getCenter();
		float fwhmVal = estimate.getFWHM();
		float fwhmPt = centerVal - fwhmVal;
		if ( xAxisUnit.length() > 0 ){
			centerVal = converter->convert( centerVal );
			fwhmPt = converter->convert( fwhmPt );
		}
		else {
			centerVal = converter->toPixel( centerVal );
			fwhmPt = converter->toPixel( fwhmPt );
		}
		fwhmVal = qAbs(centerVal - fwhmPt);

		//If our display y-values don't match those of the gaussian estimate dialog,
		//we need to convert.
		if ( displayYUnits != estimateDisplayYUnits ){
			peakVal = curve.convertValue( peakVal, estimateDisplayYUnits, displayYUnits );
		}

		setEstimateValue( i, PEAK, peakVal );
		setEstimateValue( i, CENTER, centerVal );
		setEstimateValue( i, FWHM, fwhmVal );
		bool fixedPeak = estimate.isPeakFixed();
		bool fixedCenter = estimate.isCenterFixed();
		bool fixedFwhm = estimate.isFwhmFixed();
		SpecFitSettingsFixedTableCell* cellWidget =
			dynamic_cast<SpecFitSettingsFixedTableCell*>(ui.estimateTable->cellWidget(i,FIXED));
		if ( cellWidget != NULL ){
			cellWidget->setFixedPeak( fixedPeak );
			cellWidget->setFixedCenter( fixedCenter );
			cellWidget->setFixedFwhm( fixedFwhm );
		}
	}
	delete converter;
}

void SpecFitSettingsWidgetRadio::getConversion( const String& unitStr, Bool& velocity, Bool& wavelength ) const {
		int msIndex = unitStr.find( "m/s");
		int mIndex = unitStr.find( "m");
		int angIndex = unitStr.find( "Ang");
		velocity = false;
		wavelength = false;
		if ( msIndex >= 0 ){
			velocity = true;
		}
		else if ( mIndex >= 0 || angIndex >= 0 ){
			wavelength = true;
		}
	}

void SpecFitSettingsWidgetRadio::setDisplayYUnits( const QString& units ){
	//If the display units change, we need to convert any initial Gaussian peak
	//values to the new units.
	if ( units != displayYUnits && displayYUnits.length() > 0 ){
		int estimateCount = ui.estimateTable->rowCount();
		QString fitCurveName = ui.curveComboBox->currentText();
		CanvasCurve curve = pixelCanvas->getCurve( fitCurveName );
		for ( int i = 0; i < estimateCount; i++ ){
			QTableWidgetItem* peakItem = ui.estimateTable->item(i, PEAK );
			if ( peakItem != NULL ){
				QString peakStr = peakItem->text();
				float peakVal = peakStr.toFloat();
				peakVal = curve.convertValue( peakVal, displayYUnits, units );
				setEstimateValue( i, PEAK, peakVal );
			}
		}
	}
	displayYUnits = units;
}

void SpecFitSettingsWidgetRadio::setImageYUnits( const QString& units ){
	imageYUnits = units;
}

void SpecFitSettingsWidgetRadio::setCurveName( const QString& curveName ){
	ui.curveComboBox->clear();
	ui.curveComboBox->addItem( curveName );
}

void SpecFitSettingsWidgetRadio::addCurveName( const QString& curveName ){
	ui.curveComboBox->addItem( curveName );
}

SpecFitSettingsWidgetRadio::~SpecFitSettingsWidgetRadio()
{
	reset();
}
}
