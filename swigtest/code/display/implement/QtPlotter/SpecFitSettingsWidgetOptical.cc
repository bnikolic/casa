#include "SpecFitSettingsWidgetOptical.qo.h"
#include <imageanalysis/ImageAnalysis/SpectralFitter.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <QWidget>
#include <QtGlobal>
#include <QDebug>
namespace casa {

SpecFitSettingsWidgetOptical::SpecFitSettingsWidgetOptical(QWidget *parent)
    : QWidget(parent), fitter( NULL )
{
	ui.setupUi(this);
	QValidator *validator = new QDoubleValidator(-1.0e+32, 1.0e+32,10,this);
	ui.startValueFit->setValidator(validator);
	ui.startValueFit->setMaximumWidth(100);
	ui.endValueFit->setValidator(validator);
	ui.endValueFit->setMaximumWidth(100);
	ui.fromLabelFit->setMinimumWidth(40);
	ui.fromLabelFit->setMargin(3);
	ui.fromLabelFit->setAlignment((Qt::Alignment)(Qt::AlignRight|Qt::AlignVCenter));
	ui.toLabelFit->setMinimumWidth(30);
	ui.toLabelFit->setMargin(3);
	ui.toLabelFit->setAlignment((Qt::Alignment)(Qt::AlignRight|Qt::AlignVCenter));

    connect( ui.fit, SIGNAL(clicked()), this, SLOT(specLineFit()));
	connect(ui.clean, SIGNAL(clicked()), this, SLOT(clean()));

	reset();

}

void SpecFitSettingsWidgetOptical::clear(){
	ui.startValueFit->setText("");
	ui.endValueFit->setText("");
}

void SpecFitSettingsWidgetOptical::reset(){
	if ( fitter != NULL ){
		delete fitter;
		fitter = NULL;
	}
	try{
		fitter    = new SpectralFitter();
	}
	catch (AipsError x){
		String message = "Error when starting the profiler:\n";
		logWarning(message);
	}
}

void SpecFitSettingsWidgetOptical::specLineFit(){
	*logger << LogOrigin("SpecFitOptical", "specLineFit");

	// get the values
	QString startStr = ui.startValueFit->text();
	QString endStr   = ui.endValueFit->text();
	String  msg;

	// make sure the input is reasonable

	if (startStr.isEmpty()){
		msg = String("No start value specified!");
		logWarning(msg);
		postStatus(msg);
		return;
	}
	if (endStr.isEmpty()){
		msg = String("No end value specified!");
		logWarning( msg );
		postStatus(msg);
		return;
	}

	int pos=0;
	if (ui.startValueFit->validator()->validate(startStr, pos) != QValidator::Acceptable){
		String startString(startStr.toStdString());
		msg = String("Start value not correct: ") + startString;
		logWarning( msg );
		postStatus(msg);
		return;
	}
	if (ui.endValueFit->validator()->validate(endStr, pos) != QValidator::Acceptable){
		String endString(endStr.toStdString());
		msg = String("Start value not correct: ") + endString;
		logWarning(msg);
		postStatus(msg);
		return;
	}

	// convert input values to Float
	Float startVal=(Float)startStr.toFloat();
	Float endVal  =(Float)endStr.toFloat();

	// set the fitting modes
	Bool doFitGauss(False);
	Bool doFitPoly(False);
	if (ui.fitGauss->currentText()==QString("gauss"))
		doFitGauss=True;

	Int polyN = 0;
	if (ui.fitPolyN->currentText()==QString("poly 0")){
		doFitPoly=True;
		polyN=0;
	}
	else if (ui.fitPolyN->currentText()==QString("poly 1")){
		doFitPoly=True;
		polyN=1;
	}

	// make sure something should be fitted at all
	if (!doFitGauss && !doFitPoly){
		msg = String("There is nothing to fit!");
		logWarning( msg );
		postStatus( msg );
		return;
	}

	// do the fit
	//Bool ok = fitter->fit(z_xval, z_yval, z_eval, startVal, endVal, doFitGauss, doFitPoly, polyN, msg);
	Vector<Float> z_xval = getXValues();
	Vector<Float> z_yval = getYValues();
	Vector<Float> z_eval = getZValues();
	if (!fitter->fit(z_xval, z_yval, z_eval, startVal, endVal, doFitGauss, doFitPoly, polyN, msg)){
		msg = String("Data could not be fitted!");
		postStatus(msg);
	}
	else{
		String xaxisUnit = getXAxisUnit();
		QString yUnit = getYUnit();
		QString yUnitPrefix = getYUnitPrefix();
		if (fitter->getStatus() == SpectralFitter::SUCCESS){
			// get the fit values
			Vector<Float> z_xfit, z_yfit;
			fitter->getFit(z_xval, z_xfit, z_yfit);
			// report problems
			if (z_yfit.size()<1){
				msg = String("There exist no fit values!");
				logWarning( msg );
				postStatus(msg);
				return;
			}

			// overplot the fit values
			QString fileName = getFileName();
			QString fitName = fileName + "FIT" + startStr + "-" + endStr + QString(xaxisUnit.c_str());
			pixelCanvas->addPolyLine(z_xfit, z_yfit, fitName);
		}
		postStatus((fitter->report(*logger, xaxisUnit, String(yUnit.toLatin1().data()), String(yUnitPrefix.toLatin1().data()))).c_str());
	}

}

void SpecFitSettingsWidgetOptical::setUnits( QString units ){
	ui.fitUnits->setText( units );
}

void SpecFitSettingsWidgetOptical::clean(){
	plotMainCurve();
}

void SpecFitSettingsWidgetOptical::setRange(float xmin, float xmax ){
	if (xmax < xmin){
		ui.startValueFit->clear();
		ui.endValueFit->clear();
	}
	else {
		QString startStr;
		QString endStr;
		startStr.setNum(xmin);
		endStr.setNum(xmax);
		ui.startValueFit->setText(startStr);
		ui.endValueFit->setText(endStr);
	}
}


SpecFitSettingsWidgetOptical::~SpecFitSettingsWidgetOptical()
{
	delete fitter;
}
}