//# MakeMask.cc: Qt implementation of viewer mask maker window.
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
//# $Id$


#include <display/QtViewer/MakeMask.qo.h>
#include <display/RegionShapes/RegionShapes.h>
#include <display/QtViewer/QtDisplayData.qo.h>


#include <casa/Containers/Block.h>
#include <casa/Containers/RecordField.h>
#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCUnion.h>
#include <images/Regions/WCRegion.h>
#include <images/Regions/WCCompound.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/WCPolygon.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/Attribute.h>
#include <casa/IO/AipsIO.h>
#include <images/Images/PagedImage.h>
#include <tables/Tables/TableRecord.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <casa/Utilities/CountedPtr.h>


namespace casa {

	MakeMask::MakeMask(QtDisplayPanel* qdp) {

		qdp_ = qdp;

		regData = 0;

		activeShape = 0;

		timer = new QTimer();
		flash = false;

		cb = 1;
		zIndex = 0;
		pIndex = 0;
		zAxis = "Frequency";

		setWindowTitle("Region in File");
		QVBoxLayout *layout = new QVBoxLayout;

		setLayout(layout);
		setMinimumSize(210, 180);
		setFixedSize(320, 180);

		tGroup = new QGroupBox;
		QGridLayout *tLayout = new QGridLayout;
		tGroup->setLayout(tLayout);

		tGroup->setToolTip(
		    "  1. display the image;\n"
		    "  2. assign a mouse button to rectangle or polygon;\n"
		    "  3. draw a region by pressing/clicking and dragging ;\n"
		    "  4. size and shape by dragging the corner handle;\n"
		    "  5. position by dragging the inside;\n"
		    "  6. record the region by double clicking inside (while handles on);\n"
		    "  7. cancel the region by double clicking outside (while handles on);\n"
		    "  8. delete the region by double clicking inside (while handles off). "
		);

		load = new QPushButton("Load");
		tLayout->addWidget(load, 0, 0, 1, 3);
		load->setToolTip("Load region(s) from a previously saved region file.");
		connect(load, SIGNAL(clicked()), SLOT(loadRegionFromFile()));

		save = new QPushButton("Save");
		tLayout->addWidget(save, 0, 3, 1, 3);
		save->setToolTip("Save region(s) to a region file.");
		connect(save, SIGNAL(clicked()), SLOT(saveRegionToFile()));

		removeAll = new QPushButton("CleanUp");
		tLayout->addWidget(removeAll, 1, 0, 1, 2);
		removeAll->setToolTip("Remove all regions from display.");
		connect(removeAll, SIGNAL(clicked()), SLOT(deleteAll()));

		showHide = new QPushButton("Hide");
		tLayout->addWidget(showHide, 1, 2, 1, 2);
		showHide->setToolTip("Toggle region display on/off.");
		connect(showHide, SIGNAL(clicked()), SLOT(showHideAll()));

		color = new QComboBox;
		color->addItem("Cyan");
		color->addItem("Red");
		color->addItem("Blue");
		color->addItem("Yellow");
		color->addItem("White");
		color->addItem("Black");
		tLayout->addWidget(color, 1, 4, 1, 2);
		color->setToolTip("Change region display color.");
		connect(color, SIGNAL(currentIndexChanged(const QString&)),
		        SLOT(colorAll(const QString&)));

		tLayout->addWidget(new QLabel("channels"), 2, 0, 1, 2);
		chan = new QLineEdit;
		chan->setToolTip("Set the region extend channels, example: 1,5~10\n"
		                 "Leave blank for 'all channels'");
		tLayout->addWidget(chan, 2, 2, 1, 4);


		tLayout->addWidget(new QLabel("Stokes"), 3, 0, 1, 2);
		corr = new QLineEdit;
		corr->setToolTip("Set the region extend Stokes parameters, example: 0,2~3\n"
		                 "Leave blank for 'all Stokes'");
		tLayout->addWidget(corr, 3, 2, 1, 4);

		/*
		  eGroup = new QGroupBox("Edit");
		  QGridLayout *eLayout = new QGridLayout;
		  eGroup->setLayout(eLayout);

		  remove = new QPushButton("Delete");
		  eLayout->addWidget(remove, 0, 0, 1, 1);
		  connect(remove, SIGNAL(clicked()), SLOT(doIt()));

		  shape = new QComboBox;
		  shape->addItem("Square");
		  shape->addItem("Recangle");
		  shape->addItem("Circle");
		  shape->addItem("Ellipse");
		  shape->addItem("Polygon");
		  eLayout->addWidget(shape, 0, 1, 1, 1);
		  connect(shape, SIGNAL(currentIndexChanged(const QString&)),
		                 SLOT(reShape(const QString&)));

		  rotateL = new QPushButton("D");
		  eLayout->addWidget(rotateL, 1, 0, 1, 1);
		  connect(rotateL, SIGNAL(clicked()), SLOT(doIt()));

		  rotateR = new QPushButton("R");
		  eLayout->addWidget(rotateR, 1, 1, 1, 1);
		  connect(rotateR, SIGNAL(clicked()), SLOT(doIt()));

		  left = new QPushButton("Left");
		  eLayout->addWidget(left, 2, 0, 1, 1);
		  connect(left, SIGNAL(clicked()), SLOT(doIt()));

		  right = new QPushButton("Right");
		  eLayout->addWidget(right, 2, 1, 1, 1);
		  connect(right, SIGNAL(clicked()), SLOT(doIt()));

		  up = new QPushButton("Up");
		  eLayout->addWidget(up, 3, 0, 1, 1);
		  connect(up, SIGNAL(clicked()), SLOT(doIt()));

		  down = new QPushButton("Down");
		  eLayout->addWidget(down, 3, 1, 1, 1);
		  connect(down, SIGNAL(clicked()), SLOT(doIt()));

		  wider = new QPushButton("Wider");
		  eLayout->addWidget(wider, 4, 0, 1, 1);
		  connect(wider, SIGNAL(clicked()), SLOT(doIt()));

		  narrower = new QPushButton("Narrower");
		  eLayout->addWidget(narrower, 4, 1, 1, 1);
		  connect(narrower, SIGNAL(clicked()), SLOT(doIt()));

		  taller = new QPushButton("Taller");
		  eLayout->addWidget(taller, 5, 0, 1, 1);
		  connect(taller, SIGNAL(clicked()), SLOT(doIt()));

		  shorter = new QPushButton("Shorter");
		  eLayout->addWidget(shorter, 5, 1, 1, 1);
		  connect(shorter, SIGNAL(clicked()), SLOT(doIt()));

		  layout->addWidget(eGroup);
		*/
		layout->addWidget(tGroup);

		//from double click in a region
		connect(qdp_,
		        SIGNAL(mouseRegionReady(Record, WorldCanvasHolder*)),
		        SLOT(drawRegion(Record, WorldCanvasHolder*)));

		//also from double click a region
		connect(qdp_, SIGNAL(newRegion(String)),
		        SLOT(newRegion_(String)));
		connect(qdp_,  SIGNAL(registrationChange()),
		        SLOT(deleteAll()));
		connect(qdp_, SIGNAL(activate(Record)),
		        SLOT(activate(Record)) );
		connect(qdp_, SIGNAL(animatorChange()),
		        SLOT(zPlaneChanged()) );

	}

	void MakeMask::changeAxis(String xa, String ya, String za, std::vector<int> ha ) {
		//cout << "change axis=" << xa << " " << ya
		//     << " " << za << " " << ha << endl;
		int ccb = 0;
		zAxis = "";
		pIndex = 0;
		if ( ha.size() > 0 )
			pIndex = ha[0];
		if (za.contains("Stoke"))
			zAxis = "Stokes";
		if (za.contains("Frequency"))
			zAxis = "Frequency";

		if (xa.contains("Decl") && ya.contains("Right"))
			ccb = -1;
		if (xa.contains("Right") && ya.contains("Decl"))
			ccb = 1;
		if (xa.contains("atitu") && ya.contains("ongitu"))
			ccb = -1;
		if (xa.contains("ongitu") && ya.contains("atitu"))
			ccb = 1;

		if (zAxis != "Frequency" && zAxis != "Stokes")
			ccb = 0;

		cb = ccb;

		if (cb == 0) {
			chan->setText("");
			corr->setText("");
		} else {
			if (zAxis == "Stokes") {
				chan->setText(QString::number(pIndex));
				corr->setText(QString::number(zIndex));
			} else {
				chan->setText(QString::number(zIndex));
				corr->setText(QString::number(pIndex));
			}
		}

		reDraw();
	}
	void MakeMask::closeEvent(QCloseEvent* /*event*/) {
		//qDebug() << "closeEvent";
		emit hideRegionInFile();
	}

	void MakeMask::activate(Record rcd) {
		if (!(this->isVisible()))
			return;

		//cout << "activate " << rcd << endl;
		String tool = rcd.asString("tool");

		Vector<Double> wx(2);
		wx  = -1000;

		if (this->isVisible() && rcd.isDefined("world")) {
			Record world = rcd.asRecord("world");
			Vector<Double> wld = world.asArrayDouble("wld");
			Vector<String> units = world.asArrayString("units");

			/*List<QtDisplayData*> DDs = qdp_->registeredDDs();
			ListIter<QtDisplayData*> qdds(DDs);
			if (qdds.len() > 0) {
			   qdds.toEnd();
			   qdds--;
			   QtDisplayData* qdd = qdds.getRight();*/
			if (!qdp_->isEmptyRegistered()) {
				DisplayDataHolder::DisplayDataIterator iter = qdp_->endRegistered();
				iter--;
				QtDisplayData* qdd = (*iter);
				zIndex = qdd->dd()->activeZIndex();

				if ((qdd->imageInterface())) {
					CoordinateSystem csys=(qdd->imageInterface())->coordinates();
					//Int dirInd=csys.findCoordinate(Coordinate::DIRECTION);
					//MDirection::Types dirType=csys.directionCoordinate(dirInd)
					//                      .directionType(True);
					wx(0) = Quantity(wld(0), units(0)).getValue(RegionShape::UNIT);
					wx(1) = Quantity(wld(1), units(1)).getValue(RegionShape::UNIT);
				}
			}
		}
		//cout << "wx=" << wx << endl;
		if (wx(0) == -1000 && wx(0) == -1000)
			return;

		uInt nreg = unionRegions_p.nelements();
		for (uInt k = 0; k < nreg; ++k) {

			const ImageRegion* reg = unionRegions_p[k];

			if (reg == 0)
				continue;

			const WCRegion* wcreg = &(reg->asWCRegion());
			String cmt = wcreg->comment();
			//cout << "comment=" << cmt << endl;
			String chan = cmt.before("polExt:");
			chan = chan.after("chanExt:");
			String pola = cmt.after("polExt:");
			//cout << "chanExt=" << chan << endl;
			//cout << "polExt=" << pola << endl;

			if (!planeAllowed(chan, pola))
				continue;

			//cout << wcreg->type() << " " << tool << endl;
			if ((wcreg->type()) == "WCBox" && tool.contains("ectangle")) {
				TableRecord boxrec=wcreg->toRecord("");
				const RecordInterface& blcrec=boxrec.asRecord("blc");
				const RecordInterface& trcrec=boxrec.asRecord("trc");
				CoordinateSystem *coords;
				coords=CoordinateSystem::restore(boxrec,"coordinates");
				//cout << "coords rect " << coords->nCoordinates() << endl;
				Int dirInd=coords->findCoordinate(Coordinate::DIRECTION);
				//MDirection::Types dirType=coords->
				//    directionCoordinate(dirInd).directionType(True);
				//Assuming x, y axes are dirInd and dirInd+1
				Vector<Double> blc(2);
				Vector<Double> trc(2);
				QuantumHolder h;
				for (Int j=dirInd; j <= dirInd+1; ++j) {
					const RecordInterface& subRec0=blcrec.asRecord(j);
					const RecordInterface& subRec1=trcrec.asRecord(j);
					String error;
					if (!h.fromRecord(error, subRec0)) {
						throw (AipsError
						       (String("WCBox::fromRecord - could not recover blc because ") +
						        error));
					}
					blc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
					if (!h.fromRecord(error, subRec1)) {
						throw (AipsError
						       (String("WCBox::fromRecord - could not recover trc because ") +
						        error));
					}
					trc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
				}

				if (trc(0) <= wx(0) && wx(0) <= blc(0) &&
				        blc(1) <= wx(1) && wx(1) <= trc(1)) {
					//cout << "activate rect:" << blc << " " << trc << endl;
					//active = true;
					unionRegions_p.remove(k, True);
					break;
				}
			} else if((wcreg->type())== "WCPolygon" &&  tool.contains("olygon")) {
				TableRecord polyrec=wcreg->toRecord("");
				CoordinateSystem *coords;
				coords=CoordinateSystem::restore(polyrec,"coordinates");
				//cout << "coords polyg " << coords->nCoordinates() << endl;

				//Int dirInd=coords->findCoordinate(Coordinate::DIRECTION);
				//MDirection::Types dirType=coords->
				//        directionCoordinate(dirInd).directionType(True);
				Vector<Double> x;
				Vector<Double> y;
				const RecordInterface& subRecord0 = polyrec.asRecord("x");
				String error;
				QuantumHolder h;
				if (!h.fromRecord(error, subRecord0)) {
					throw (AipsError
					       (String("WCPolygon::fromRecord - could not recover X") +
					        " Quantum vector because " +error));
				}
				x = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);
				const RecordInterface& subRecord1 = polyrec.asRecord("y");
				if (!h.fromRecord(error, subRecord1)) {
					throw (AipsError
					       (String("WCPolygon::fromRecord - could not recover Y") +
					        " Quantum vector because " +error));
				}
				y = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);
				Double xd = -10000;
				Double yd = -10000;
				Double xc = -xd;
				Double yc = -yd;
				for (uInt j = 0; j < x.nelements(); j++) {
					xd = fmax(xd, x(j));
					yd = fmax(yd, y(j));
					xc = fmin(xc, x(j));
					yc = fmin(yc, y(j));
				}
				if (xc <= wx(0) && wx(0) <= xd &&
				        yc <= wx(1) && wx(1) <= yd) {
					//cout << "activate poly: " << x << " " << y << endl;
					unionRegions_p.remove(k, True);
					break;
				}

			}

		}
		reDraw();
	}

	void MakeMask::wcChanged(const String cType,
	                         const Vector<Double> vx, const Vector<Double> vy) {
		cout << "wcChanged:" << cType << vx << vy << endl;
		if (!isVisible())
			return;

	}

	void MakeMask::reShape(const QString& shape) {
		qDebug() << "reShape" << shape;


	}

	void MakeMask::loadRegionFromFile() {
		QString sName = QFileDialog::getOpenFileName(this,
		                tr("Open Region File"),
		                ".",
		                tr("Regions (*.rgn)"));
		//qDebug() << "loadRegion from" << sName;
		if (sName == "")
			return;

		String rName(sName.toStdString());
		ImageRegion* reg;
		TableRecord rec;
		try {
			//cout << "rName=" << rName << endl;
			AipsIO os(rName, ByteIO::Old);
			//os.open(rName);
			os >> rec;
			//os.close();
			//cout << "infile region record:\n" << rec << endl;
			reg = ImageRegion::fromRecord(rec, rName+".tbl");

			//WCUnion leUnion(unionRegions_p);
			//const WCRegion* newUnion = new WCUnion(reg->asWCRegion(), leUnion);
			//WCUnion* unfolded = unfoldCompositeRegionToSimpleUnion(newUnion);
			//cout << "unfolded:" << unfolded->toRecord("") << endl;
			//cout << "leUnion:" << leUnion.toRecord("") << endl;

			if (!reg)
				return;

			PtrBlock<const WCRegion* > outRegPtrs ;
			const WCRegion* rpt = &(reg->asWCRegion());
			String cmt = rpt->comment();
			//cout << "comment=" << cmt << endl;
			unfoldIntoSimpleRegionPtrs(outRegPtrs, rpt);

			if (outRegPtrs.nelements() < 1) {
				QMessageBox::warning(this, "FILEBOX", "The box file is empty.");
			}
			//cout << "number of boxes =" << outRegPtrs.nelements() << endl;
			for (uInt m = 0; m < outRegPtrs.nelements(); m++) {
				//if (m < 3) {
				//   cout << "record " << m << ":" << endl;
				//   cout << outRegPtrs[m]->toRecord("") << endl;
				//}
				uInt nreg=unionRegions_p.nelements();
				unionRegions_p.resize(nreg + 1, True);
				WCRegion* regM = const_cast<WCRegion*>(outRegPtrs[m]);
				regM->setComment(cmt);
				unionRegions_p[nreg] = new const ImageRegion(regM);
				//if (m < 3)
				//   cout << "ImageRegion=" << unionRegions_p[nreg]->toRecord("")
				//        << endl;
				//const_cast<WCRegion*>(outRegPtrs[m]));
			}
		} catch(...) {
			cout << "Failed to read region record" << endl;
		}

		if (cb == 0) {
			chan->setText("");
			corr->setText("");
		} else {
			if (zAxis == "Stokes") {
				chan->setText(QString::number(pIndex));
				corr->setText(QString::number(zIndex));
			} else {
				chan->setText(QString::number(zIndex));
				corr->setText(QString::number(pIndex));
			}
		}

		reDraw();

	}

	void MakeMask::saveRegionToFile() {
		//cout << "cb=" << cb << " zAxis=" << zAxis
		//     << " zIndex=" << zIndex << " pIndex=" << pIndex << endl;
		QString sName = QFileDialog::getSaveFileName(this,
		                tr("Save Region File"),
		                "./",
		                tr("Regions (*.rgn)"));
		//qDebug() << "saveRegion to" << sName;
		if (sName != "") {
			QString ext = sName.section('.', -1).toLower();
			//qDebug() << ext;
			if (ext != "rgn") {
				sName.append(".rgn");
			}
			//qDebug() << sName;

			String regname(sName.toStdString());

			if (unionRegions_p.nelements() < 1) {
				QMessageBox::warning(this, "FILEBOX", "There is no box to save.");
			} else {
				WCUnion leUnion(unionRegions_p);
				leUnion.setComment("chanExt:" +
				                   chan->text().toStdString() +
				                   "polExt:" +
				                   corr->text().toStdString());
				//cout << "chan=" << chan->text().toStdString()
				//     << " corr=" << corr->text().toStdString() << endl;
				ImageRegion* reg = new ImageRegion(leUnion);

				try {
					//AipsIO os(regname, ByteIO::NewNoReplace);
					AipsIO os(regname, ByteIO::New);
					os << reg->toRecord(regname+".tbl");
				} catch(...) {
					QMessageBox::warning(this, "FILEBOX",
					                     "Could not create the region file.\n"
					                     "Please check pathname and directory\n"
					                     "permissions.");
				}
			}
		}
		if (cb == 0) {
			chan->setText("");
			corr->setText("");
		} else {
			if (zAxis == "Stokes") {
				chan->setText(QString::number(pIndex));
				corr->setText(QString::number(zIndex));
			} else {
				chan->setText(QString::number(zIndex));
				corr->setText(QString::number(pIndex));
			}
		}
	}

	void MakeMask::showHelp() {

	}

	void MakeMask::addBox(RegionShape*) {
		qDebug() << "addBox";
	}

	void MakeMask::deleteBox(RegionShape*) {
		qDebug() << "deleteBox";
	}

	void MakeMask::deleteAll() {
		//qDebug() << "deleteAll";
		uInt nreg=unionRegions_p.nelements();
		for (uInt k=0; k< nreg; ++k) {
			if (unionRegions_p[k] !=0) {
				delete unionRegions_p[k];
			}
		}
		unionRegions_p.resize(0, True);
		reDraw();
	}

	void MakeMask::reDraw() {

		/*List<QtDisplayData*> DDs = qdp_->registeredDDs();
		ListIter<QtDisplayData*> qdds(DDs);
		if (qdds.len() == 0)
		   return ;

		qdds.toEnd();
		qdds--;
		QtDisplayData* qdd = qdds.getRight();*/
		if ( ! qdp_->isEmptyRegistered()) {
			DisplayDataHolder::DisplayDataIterator iter = qdp_->endRegistered();
			iter--;
			QtDisplayData* qdd = (*iter);
			qdp_->hold();
			qdp_->panelDisplay()->removeDisplayData(*regData);

			//qDebug() << "showHide=" << showHide->text();
			if (unionRegions_p.nelements() > 0 &&
			        showHide->text() == "Hide" && cb != 0) {
				WCUnion leUnion(unionRegions_p);
				ImageRegion* reg = new ImageRegion(leUnion);
				//cout << "to be draw ImageRegion:\n"
				//     << reg->toRecord("arbitrary") << endl;
				regData = regionToShape(qdd, reg);

				qdp_->panelDisplay()->addDisplayData(*regData);
			}
			qdp_->release();
		}
	}

	RSComposite* MakeMask::regionToShape(
	    QtDisplayData* qdd, const ImageRegion* reg) {

		if (reg && reg->isWCRegion()) {

			const WCRegion* wcreg = reg->asWCRegionPtr();
			CoordinateSystem csys;
			csys = (qdd->imageInterface())->coordinates();
			//cout << "before showreg csys" << endl;
			Int dirInd =
			    csys.findCoordinate(Coordinate::DIRECTION);
			MDirection::Types dirType = csys.
			                            directionCoordinate(dirInd).directionType(True);
			RSComposite *theShapes= new RSComposite(dirType);
			addRegionsToShape(theShapes, wcreg);
			theShapes->setLineColor(color->currentText().toStdString());
			theShapes->setLineWidth(1);

			return theShapes;
		} else {
			return 0;
		}
	}

	void MakeMask::addRegionsToShape(RSComposite*& theShapes,
	                                 const WCRegion*& wcreg) {
		//cout << "WCRegion->comment="
		//     << wcreg->comment() << endl;
		String rest = wcreg->comment();
		String chan = rest.before("polExt:");
		chan = chan.after("chanExt:");
		String pola = rest.after("polExt:");
		//cout << "chanExt=" << chan << endl;
		//cout << "polExt=" << pola << endl;
		//  cout << "aRegion\n\n" << endl;
		//  cout << wcreg->toRecord("") << endl;

		if (!planeAllowed(chan, pola))
			return;

		if((wcreg->type()) == "WCBox") {
			TableRecord boxrec=wcreg->toRecord("");
			const RecordInterface& blcrec=boxrec.asRecord("blc");
			const RecordInterface& trcrec=boxrec.asRecord("trc");
			CoordinateSystem *coords;
			coords=CoordinateSystem::restore(boxrec,"coordinates");
			//cout << "coords rect " << coords->nCoordinates() << endl;
			Int dirInd=coords->findCoordinate(Coordinate::DIRECTION);
			MDirection::Types dirType=coords->
			                          directionCoordinate(dirInd).directionType(True);
			//Assuming x, y axes are dirInd and dirInd+1
			Vector<Double> blc(2);
			Vector<Double> trc(2);
			QuantumHolder h;
			for (Int j=dirInd; j <= dirInd+1; ++j) {
				const RecordInterface& subRec0=blcrec.asRecord(j);
				const RecordInterface& subRec1=trcrec.asRecord(j);
				String error;
				if (!h.fromRecord(error, subRec0)) {
					throw (AipsError
					       ("WCBox::fromRecord - could not recover blc because "+error));
				}

				blc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
				if (!h.fromRecord(error, subRec1)) {
					throw (AipsError
					       ("WCBox::fromRecord - could not recover trc because "+error));
				}
				trc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
			}

			Vector<Int> chanRange(2);
			Int spcInd=coords->findCoordinate(Coordinate::SPECTRAL);
			//cout << "speInd=" << spcInd << endl;
			if (blcrec.size() <= (uInt)spcInd || trcrec.size() <= (uInt)spcInd) {
				//set spectral range -1, -1 to indicate all chans
				chanRange(0) = -1;
				chanRange(1) = -1;
			} else {
				const RecordInterface& specRec0=blcrec.asRecord(spcInd);
				const RecordInterface& specRec1=trcrec.asRecord(spcInd);
				//cout << "spec0=" << specRec0 << endl;
				//cout << "spec1=" << specRec1 << endl;

				SpectralCoordinate spectralCoord =
				    coords->spectralCoordinate(spcInd);

				String error;
				Vector<String> units(1);
				units = "Hz";
				spectralCoord.setWorldAxisUnits(units);
				Vector<Double> spectralWorld(1);
				Vector<Double> spectralPixel(1);

				if (!h.fromRecord(error, specRec0)) {
					throw (AipsError
					       ("WCBox::fromRecord - could not recover trc because "+error));
				}
				if (specRec0.asString("unit") == "pix") {
					chanRange(0) = (Int)h.asQuantumDouble().getValue("pix");
					//cout << "pix_val_1=" << chanRange(0) << endl;
				} else {
					Double hz1 = h.asQuantumDouble().getValue("Hz");
					spectralWorld(0) = hz1;
					spectralCoord.toPixel(spectralWorld, spectralPixel);
					chanRange(0)  = (Int)spectralPixel(0);
					//cout << "hz_val_1=" << chanRange(0) << endl;
				}

				if (!h.fromRecord(error, specRec1)) {
					throw (AipsError
					       ("WCBox::fromRecord - could not recover trc because "+error));
				}
				if (specRec1.asString("unit") == "pix") {
					chanRange(1) = (Int)h.asQuantumDouble().getValue("pix");
					//cout << "pix_val_2=" << chanRange(1) << endl;
				} else {
					Double hz2 = h.asQuantumDouble().getValue("Hz");
					spectralWorld(0) = hz2;
					spectralCoord.toPixel(spectralWorld, spectralPixel);
					chanRange(1)  = (Int)spectralPixel(0);
					//cout << "hz_val_2=" << chanRange(1) << endl;
				}
			}

			QString chans = "";
			if (chanRange(0) > 0 && chanRange(1) > 0) {
				chanRange(0) = chanRange(0) - 1;
				chanRange(1) = chanRange(1) - 1;
			}
			if (chanRange(0) > -1 && chanRange(1) > -1) {
				chans = QString::number(chanRange(0)) +
				        QString("~") +
				        QString::number(chanRange(1));
			}
			//qDebug() << chans;
			if (!planeAllowed(chans.toStdString(), ""))
				return;

			double xw = fabs(trc(0)-blc(0));
			double xh = fabs(trc(1)-blc(1));
			if (xw <= 0 || xh <= 0)
				return;
			RSRectangle *rect = (cb == -1) ?
			                    new RSRectangle(
			                        (blc(1)+trc(1))/2.0,(blc(0)+trc(0))/2.0, xh, xw, dirType) :
			                    new RSRectangle(
			                        (blc(0)+trc(0))/2.0,(blc(1)+trc(1))/2.0, xw, xh, dirType);

			rect->setLineColor(color->currentText().toStdString());
			theShapes->addShape(rect);
			//cout << "rect=" << rect->toRecord() << endl;
		} else if((wcreg->type())== "WCPolygon") {
			TableRecord polyrec=wcreg->toRecord("");
			CoordinateSystem *coords;
			coords=CoordinateSystem::restore(polyrec,"coordinates");
			//cout << "coords polyg " << coords->nCoordinates() << endl;

			Int dirInd=coords->findCoordinate(Coordinate::DIRECTION);
			MDirection::Types dirType=coords->
			                          directionCoordinate(dirInd).directionType(True);
			Vector<Double> x;
			Vector<Double> y;
			const RecordInterface& subRecord0 = polyrec.asRecord("x");
			String error;
			QuantumHolder h;
			if (!h.fromRecord(error, subRecord0)) {
				throw (AipsError
				       ("WCPolygon::fromRecord - could not recover X Quantum vector because "
				        +error));
			}
			x = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);
			const RecordInterface& subRecord1 = polyrec.asRecord("y");
			if (!h.fromRecord(error, subRecord1)) {
				throw (AipsError
				       ("WCPolygon::fromRecord - could not recover Y Quantum vector because "
				        +error));
			}
			y = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);
			/*
			Int spcInd=coords->findCoordinate(Coordinate::SPECTRAL);
			const RecordInterface& specRec0=blcrec.asRecord(spcInd);
			const RecordInterface& specRec1=trcrec.asRecord(spcInd);
			//cout << "spec0=" << specRec0 << endl;
			//cout << "spec1=" << specRec1 << endl;
			SpectralCoordinate spectralCoord =
			     coords->spectralCoordinate(spcInd);

			String error;
			Vector<String> units(1); units = "Hz";
			spectralCoord.setWorldAxisUnits(units);
			Vector<Double> spectralWorld(1);
			Vector<Double> spectralPixel(1);
			Vector<Int> chanRange(2);

			if (!h.fromRecord(error, specRec0)) {
			     throw (AipsError
			      ("WCBox::fromRecord - could not recover trc because "+error));
			}
			Double hz1 = h.asQuantumDouble().getValue("Hz");
			spectralWorld(0) = hz1;
			spectralCoord.toPixel(spectralWorld, spectralPixel);
			chanRange(0)  = (Int)spectralPixel(0);

			if (!h.fromRecord(error, specRec1)) {
			     throw (AipsError
			      ("WCBox::fromRecord - could not recover trc because "+error));
			}
			Double hz2 = h.asQuantumDouble().getValue("Hz");
			spectralWorld(0) = hz2;
			spectralCoord.toPixel(spectralWorld, spectralPixel);
			chanRange(1)  = (Int)spectralPixel(0);

			QString chans = QString::number(chanRange(0)) +
			                QString("~") +
			                QString::number(chanRange(1));
			//qDebug() << chans;
			if (!planeAllowed(chans.toStdString(), ""))
			   return;
			*/

			RSPolygon *poly= (cb == -1) ?
			                 new RSPolygon(y,x,dirType) : new RSPolygon(x,y,dirType);
			poly->setLineColor(color->currentText().toStdString());
			theShapes->addShape(poly);
			//cout << "poly=" << poly->toRecord() << endl;
		} else if((wcreg->type()) == "WCUnion" ||(wcreg->type()) == "WCIntersection") {
			PtrBlock<const WCRegion*> regPtrs=
			    (static_cast<const WCCompound* >(wcreg))->regions();
			//cout << "number of wcregions " << regPtrs.nelements() << endl;
			for (uInt j=0; j < regPtrs.nelements(); ++j) {
				addRegionsToShape(theShapes, regPtrs[j]);
			}

		}

	}

	void MakeMask::showHideAll() {
		//qDebug() << "showHide";
		if (showHide->text() == "Hide") {
			showHide->setText("Show");
		} else {
			showHide->setText("Hide");
		}
		reDraw();
	}

	void MakeMask::colorAll(const QString& /*clr*/) {
		//qDebug() << "colorAll" << clr;
		reDraw();
	}

	void MakeMask::rotateBox(int /*cb*/) {
	}

	void MakeMask::newRegion_(String /*imgFilename*/) {
		//cout << "newRegion_" << imgFilename << endl;

		if (this->isVisible()) {
			if (qdp_->hasRegion()) {
				uInt regNo=unionRegions_p.nelements();
				unionRegions_p.resize(regNo+1);
				unionRegions_p[regNo]=new const
				ImageRegion(qdp_->lastRegion());
				//cout << "regNo=" << regNo << " WCRegion="
				//     << unionRegions_p[regNo]->isWCRegion()
				//     << "   LCRegion="
				//     << unionRegions_p[regNo]->isLCRegion()
				//     << endl;
			}
		}

		reDraw();
	}

	void MakeMask::drawRegion(Record /*mousereg*/,
	                          WorldCanvasHolder * /*wch*/) {
	}

	WCUnion* MakeMask::unfoldCompositeRegionToSimpleUnion(const WCRegion*& wcreg) {
		PtrBlock<const WCRegion* > outRegPtrs ;
		unfoldIntoSimpleRegionPtrs(outRegPtrs, wcreg);
		WCUnion* outputUnion = new WCUnion(True, outRegPtrs);
		return outputUnion;
	}

	void MakeMask::unfoldIntoSimpleRegionPtrs(PtrBlock<const WCRegion*>& outRegPtrs,
	        const WCRegion*& wcreg) {
		if ((wcreg->type()) == "WCBox") {
			uInt nreg=outRegPtrs.nelements();
			outRegPtrs.resize(nreg+1);
			outRegPtrs[nreg]=new WCBox(static_cast<const WCBox & >(*wcreg));
		} else if((wcreg->type()) == "WCPolygon") {
			uInt nreg=outRegPtrs.nelements();
			outRegPtrs.resize(nreg+1);
			outRegPtrs[nreg]=new WCPolygon(static_cast<const WCPolygon & >(*wcreg));
		} else if((wcreg->type()) == "WCUnion" ||
		          (wcreg->type()) == "WCIntersection" ) {
			PtrBlock<const WCRegion*> regPtrs =
			    (static_cast<const WCCompound* >(wcreg))->regions();
			//cout << "number of wcregions " << regPtrs.nelements() << endl;
			for (uInt j=0; j < regPtrs.nelements(); ++j) {
				unfoldIntoSimpleRegionPtrs(outRegPtrs, regPtrs[j]);
			}
		}
	}

	bool MakeMask::planeAllowed(String xa, String ya) {
		xa.gsub(" ", "");
		ya.gsub(" ", "");

		bool allowc = false;
		bool allowp = false;
		//cout << "xa=" << xa << " ya=" << ya << endl;
		if (xa.length() == 0)
			allowc = true;
		if (ya.length() == 0)
			allowp = true;
		if (allowc && allowp)
			return true;

		int idx = zIndex;
		int ipx = pIndex;
		if (zAxis == "Stokes") {
			ipx = zIndex;
			idx = pIndex;
		}
		//cout << "idx=" << idx << " ipx=" << ipx << endl;
		String w[10];
		if (!allowc) {
			Int nw = split(xa, w, 10, ',');
			for (Int k = 0; k < nw; k++) {
				String x[2];
				Int nx = split(w[k], x, 2, '~');
				if (nx == 2 &&
				        idx >= atoi(x[0].c_str()) &&
				        idx <= atoi(x[1].c_str())) {
					allowc = true;
					break;
				}
				if (nx == 1 && atoi(x[0].c_str()) == idx) {
					allowc = true;
					break;
				}
			}
		}
		if (!allowp) {
			Int nw = split(ya, w, 10, ',');
			for (Int k = 0; k < nw; k++) {
				String x[2];
				Int nx = split(w[k], x, 2, '~');
				if (nx == 2 &&
				        ipx >= atoi(x[0].c_str()) &&
				        ipx <= atoi(x[1].c_str())) {
					allowp = true;
					break;
				}
				if (nx == 1 && atoi(x[0].c_str()) == ipx) {
					allowp = true;
					break;
				}
			}
		}
		//cout << "allowp=" << allowp << " allowc=" << allowc << endl;
		return (allowp && allowc);
	}

	void MakeMask::zPlaneChanged() {
		/*List<QtDisplayData*> DDs = qdp_->registeredDDs();
		ListIter<QtDisplayData*> qdds(DDs);
		if (qdds.len() > 0) {
		   qdds.toEnd();
		   qdds--;
		   QtDisplayData* qdd = qdds.getRight();*/
		if ( !qdp_->isEmptyRegistered()) {
			DisplayDataHolder::DisplayDataIterator iter = qdp_->endRegistered();
			iter--;
			QtDisplayData* qdd = (*iter);
			//cout << "img=" << qdd->imageInterface() << endl;
			if (qdd->imageInterface()==0)
				return;
			zIndex = qdd->dd()->activeZIndex();
		}
		if (cb == 0) {
			chan->setText("");
			corr->setText("");
		} else {
			if (zAxis == "Stokes") {
				chan->setText(QString::number(pIndex));
				corr->setText(QString::number(zIndex));
			} else {
				chan->setText(QString::number(zIndex));
				corr->setText(QString::number(pIndex));
			}
		}
		reDraw();
	}

	void MakeMask::pPlaneChanged() {
		//List<QtDisplayData*> DDs = qdp_->registeredDDs();
		//ListIter<QtDisplayData*> qdds(DDs);
		//if (qdds.len() > 0) {
		//   qdds.toEnd();
		//   qdds--;
		//   QtDisplayData* qdd = qdds.getRight();
		//zIndex = qdd->dd()->activeZIndex();
		//  pIndex = 0;
		//}
		cout << "pchanged" << endl;
		pIndex = 0;
		reDraw();
	}

	void MakeMask::doIt() {
		QPushButton* action = dynamic_cast<QPushButton*>(sender());
		if (action == 0) {
			return;
		}
		qDebug() << action->text() << "clicked";

	}


}
