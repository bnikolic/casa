
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
 * File FieldRow.h
 */
 
#ifndef FieldRow_CLASS
#define FieldRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::FieldRowIDL;
#endif






#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;




	

	

	

	

	

	

	

	

	
#include "CDirectionReferenceCode.h"
using namespace DirectionReferenceCodeMod;
	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file Field.h
    \brief Generated from model's revision "1.54", branch "HEAD"
*/

namespace asdm {

//class asdm::FieldTable;


// class asdm::EphemerisRow;
class EphemerisRow;

// class asdm::SourceRow;
class SourceRow;

// class asdm::FieldRow;
class FieldRow;
	

class FieldRow;
typedef void (FieldRow::*FieldAttributeFromBin) (EndianISStream& eiss);

/**
 * The FieldRow class is a row of a FieldTable.
 * 
 * Generated from model's revision "1.54", branch "HEAD"
 *
 */
class FieldRow {
friend class asdm::FieldTable;

public:

	virtual ~FieldRow();

	/**
	 * Return the table to which this row belongs.
	 */
	FieldTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute fieldId
	
	
	

	
 	/**
 	 * Get fieldId.
 	 * @return fieldId as Tag
 	 */
 	Tag getFieldId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute fieldName
	
	
	

	
 	/**
 	 * Get fieldName.
 	 * @return fieldName as string
 	 */
 	string getFieldName() const;
	
 
 	
 	
 	/**
 	 * Set fieldName with the specified string.
 	 * @param fieldName The string value to which fieldName is to be set.
 	 
 		
 			
 	 */
 	void setFieldName (string fieldName);
  		
	
	
	


	
	// ===> Attribute code
	
	
	

	
 	/**
 	 * Get code.
 	 * @return code as string
 	 */
 	string getCode() const;
	
 
 	
 	
 	/**
 	 * Set code with the specified string.
 	 * @param code The string value to which code is to be set.
 	 
 		
 			
 	 */
 	void setCode (string code);
  		
	
	
	


	
	// ===> Attribute numPoly
	
	
	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int getNumPoly() const;
	
 
 	
 	
 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 		
 			
 	 */
 	void setNumPoly (int numPoly);
  		
	
	
	


	
	// ===> Attribute delayDir
	
	
	

	
 	/**
 	 * Get delayDir.
 	 * @return delayDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getDelayDir() const;
	
 
 	
 	
 	/**
 	 * Set delayDir with the specified vector<vector<Angle > >.
 	 * @param delayDir The vector<vector<Angle > > value to which delayDir is to be set.
 	 
 		
 			
 	 */
 	void setDelayDir (vector<vector<Angle > > delayDir);
  		
	
	
	


	
	// ===> Attribute phaseDir
	
	
	

	
 	/**
 	 * Get phaseDir.
 	 * @return phaseDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getPhaseDir() const;
	
 
 	
 	
 	/**
 	 * Set phaseDir with the specified vector<vector<Angle > >.
 	 * @param phaseDir The vector<vector<Angle > > value to which phaseDir is to be set.
 	 
 		
 			
 	 */
 	void setPhaseDir (vector<vector<Angle > > phaseDir);
  		
	
	
	


	
	// ===> Attribute referenceDir
	
	
	

	
 	/**
 	 * Get referenceDir.
 	 * @return referenceDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getReferenceDir() const;
	
 
 	
 	
 	/**
 	 * Set referenceDir with the specified vector<vector<Angle > >.
 	 * @param referenceDir The vector<vector<Angle > > value to which referenceDir is to be set.
 	 
 		
 			
 	 */
 	void setReferenceDir (vector<vector<Angle > > referenceDir);
  		
	
	
	


	
	// ===> Attribute time, which is optional
	
	
	
	/**
	 * The attribute time is optional. Return true if this attribute exists.
	 * @return true if and only if the time attribute exists. 
	 */
	bool isTimeExists() const;
	

	
 	/**
 	 * Get time, which is optional.
 	 * @return time as ArrayTime
 	 * @throws IllegalAccessException If time does not exist.
 	 */
 	ArrayTime getTime() const;
	
 
 	
 	
 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 		
 	 */
 	void setTime (ArrayTime time);
		
	
	
	
	/**
	 * Mark time, which is an optional field, as non-existent.
	 */
	void clearTime ();
	


	
	// ===> Attribute directionCode, which is optional
	
	
	
	/**
	 * The attribute directionCode is optional. Return true if this attribute exists.
	 * @return true if and only if the directionCode attribute exists. 
	 */
	bool isDirectionCodeExists() const;
	

	
 	/**
 	 * Get directionCode, which is optional.
 	 * @return directionCode as DirectionReferenceCodeMod::DirectionReferenceCode
 	 * @throws IllegalAccessException If directionCode does not exist.
 	 */
 	DirectionReferenceCodeMod::DirectionReferenceCode getDirectionCode() const;
	
 
 	
 	
 	/**
 	 * Set directionCode with the specified DirectionReferenceCodeMod::DirectionReferenceCode.
 	 * @param directionCode The DirectionReferenceCodeMod::DirectionReferenceCode value to which directionCode is to be set.
 	 
 		
 	 */
 	void setDirectionCode (DirectionReferenceCodeMod::DirectionReferenceCode directionCode);
		
	
	
	
	/**
	 * Mark directionCode, which is an optional field, as non-existent.
	 */
	void clearDirectionCode ();
	


	
	// ===> Attribute directionEquinox, which is optional
	
	
	
	/**
	 * The attribute directionEquinox is optional. Return true if this attribute exists.
	 * @return true if and only if the directionEquinox attribute exists. 
	 */
	bool isDirectionEquinoxExists() const;
	

	
 	/**
 	 * Get directionEquinox, which is optional.
 	 * @return directionEquinox as ArrayTime
 	 * @throws IllegalAccessException If directionEquinox does not exist.
 	 */
 	ArrayTime getDirectionEquinox() const;
	
 
 	
 	
 	/**
 	 * Set directionEquinox with the specified ArrayTime.
 	 * @param directionEquinox The ArrayTime value to which directionEquinox is to be set.
 	 
 		
 	 */
 	void setDirectionEquinox (ArrayTime directionEquinox);
		
	
	
	
	/**
	 * Mark directionEquinox, which is an optional field, as non-existent.
	 */
	void clearDirectionEquinox ();
	


	
	// ===> Attribute assocNature, which is optional
	
	
	
	/**
	 * The attribute assocNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocNature attribute exists. 
	 */
	bool isAssocNatureExists() const;
	

	
 	/**
 	 * Get assocNature, which is optional.
 	 * @return assocNature as string
 	 * @throws IllegalAccessException If assocNature does not exist.
 	 */
 	string getAssocNature() const;
	
 
 	
 	
 	/**
 	 * Set assocNature with the specified string.
 	 * @param assocNature The string value to which assocNature is to be set.
 	 
 		
 	 */
 	void setAssocNature (string assocNature);
		
	
	
	
	/**
	 * Mark assocNature, which is an optional field, as non-existent.
	 */
	void clearAssocNature ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute assocFieldId, which is optional
	
	
	
	/**
	 * The attribute assocFieldId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocFieldId attribute exists. 
	 */
	bool isAssocFieldIdExists() const;
	

	
 	/**
 	 * Get assocFieldId, which is optional.
 	 * @return assocFieldId as Tag
 	 * @throws IllegalAccessException If assocFieldId does not exist.
 	 */
 	Tag getAssocFieldId() const;
	
 
 	
 	
 	/**
 	 * Set assocFieldId with the specified Tag.
 	 * @param assocFieldId The Tag value to which assocFieldId is to be set.
 	 
 		
 	 */
 	void setAssocFieldId (Tag assocFieldId);
		
	
	
	
	/**
	 * Mark assocFieldId, which is an optional field, as non-existent.
	 */
	void clearAssocFieldId ();
	


	
	// ===> Attribute ephemerisId, which is optional
	
	
	
	/**
	 * The attribute ephemerisId is optional. Return true if this attribute exists.
	 * @return true if and only if the ephemerisId attribute exists. 
	 */
	bool isEphemerisIdExists() const;
	

	
 	/**
 	 * Get ephemerisId, which is optional.
 	 * @return ephemerisId as Tag
 	 * @throws IllegalAccessException If ephemerisId does not exist.
 	 */
 	Tag getEphemerisId() const;
	
 
 	
 	
 	/**
 	 * Set ephemerisId with the specified Tag.
 	 * @param ephemerisId The Tag value to which ephemerisId is to be set.
 	 
 		
 	 */
 	void setEphemerisId (Tag ephemerisId);
		
	
	
	
	/**
	 * Mark ephemerisId, which is an optional field, as non-existent.
	 */
	void clearEphemerisId ();
	


	
	// ===> Attribute sourceId, which is optional
	
	
	
	/**
	 * The attribute sourceId is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceId attribute exists. 
	 */
	bool isSourceIdExists() const;
	

	
 	/**
 	 * Get sourceId, which is optional.
 	 * @return sourceId as int
 	 * @throws IllegalAccessException If sourceId does not exist.
 	 */
 	int getSourceId() const;
	
 
 	
 	
 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 		
 	 */
 	void setSourceId (int sourceId);
		
	
	
	
	/**
	 * Mark sourceId, which is an optional field, as non-existent.
	 */
	void clearSourceId ();
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * ephemerisId pointer to the row in the Ephemeris table having Ephemeris.ephemerisId == ephemerisId
	 * @return a EphemerisRow*
	 * 
	 
	 * throws IllegalAccessException
	 
	 */
	 EphemerisRow* getEphemerisUsingEphemerisId();
	 

	

	

	
		
	// ===> Slice link from a row of Field table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having sourceId == this.sourceId
	 * 
	 * @return a vector of SourceRow *
	 */
	vector <SourceRow *> getSources();
	
	

	

	

	
		
	/**
	 * assocFieldId pointer to the row in the Field table having Field.assocFieldId == assocFieldId
	 * @return a FieldRow*
	 * 
	 
	 * throws IllegalAccessException
	 
	 */
	 FieldRow* getFieldUsingAssocFieldId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this FieldRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param fieldName
	    
	 * @param code
	    
	 * @param numPoly
	    
	 * @param delayDir
	    
	 * @param phaseDir
	    
	 * @param referenceDir
	    
	 */ 
	bool compareNoAutoInc(string fieldName, string code, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param fieldName
	    
	 * @param code
	    
	 * @param numPoly
	    
	 * @param delayDir
	    
	 * @param phaseDir
	    
	 * @param referenceDir
	    
	 */ 
	bool compareRequiredValue(string fieldName, string code, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FieldRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(FieldRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FieldRowIDL struct.
	 */
	FieldRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FieldRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (FieldRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (string rowDoc) ;	

private:
	/**
	 * The table to which this row belongs.
	 */
	FieldTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a FieldRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FieldRow (FieldTable &table);

	/**
	 * Create a FieldRow using a copy constructor mechanism.
	 * <p>
	 * Given a FieldRow row and a FieldTable table, the method creates a new
	 * FieldRow owned by table. Each attribute of the created row is a copy (deep)
	 * of the corresponding attribute of row. The method does not add the created
	 * row to its table, its simply parents it to table, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a row with default values for its attributes. 
	 *
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 * @param row  The row which is to be copied.
	 */
	 FieldRow (FieldTable &table, FieldRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute fieldId
	
	

	Tag fieldId;

	
	
 	
 	/**
 	 * Set fieldId with the specified Tag value.
 	 * @param fieldId The Tag value to which fieldId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setFieldId (Tag fieldId);
  		
	

	
	// ===> Attribute fieldName
	
	

	string fieldName;

	
	
 	

	
	// ===> Attribute code
	
	

	string code;

	
	
 	

	
	// ===> Attribute numPoly
	
	

	int numPoly;

	
	
 	

	
	// ===> Attribute delayDir
	
	

	vector<vector<Angle > > delayDir;

	
	
 	

	
	// ===> Attribute phaseDir
	
	

	vector<vector<Angle > > phaseDir;

	
	
 	

	
	// ===> Attribute referenceDir
	
	

	vector<vector<Angle > > referenceDir;

	
	
 	

	
	// ===> Attribute time, which is optional
	
	
	bool timeExists;
	

	ArrayTime time;

	
	
 	

	
	// ===> Attribute directionCode, which is optional
	
	
	bool directionCodeExists;
	

	DirectionReferenceCodeMod::DirectionReferenceCode directionCode;

	
	
 	

	
	// ===> Attribute directionEquinox, which is optional
	
	
	bool directionEquinoxExists;
	

	ArrayTime directionEquinox;

	
	
 	

	
	// ===> Attribute assocNature, which is optional
	
	
	bool assocNatureExists;
	

	string assocNature;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute assocFieldId, which is optional
	
	
	bool assocFieldIdExists;
	

	Tag assocFieldId;

	
	
 	

	
	// ===> Attribute ephemerisId, which is optional
	
	
	bool ephemerisIdExists;
	

	Tag ephemerisId;

	
	
 	

	
	// ===> Attribute sourceId, which is optional
	
	
	bool sourceIdExists;
	

	int sourceId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		


	

	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, FieldAttributeFromBin> fromBinMethods;
void fieldIdFromBin( EndianISStream& eiss);
void fieldNameFromBin( EndianISStream& eiss);
void codeFromBin( EndianISStream& eiss);
void numPolyFromBin( EndianISStream& eiss);
void delayDirFromBin( EndianISStream& eiss);
void phaseDirFromBin( EndianISStream& eiss);
void referenceDirFromBin( EndianISStream& eiss);

void timeFromBin( EndianISStream& eiss);
void directionCodeFromBin( EndianISStream& eiss);
void directionEquinoxFromBin( EndianISStream& eiss);
void assocNatureFromBin( EndianISStream& eiss);
void ephemerisIdFromBin( EndianISStream& eiss);
void sourceIdFromBin( EndianISStream& eiss);
void assocFieldIdFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the FieldTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static FieldRow* fromBin(EndianISStream& eiss, FieldTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* Field_CLASS */
