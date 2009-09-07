
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
 * File CalGainTable.h
 */
 
#ifndef CalGainTable_CLASS
#define CalGainTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;

#include <Angle.h>
#include <AngularRate.h>
#include <ArrayTime.h>
#include <ArrayTimeInterval.h>
#include <Complex.h>
#include <Entity.h>
#include <EntityId.h>
#include <EntityRef.h>
#include <Flux.h>
#include <Frequency.h>
#include <Humidity.h>
#include <Interval.h>
#include <Length.h>
#include <PartId.h>
#include <Pressure.h>
#include <Speed.h>
#include <Tag.h>
#include <Temperature.h>
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <UniquenessViolationException.h>
#include <NoSuchRow.h>
#include <DuplicateKey.h>

/*
#include <Enumerations.h>
using namespace enumerations;
*/




	

	

	

	

	

	

	

	

	


#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalGainTableIDL;
#endif

using asdm::Angle;
using asdm::AngularRate;
using asdm::ArrayTime;
using asdm::Complex;
using asdm::Entity;
using asdm::EntityId;
using asdm::EntityRef;
using asdm::Flux;
using asdm::Frequency;
using asdm::Humidity;
using asdm::Interval;
using asdm::Length;
using asdm::PartId;
using asdm::Pressure;
using asdm::Speed;
using asdm::Tag;
using asdm::Temperature;

using asdm::DuplicateKey;
using asdm::ConversionException;
using asdm::NoSuchRow;
using asdm::DuplicateKey;

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::CalGainRow;

class ASDM;
class CalGainRow;
/**
 * The CalGainTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * This Table is a placeholder to be used to wrap up casa gain tables produced  in the Science Pipeline and Offline so that they can be archived in the  ALMA Calibration Data Base.
 * <BR>
 
 * Generated from model's revision "1.50.2.3", branch "WVR-2009-07-B"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of CalGain </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> calDataId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in CalData Table. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> calReductionId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row  in CalReductionTable. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> startValidTime </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp; the start time of result validity period. </TD>
 * </TR>
	
 * <TR>
 * <TD> endValidTime </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp; the end time of result validity period. </TD>
 * </TR>
	
 * <TR>
 * <TD> gain </TD> 
 * <TD> float </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;TBD </TD>
 * </TR>
	
 * <TR>
 * <TD> gainValid </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;TBD </TD>
 * </TR>
	
 * <TR>
 * <TD> fit </TD> 
 * <TD> float </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;TBD </TD>
 * </TR>
	
 * <TR>
 * <TD> fitWeight </TD> 
 * <TD> float </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;TBD </TD>
 * </TR>
	
 * <TR>
 * <TD> totalGainValid </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;TBD </TD>
 * </TR>
	
 * <TR>
 * <TD> totalFit </TD> 
 * <TD> float </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;TBD </TD>
 * </TR>
	
 * <TR>
 * <TD> totalFitWeight </TD> 
 * <TD> float </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;TBD </TD>
 * </TR>
	


 * </TABLE>
 */
class CalGainTable : public Representable {
	friend class asdm::ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~CalGainTable();
	
	/**
	 * Return the container to which this table belongs.
	 *
	 * @return the ASDM containing this table.
	 */
	ASDM &getContainer() const;
	
	/**
	 * Return the number of rows in the table.
	 *
	 * @return the number of rows in an unsigned int.
	 */
	unsigned int size() ;
	
	/**
	 * Return the name of this table.
	 *
	 * @return the name of this table in a string.
	 */
	string getName() const;

	/**
	 * Return this table's Entity.
	 */
	Entity getEntity() const;

	/**
	 * Set this table's Entity.
	 * @param e An entity. 
	 */
	void setEntity(Entity e);

	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a CalGainRow
	 */
	CalGainRow *newRow();
	
	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	CalGainRow* newRowEmpty();

	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param gain. 
	
 	 * @param gainValid. 
	
 	 * @param fit. 
	
 	 * @param fitWeight. 
	
 	 * @param totalGainValid. 
	
 	 * @param totalFit. 
	
 	 * @param totalFitWeight. 
	
     */
	CalGainRow *newRow(Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, float gain, bool gainValid, float fit, float fitWeight, bool totalGainValid, float totalFit, float totalFitWeight);
	
	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	CalGainRow *newRowFull(Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, float gain, bool gainValid, float fit, float fitWeight, bool totalGainValid, float totalFit, float totalFitWeight);


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new CalGainRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new CalGainRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 CalGainRow *newRow(CalGainRow *row); 

	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	 CalGainRow *newRowCopy(CalGainRow *row); 

	//
	// ====> Append a row to its table.
	//
 
	
	/**
	 * Add a row.
	 * @param x a pointer to the CalGainRow to be added.
	 *
	 * @return a pointer to a CalGainRow. If the table contains a CalGainRow whose attributes (key and mandatory values) are equal to x ones
	 * then returns a pointer on that CalGainRow, otherwise returns x.
	 *
	 * @throw DuplicateKey { thrown when the table contains a CalGainRow with a key equal to the x one but having
	 * and a value section different from x one }
	 *
	
	 */
	CalGainRow* add(CalGainRow* x) ; 

 



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of CalGainRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalGainTable.
	 */
	vector<CalGainRow *> get() ;
	


 
	
	/**
 	 * Returns a CalGainRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param calDataId. 
	
	 * @param calReductionId. 
	
 	 *
	 */
 	CalGainRow* getRowByKey(Tag calDataId, Tag calReductionId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param calDataId.
 	 		
 	 * @param calReductionId.
 	 		
 	 * @param startValidTime.
 	 		
 	 * @param endValidTime.
 	 		
 	 * @param gain.
 	 		
 	 * @param gainValid.
 	 		
 	 * @param fit.
 	 		
 	 * @param fitWeight.
 	 		
 	 * @param totalGainValid.
 	 		
 	 * @param totalFit.
 	 		
 	 * @param totalFitWeight.
 	 		 
 	 */
	CalGainRow* lookup(Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, float gain, bool gainValid, float fit, float fitWeight, bool totalGainValid, float totalFit, float totalFitWeight); 


#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a CalGainTableIDL CORBA structure.
	 *
	 * @return a pointer to a CalGainTableIDL
	 */
	CalGainTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a CalGainTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(CalGainTableIDL x) ;
#endif

	/**
	 * To be implemented
	 * @throws ConversionException
	 */
	char *toFITS() const ;

	/**
	 * To be implemented
	 * @throws ConversionException
	 */
	void fromFITS(char *fits) ;

	/**
	 * To be implemented
	 * @throw ConversionException
	 */
	string toVOTable() const ;

	/**
	 * To be implemented
	 * @throws ConversionException
	 */
	void fromVOTable(string vo) ;

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for CalGain (CalGainTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a CalGain (CalGainTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
	
   /**
	 * Serialize this into a stream of bytes and encapsulates that stream into a MIME message.
	 * @returns a string containing the MIME message.
	 * 
	 */
	string toMIME();
	
   /** 
     * Extracts the binary part of a MIME message and deserialize its content
	 * to fill this with the result of the deserialization. 
	 * @param mimeMsg the string containing the MIME message.
	 * @throws ConversionException
	 */
	 void setFromMIME(const string & mimeMsg);
	
	/**
	  * Stores a representation (binary or XML) of this table into a file.
	  *
	  * Depending on the boolean value of its private field fileAsBin a binary serialization  of this (fileAsBin==true)  
	  * will be saved in a file "CalGain.bin" or an XML representation (fileAsBin==false) will be saved in a file "CalGain.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a CalGainTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	

private:

	/**
	 * Create a CalGainTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	CalGainTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	


	/**
	 * The name of this table.
	 */
	static string tableName;


	/**
	 * The list of field names that make up key key.
	 */
	static vector<string> key;


	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @throws DuplicateKey
	 
	 */
	CalGainRow* checkAndAdd(CalGainRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private ArrayList of CalGainRow s.
   vector<CalGainRow * > privateRows;
   

			
	vector<CalGainRow *> row;


	void error() ; //throw(ConversionException);

};

} // End namespace asdm

#endif /* CalGainTable_CLASS */
