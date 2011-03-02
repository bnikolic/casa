
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * File CalPointingModelRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalPointingModelRow.h>
#include <CalPointingModelTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalPointingModelRow;
using asdm::CalPointingModelTable;

using asdm::CalDataTable;
using asdm::CalDataRow;

using asdm::CalReductionTable;
using asdm::CalReductionRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	CalPointingModelRow::~CalPointingModelRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalPointingModelTable &CalPointingModelRow::getTable() const {
		return table;
	}

	bool CalPointingModelRow::isAdded() const {
		return hasBeenAdded;
	}	

	void CalPointingModelRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPointingModelRowIDL struct.
	 */
	CalPointingModelRowIDL *CalPointingModelRow::toIDL() const {
		CalPointingModelRowIDL *x = new CalPointingModelRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->antennaMake = antennaMake;
 				
 			
		
	

	
  		
		
		
			
				
		x->pointingModelMode = pointingModelMode;
 				
 			
		
	

	
  		
		
		
			
				
		x->polarizationType = polarizationType;
 				
 			
		
	

	
  		
		
		
			
				
		x->numCoeff = numCoeff;
 				
 			
		
	

	
  		
		
		
			
		x->coeffName.length(coeffName.size());
		for (unsigned int i = 0; i < coeffName.size(); ++i) {
			
				
			x->coeffName[i] = CORBA::string_dup(coeffName.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffVal.length(coeffVal.size());
		for (unsigned int i = 0; i < coeffVal.size(); ++i) {
			
				
			x->coeffVal[i] = coeffVal.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffError.length(coeffError.size());
		for (unsigned int i = 0; i < coeffError.size(); ++i) {
			
				
			x->coeffError[i] = coeffError.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffFixed.length(coeffFixed.size());
		for (unsigned int i = 0; i < coeffFixed.size(); ++i) {
			
				
			x->coeffFixed[i] = coeffFixed.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->azimuthRMS = azimuthRMS.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->elevationRms = elevationRms.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->skyRMS = skyRMS.toIDLAngle();
			
		
	

	
  		
		
		
			
				
		x->reducedChiSquared = reducedChiSquared;
 				
 			
		
	

	
  		
		
		x->numObsExists = numObsExists;
		
		
			
				
		x->numObs = numObs;
 				
 			
		
	

	
  		
		
		x->coeffFormulaExists = coeffFormulaExists;
		
		
			
		x->coeffFormula.length(coeffFormula.size());
		for (unsigned int i = 0; i < coeffFormula.size(); ++i) {
			
				
			x->coeffFormula[i] = CORBA::string_dup(coeffFormula.at(i).c_str());
				
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPointingModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalPointingModelRow::setFromIDL (CalPointingModelRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
		
		
			
		setPointingModelMode(x.pointingModelMode);
  			
 		
		
	

	
		
		
			
		setPolarizationType(x.polarizationType);
  			
 		
		
	

	
		
		
			
		setNumCoeff(x.numCoeff);
  			
 		
		
	

	
		
		
			
		coeffName .clear();
		for (unsigned int i = 0; i <x.coeffName.length(); ++i) {
			
			coeffName.push_back(string (x.coeffName[i]));
			
		}
			
  		
		
	

	
		
		
			
		coeffVal .clear();
		for (unsigned int i = 0; i <x.coeffVal.length(); ++i) {
			
			coeffVal.push_back(x.coeffVal[i]);
  			
		}
			
  		
		
	

	
		
		
			
		coeffError .clear();
		for (unsigned int i = 0; i <x.coeffError.length(); ++i) {
			
			coeffError.push_back(x.coeffError[i]);
  			
		}
			
  		
		
	

	
		
		
			
		coeffFixed .clear();
		for (unsigned int i = 0; i <x.coeffFixed.length(); ++i) {
			
			coeffFixed.push_back(x.coeffFixed[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setAzimuthRMS(Angle (x.azimuthRMS));
			
 		
		
	

	
		
		
			
		setElevationRms(Angle (x.elevationRms));
			
 		
		
	

	
		
		
			
		setSkyRMS(Angle (x.skyRMS));
			
 		
		
	

	
		
		
			
		setReducedChiSquared(x.reducedChiSquared);
  			
 		
		
	

	
		
		numObsExists = x.numObsExists;
		if (x.numObsExists) {
		
		
			
		setNumObs(x.numObs);
  			
 		
		
		}
		
	

	
		
		coeffFormulaExists = x.coeffFormulaExists;
		if (x.coeffFormulaExists) {
		
		
			
		coeffFormula .clear();
		for (unsigned int i = 0; i <x.coeffFormula.length(); ++i) {
			
			coeffFormula.push_back(string (x.coeffFormula[i]));
			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPointingModel");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalPointingModelRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaMake", antennaMake));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("pointingModelMode", pointingModelMode));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationType", polarizationType));
		
		
	

  	
 		
		
		Parser::toXML(numCoeff, "numCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffName, "coeffName", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffVal, "coeffVal", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffError, "coeffError", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffFixed, "coeffFixed", buf);
		
		
	

  	
 		
		
		Parser::toXML(azimuthRMS, "azimuthRMS", buf);
		
		
	

  	
 		
		
		Parser::toXML(elevationRms, "elevationRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(skyRMS, "skyRMS", buf);
		
		
	

  	
 		
		
		Parser::toXML(reducedChiSquared, "reducedChiSquared", buf);
		
		
	

  	
 		
		if (numObsExists) {
		
		
		Parser::toXML(numObs, "numObs", buf);
		
		
		}
		
	

  	
 		
		if (coeffFormulaExists) {
		
		
		Parser::toXML(coeffFormula, "coeffFormula", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(calDataId, "calDataId", buf);
		
		
	

  	
 		
		
		Parser::toXML(calReductionId, "calReductionId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalPointingModelRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalPointingModel",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalPointingModel",rowDoc);
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalPointingModel",rowDoc));
			
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","CalPointingModel",rowDoc);
		
		
		
	

	
		
		
		
		pointingModelMode = EnumerationParser::getPointingModelMode("pointingModelMode","CalPointingModel",rowDoc);
		
		
		
	

	
		
		
		
		polarizationType = EnumerationParser::getPolarizationType("polarizationType","CalPointingModel",rowDoc);
		
		
		
	

	
  		
			
	  	setNumCoeff(Parser::getInteger("numCoeff","CalPointingModel",rowDoc));
			
		
	

	
  		
			
					
	  	setCoeffName(Parser::get1DString("coeffName","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffVal(Parser::get1DFloat("coeffVal","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffError(Parser::get1DFloat("coeffError","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffFixed(Parser::get1DBoolean("coeffFixed","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setAzimuthRMS(Parser::getAngle("azimuthRMS","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setElevationRms(Parser::getAngle("elevationRms","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setSkyRMS(Parser::getAngle("skyRMS","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setReducedChiSquared(Parser::getDouble("reducedChiSquared","CalPointingModel",rowDoc));
			
		
	

	
  		
        if (row.isStr("<numObs>")) {
			
	  		setNumObs(Parser::getInteger("numObs","CalPointingModel",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<coeffFormula>")) {
			
								
	  		setCoeffFormula(Parser::get1DString("coeffFormula","CalPointingModel",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPointingModel");
		}
	}
	
	void CalPointingModelRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeString(antennaName);
				
		
	

	
	
		
					
			eoss.writeInt(receiverBand);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
					
			eoss.writeInt(antennaMake);
				
		
	

	
	
		
					
			eoss.writeInt(pointingModelMode);
				
		
	

	
	
		
					
			eoss.writeInt(polarizationType);
				
		
	

	
	
		
						
			eoss.writeInt(numCoeff);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) coeffName.size());
		for (unsigned int i = 0; i < coeffName.size(); i++)
				
			eoss.writeString(coeffName.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) coeffVal.size());
		for (unsigned int i = 0; i < coeffVal.size(); i++)
				
			eoss.writeFloat(coeffVal.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) coeffError.size());
		for (unsigned int i = 0; i < coeffError.size(); i++)
				
			eoss.writeFloat(coeffError.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) coeffFixed.size());
		for (unsigned int i = 0; i < coeffFixed.size(); i++)
				
			eoss.writeBoolean(coeffFixed.at(i));
				
				
						
		
	

	
	
		
	azimuthRMS.toBin(eoss);
		
	

	
	
		
	elevationRms.toBin(eoss);
		
	

	
	
		
	skyRMS.toBin(eoss);
		
	

	
	
		
						
			eoss.writeDouble(reducedChiSquared);
				
		
	


	
	
	eoss.writeBoolean(numObsExists);
	if (numObsExists) {
	
	
	
		
						
			eoss.writeInt(numObs);
				
		
	

	}

	eoss.writeBoolean(coeffFormulaExists);
	if (coeffFormulaExists) {
	
	
	
		
		
			
		eoss.writeInt((int) coeffFormula.size());
		for (unsigned int i = 0; i < coeffFormula.size(); i++)
				
			eoss.writeString(coeffFormula.at(i));
				
				
						
		
	

	}

	}
	
void CalPointingModelRow::antennaNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaName =  eiss.readString();
			
		
	
	
}
void CalPointingModelRow::receiverBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		receiverBand = CReceiverBand::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingModelRow::calDataIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calDataId =  Tag::fromBin(eiss);
		
	
	
}
void CalPointingModelRow::calReductionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calReductionId =  Tag::fromBin(eiss);
		
	
	
}
void CalPointingModelRow::startValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalPointingModelRow::endValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalPointingModelRow::antennaMakeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaMake = CAntennaMake::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingModelRow::pointingModelModeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		pointingModelMode = CPointingModelMode::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingModelRow::polarizationTypeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		polarizationType = CPolarizationType::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingModelRow::numCoeffFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numCoeff =  eiss.readInt();
			
		
	
	
}
void CalPointingModelRow::coeffNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		coeffName.clear();
		
		unsigned int coeffNameDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < coeffNameDim1; i++)
			
			coeffName.push_back(eiss.readString());
			
	

		
	
	
}
void CalPointingModelRow::coeffValFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		coeffVal.clear();
		
		unsigned int coeffValDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < coeffValDim1; i++)
			
			coeffVal.push_back(eiss.readFloat());
			
	

		
	
	
}
void CalPointingModelRow::coeffErrorFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		coeffError.clear();
		
		unsigned int coeffErrorDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < coeffErrorDim1; i++)
			
			coeffError.push_back(eiss.readFloat());
			
	

		
	
	
}
void CalPointingModelRow::coeffFixedFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		coeffFixed.clear();
		
		unsigned int coeffFixedDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < coeffFixedDim1; i++)
			
			coeffFixed.push_back(eiss.readBoolean());
			
	

		
	
	
}
void CalPointingModelRow::azimuthRMSFromBin(EndianISStream& eiss) {
		
	
		
		
		azimuthRMS =  Angle::fromBin(eiss);
		
	
	
}
void CalPointingModelRow::elevationRmsFromBin(EndianISStream& eiss) {
		
	
		
		
		elevationRms =  Angle::fromBin(eiss);
		
	
	
}
void CalPointingModelRow::skyRMSFromBin(EndianISStream& eiss) {
		
	
		
		
		skyRMS =  Angle::fromBin(eiss);
		
	
	
}
void CalPointingModelRow::reducedChiSquaredFromBin(EndianISStream& eiss) {
		
	
	
		
			
		reducedChiSquared =  eiss.readDouble();
			
		
	
	
}

void CalPointingModelRow::numObsFromBin(EndianISStream& eiss) {
		
	numObsExists = eiss.readBoolean();
	if (numObsExists) {
		
	
	
		
			
		numObs =  eiss.readInt();
			
		
	

	}
	
}
void CalPointingModelRow::coeffFormulaFromBin(EndianISStream& eiss) {
		
	coeffFormulaExists = eiss.readBoolean();
	if (coeffFormulaExists) {
		
	
	
		
			
	
		coeffFormula.clear();
		
		unsigned int coeffFormulaDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < coeffFormulaDim1; i++)
			
			coeffFormula.push_back(eiss.readString());
			
	

		
	

	}
	
}
	
	
	CalPointingModelRow* CalPointingModelRow::fromBin(EndianISStream& eiss, CalPointingModelTable& table, const vector<string>& attributesSeq) {
		CalPointingModelRow* row = new  CalPointingModelRow(table);
		
		map<string, CalPointingModelAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalPointingModelTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalPointingModelRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalPointingModel");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalPointingModelRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalPointingModel");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingModelRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingModelRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake CalPointingModelRow::getAntennaMake() const {
	
  		return antennaMake;
 	}

 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setAntennaMake (AntennaMakeMod::AntennaMake antennaMake)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaMake = antennaMake;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingModelMode.
 	 * @return pointingModelMode as PointingModelModeMod::PointingModelMode
 	 */
 	PointingModelModeMod::PointingModelMode CalPointingModelRow::getPointingModelMode() const {
	
  		return pointingModelMode;
 	}

 	/**
 	 * Set pointingModelMode with the specified PointingModelModeMod::PointingModelMode.
 	 * @param pointingModelMode The PointingModelModeMod::PointingModelMode value to which pointingModelMode is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setPointingModelMode (PointingModelModeMod::PointingModelMode pointingModelMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingModelMode = pointingModelMode;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationType.
 	 * @return polarizationType as PolarizationTypeMod::PolarizationType
 	 */
 	PolarizationTypeMod::PolarizationType CalPointingModelRow::getPolarizationType() const {
	
  		return polarizationType;
 	}

 	/**
 	 * Set polarizationType with the specified PolarizationTypeMod::PolarizationType.
 	 * @param polarizationType The PolarizationTypeMod::PolarizationType value to which polarizationType is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setPolarizationType (PolarizationTypeMod::PolarizationType polarizationType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationType = polarizationType;
	
 	}
	
	

	

	
 	/**
 	 * Get numCoeff.
 	 * @return numCoeff as int
 	 */
 	int CalPointingModelRow::getNumCoeff() const {
	
  		return numCoeff;
 	}

 	/**
 	 * Set numCoeff with the specified int.
 	 * @param numCoeff The int value to which numCoeff is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setNumCoeff (int numCoeff)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numCoeff = numCoeff;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffName.
 	 * @return coeffName as vector<string >
 	 */
 	vector<string > CalPointingModelRow::getCoeffName() const {
	
  		return coeffName;
 	}

 	/**
 	 * Set coeffName with the specified vector<string >.
 	 * @param coeffName The vector<string > value to which coeffName is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffName (vector<string > coeffName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffName = coeffName;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffVal.
 	 * @return coeffVal as vector<float >
 	 */
 	vector<float > CalPointingModelRow::getCoeffVal() const {
	
  		return coeffVal;
 	}

 	/**
 	 * Set coeffVal with the specified vector<float >.
 	 * @param coeffVal The vector<float > value to which coeffVal is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffVal (vector<float > coeffVal)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffVal = coeffVal;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffError.
 	 * @return coeffError as vector<float >
 	 */
 	vector<float > CalPointingModelRow::getCoeffError() const {
	
  		return coeffError;
 	}

 	/**
 	 * Set coeffError with the specified vector<float >.
 	 * @param coeffError The vector<float > value to which coeffError is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffError (vector<float > coeffError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffError = coeffError;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffFixed.
 	 * @return coeffFixed as vector<bool >
 	 */
 	vector<bool > CalPointingModelRow::getCoeffFixed() const {
	
  		return coeffFixed;
 	}

 	/**
 	 * Set coeffFixed with the specified vector<bool >.
 	 * @param coeffFixed The vector<bool > value to which coeffFixed is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffFixed (vector<bool > coeffFixed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffFixed = coeffFixed;
	
 	}
	
	

	

	
 	/**
 	 * Get azimuthRMS.
 	 * @return azimuthRMS as Angle
 	 */
 	Angle CalPointingModelRow::getAzimuthRMS() const {
	
  		return azimuthRMS;
 	}

 	/**
 	 * Set azimuthRMS with the specified Angle.
 	 * @param azimuthRMS The Angle value to which azimuthRMS is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setAzimuthRMS (Angle azimuthRMS)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->azimuthRMS = azimuthRMS;
	
 	}
	
	

	

	
 	/**
 	 * Get elevationRms.
 	 * @return elevationRms as Angle
 	 */
 	Angle CalPointingModelRow::getElevationRms() const {
	
  		return elevationRms;
 	}

 	/**
 	 * Set elevationRms with the specified Angle.
 	 * @param elevationRms The Angle value to which elevationRms is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setElevationRms (Angle elevationRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->elevationRms = elevationRms;
	
 	}
	
	

	

	
 	/**
 	 * Get skyRMS.
 	 * @return skyRMS as Angle
 	 */
 	Angle CalPointingModelRow::getSkyRMS() const {
	
  		return skyRMS;
 	}

 	/**
 	 * Set skyRMS with the specified Angle.
 	 * @param skyRMS The Angle value to which skyRMS is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setSkyRMS (Angle skyRMS)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->skyRMS = skyRMS;
	
 	}
	
	

	

	
 	/**
 	 * Get reducedChiSquared.
 	 * @return reducedChiSquared as double
 	 */
 	double CalPointingModelRow::getReducedChiSquared() const {
	
  		return reducedChiSquared;
 	}

 	/**
 	 * Set reducedChiSquared with the specified double.
 	 * @param reducedChiSquared The double value to which reducedChiSquared is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setReducedChiSquared (double reducedChiSquared)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->reducedChiSquared = reducedChiSquared;
	
 	}
	
	

	
	/**
	 * The attribute numObs is optional. Return true if this attribute exists.
	 * @return true if and only if the numObs attribute exists. 
	 */
	bool CalPointingModelRow::isNumObsExists() const {
		return numObsExists;
	}
	

	
 	/**
 	 * Get numObs, which is optional.
 	 * @return numObs as int
 	 * @throw IllegalAccessException If numObs does not exist.
 	 */
 	int CalPointingModelRow::getNumObs() const  {
		if (!numObsExists) {
			throw IllegalAccessException("numObs", "CalPointingModel");
		}
	
  		return numObs;
 	}

 	/**
 	 * Set numObs with the specified int.
 	 * @param numObs The int value to which numObs is to be set.
 	 
 	
 	 */
 	void CalPointingModelRow::setNumObs (int numObs) {
	
 		this->numObs = numObs;
	
		numObsExists = true;
	
 	}
	
	
	/**
	 * Mark numObs, which is an optional field, as non-existent.
	 */
	void CalPointingModelRow::clearNumObs () {
		numObsExists = false;
	}
	

	
	/**
	 * The attribute coeffFormula is optional. Return true if this attribute exists.
	 * @return true if and only if the coeffFormula attribute exists. 
	 */
	bool CalPointingModelRow::isCoeffFormulaExists() const {
		return coeffFormulaExists;
	}
	

	
 	/**
 	 * Get coeffFormula, which is optional.
 	 * @return coeffFormula as vector<string >
 	 * @throw IllegalAccessException If coeffFormula does not exist.
 	 */
 	vector<string > CalPointingModelRow::getCoeffFormula() const  {
		if (!coeffFormulaExists) {
			throw IllegalAccessException("coeffFormula", "CalPointingModel");
		}
	
  		return coeffFormula;
 	}

 	/**
 	 * Set coeffFormula with the specified vector<string >.
 	 * @param coeffFormula The vector<string > value to which coeffFormula is to be set.
 	 
 	
 	 */
 	void CalPointingModelRow::setCoeffFormula (vector<string > coeffFormula) {
	
 		this->coeffFormula = coeffFormula;
	
		coeffFormulaExists = true;
	
 	}
	
	
	/**
	 * Mark coeffFormula, which is an optional field, as non-existent.
	 */
	void CalPointingModelRow::clearCoeffFormula () {
		coeffFormulaExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalPointingModelRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalPointingModel");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalPointingModelRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalPointingModel");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalPointingModelRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalPointingModelRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalPointingModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPointingModelRow::CalPointingModelRow (CalPointingModelTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		numObsExists = false;
	

	
		coeffFormulaExists = false;
	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaMake = CAntennaMake::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
pointingModelMode = CPointingModelMode::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
polarizationType = CPolarizationType::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaName"] = &CalPointingModelRow::antennaNameFromBin; 
	 fromBinMethods["receiverBand"] = &CalPointingModelRow::receiverBandFromBin; 
	 fromBinMethods["calDataId"] = &CalPointingModelRow::calDataIdFromBin; 
	 fromBinMethods["calReductionId"] = &CalPointingModelRow::calReductionIdFromBin; 
	 fromBinMethods["startValidTime"] = &CalPointingModelRow::startValidTimeFromBin; 
	 fromBinMethods["endValidTime"] = &CalPointingModelRow::endValidTimeFromBin; 
	 fromBinMethods["antennaMake"] = &CalPointingModelRow::antennaMakeFromBin; 
	 fromBinMethods["pointingModelMode"] = &CalPointingModelRow::pointingModelModeFromBin; 
	 fromBinMethods["polarizationType"] = &CalPointingModelRow::polarizationTypeFromBin; 
	 fromBinMethods["numCoeff"] = &CalPointingModelRow::numCoeffFromBin; 
	 fromBinMethods["coeffName"] = &CalPointingModelRow::coeffNameFromBin; 
	 fromBinMethods["coeffVal"] = &CalPointingModelRow::coeffValFromBin; 
	 fromBinMethods["coeffError"] = &CalPointingModelRow::coeffErrorFromBin; 
	 fromBinMethods["coeffFixed"] = &CalPointingModelRow::coeffFixedFromBin; 
	 fromBinMethods["azimuthRMS"] = &CalPointingModelRow::azimuthRMSFromBin; 
	 fromBinMethods["elevationRms"] = &CalPointingModelRow::elevationRmsFromBin; 
	 fromBinMethods["skyRMS"] = &CalPointingModelRow::skyRMSFromBin; 
	 fromBinMethods["reducedChiSquared"] = &CalPointingModelRow::reducedChiSquaredFromBin; 
		
	
	 fromBinMethods["numObs"] = &CalPointingModelRow::numObsFromBin; 
	 fromBinMethods["coeffFormula"] = &CalPointingModelRow::coeffFormulaFromBin; 
	
	}
	
	CalPointingModelRow::CalPointingModelRow (CalPointingModelTable &t, CalPointingModelRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		numObsExists = false;
	

	
		coeffFormulaExists = false;
	

	
	

	
		
		}
		else {
	
		
			antennaName = row.antennaName;
		
			receiverBand = row.receiverBand;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			antennaMake = row.antennaMake;
		
			pointingModelMode = row.pointingModelMode;
		
			polarizationType = row.polarizationType;
		
			numCoeff = row.numCoeff;
		
			coeffName = row.coeffName;
		
			coeffVal = row.coeffVal;
		
			coeffError = row.coeffError;
		
			coeffFixed = row.coeffFixed;
		
			azimuthRMS = row.azimuthRMS;
		
			elevationRms = row.elevationRms;
		
			skyRMS = row.skyRMS;
		
			reducedChiSquared = row.reducedChiSquared;
		
		
		
		
		if (row.numObsExists) {
			numObs = row.numObs;		
			numObsExists = true;
		}
		else
			numObsExists = false;
		
		if (row.coeffFormulaExists) {
			coeffFormula = row.coeffFormula;		
			coeffFormulaExists = true;
		}
		else
			coeffFormulaExists = false;
		
		}
		
		 fromBinMethods["antennaName"] = &CalPointingModelRow::antennaNameFromBin; 
		 fromBinMethods["receiverBand"] = &CalPointingModelRow::receiverBandFromBin; 
		 fromBinMethods["calDataId"] = &CalPointingModelRow::calDataIdFromBin; 
		 fromBinMethods["calReductionId"] = &CalPointingModelRow::calReductionIdFromBin; 
		 fromBinMethods["startValidTime"] = &CalPointingModelRow::startValidTimeFromBin; 
		 fromBinMethods["endValidTime"] = &CalPointingModelRow::endValidTimeFromBin; 
		 fromBinMethods["antennaMake"] = &CalPointingModelRow::antennaMakeFromBin; 
		 fromBinMethods["pointingModelMode"] = &CalPointingModelRow::pointingModelModeFromBin; 
		 fromBinMethods["polarizationType"] = &CalPointingModelRow::polarizationTypeFromBin; 
		 fromBinMethods["numCoeff"] = &CalPointingModelRow::numCoeffFromBin; 
		 fromBinMethods["coeffName"] = &CalPointingModelRow::coeffNameFromBin; 
		 fromBinMethods["coeffVal"] = &CalPointingModelRow::coeffValFromBin; 
		 fromBinMethods["coeffError"] = &CalPointingModelRow::coeffErrorFromBin; 
		 fromBinMethods["coeffFixed"] = &CalPointingModelRow::coeffFixedFromBin; 
		 fromBinMethods["azimuthRMS"] = &CalPointingModelRow::azimuthRMSFromBin; 
		 fromBinMethods["elevationRms"] = &CalPointingModelRow::elevationRmsFromBin; 
		 fromBinMethods["skyRMS"] = &CalPointingModelRow::skyRMSFromBin; 
		 fromBinMethods["reducedChiSquared"] = &CalPointingModelRow::reducedChiSquaredFromBin; 
			
	
		 fromBinMethods["numObs"] = &CalPointingModelRow::numObsFromBin; 
		 fromBinMethods["coeffFormula"] = &CalPointingModelRow::coeffFormulaFromBin; 
			
	}

	
	bool CalPointingModelRow::compareNoAutoInc(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, AntennaMakeMod::AntennaMake antennaMake, PointingModelModeMod::PointingModelMode pointingModelMode, PolarizationTypeMod::PolarizationType polarizationType, int numCoeff, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, Angle azimuthRMS, Angle elevationRms, Angle skyRMS, double reducedChiSquared) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingModelMode == pointingModelMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationType == polarizationType);
		
		if (!result) return false;
	

	
		
		result = result && (this->numCoeff == numCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffName == coeffName);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffVal == coeffVal);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffError == coeffError);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffFixed == coeffFixed);
		
		if (!result) return false;
	

	
		
		result = result && (this->azimuthRMS == azimuthRMS);
		
		if (!result) return false;
	

	
		
		result = result && (this->elevationRms == elevationRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->skyRMS == skyRMS);
		
		if (!result) return false;
	

	
		
		result = result && (this->reducedChiSquared == reducedChiSquared);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalPointingModelRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, AntennaMakeMod::AntennaMake antennaMake, PointingModelModeMod::PointingModelMode pointingModelMode, PolarizationTypeMod::PolarizationType polarizationType, int numCoeff, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, Angle azimuthRMS, Angle elevationRms, Angle skyRMS, double reducedChiSquared) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->antennaMake == antennaMake)) return false;
	

	
		if (!(this->pointingModelMode == pointingModelMode)) return false;
	

	
		if (!(this->polarizationType == polarizationType)) return false;
	

	
		if (!(this->numCoeff == numCoeff)) return false;
	

	
		if (!(this->coeffName == coeffName)) return false;
	

	
		if (!(this->coeffVal == coeffVal)) return false;
	

	
		if (!(this->coeffError == coeffError)) return false;
	

	
		if (!(this->coeffFixed == coeffFixed)) return false;
	

	
		if (!(this->azimuthRMS == azimuthRMS)) return false;
	

	
		if (!(this->elevationRms == elevationRms)) return false;
	

	
		if (!(this->skyRMS == skyRMS)) return false;
	

	
		if (!(this->reducedChiSquared == reducedChiSquared)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPointingModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalPointingModelRow::equalByRequiredValue(CalPointingModelRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->antennaMake != x->antennaMake) return false;
			
		if (this->pointingModelMode != x->pointingModelMode) return false;
			
		if (this->polarizationType != x->polarizationType) return false;
			
		if (this->numCoeff != x->numCoeff) return false;
			
		if (this->coeffName != x->coeffName) return false;
			
		if (this->coeffVal != x->coeffVal) return false;
			
		if (this->coeffError != x->coeffError) return false;
			
		if (this->coeffFixed != x->coeffFixed) return false;
			
		if (this->azimuthRMS != x->azimuthRMS) return false;
			
		if (this->elevationRms != x->elevationRms) return false;
			
		if (this->skyRMS != x->skyRMS) return false;
			
		if (this->reducedChiSquared != x->reducedChiSquared) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalPointingModelAttributeFromBin> CalPointingModelRow::initFromBinMethods() {
		map<string, CalPointingModelAttributeFromBin> result;
		
		result["antennaName"] = &CalPointingModelRow::antennaNameFromBin;
		result["receiverBand"] = &CalPointingModelRow::receiverBandFromBin;
		result["calDataId"] = &CalPointingModelRow::calDataIdFromBin;
		result["calReductionId"] = &CalPointingModelRow::calReductionIdFromBin;
		result["startValidTime"] = &CalPointingModelRow::startValidTimeFromBin;
		result["endValidTime"] = &CalPointingModelRow::endValidTimeFromBin;
		result["antennaMake"] = &CalPointingModelRow::antennaMakeFromBin;
		result["pointingModelMode"] = &CalPointingModelRow::pointingModelModeFromBin;
		result["polarizationType"] = &CalPointingModelRow::polarizationTypeFromBin;
		result["numCoeff"] = &CalPointingModelRow::numCoeffFromBin;
		result["coeffName"] = &CalPointingModelRow::coeffNameFromBin;
		result["coeffVal"] = &CalPointingModelRow::coeffValFromBin;
		result["coeffError"] = &CalPointingModelRow::coeffErrorFromBin;
		result["coeffFixed"] = &CalPointingModelRow::coeffFixedFromBin;
		result["azimuthRMS"] = &CalPointingModelRow::azimuthRMSFromBin;
		result["elevationRms"] = &CalPointingModelRow::elevationRmsFromBin;
		result["skyRMS"] = &CalPointingModelRow::skyRMSFromBin;
		result["reducedChiSquared"] = &CalPointingModelRow::reducedChiSquaredFromBin;
		
		
		result["numObs"] = &CalPointingModelRow::numObsFromBin;
		result["coeffFormula"] = &CalPointingModelRow::coeffFormulaFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
