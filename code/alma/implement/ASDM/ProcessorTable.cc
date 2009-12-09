
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
 * File ProcessorTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <ProcessorTable.h>
#include <ProcessorRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::ProcessorTable;
using asdm::ProcessorRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string ProcessorTable::tableName = "Processor";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> ProcessorTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> ProcessorTable::getKeyName() {
		return key;
	}


	ProcessorTable::ProcessorTable(ASDM &c) : container(c) {

	
		key.push_back("processorId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("ProcessorTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for ProcessorTable.
 */
 
	ProcessorTable::~ProcessorTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &ProcessorTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int ProcessorTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string ProcessorTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity ProcessorTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void ProcessorTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	ProcessorRow *ProcessorTable::newRow() {
		return new ProcessorRow (*this);
	}
	
	ProcessorRow *ProcessorTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param modeId. 
	
 	 * @param processorType. 
	
 	 * @param processorSubType. 
	
     */
	ProcessorRow* ProcessorTable::newRow(Tag modeId, ProcessorTypeMod::ProcessorType processorType, ProcessorSubTypeMod::ProcessorSubType processorSubType){
		ProcessorRow *row = new ProcessorRow(*this);
			
		row->setModeId(modeId);
			
		row->setProcessorType(processorType);
			
		row->setProcessorSubType(processorSubType);
	
		return row;		
	}	

	ProcessorRow* ProcessorTable::newRowFull(Tag modeId, ProcessorTypeMod::ProcessorType processorType, ProcessorSubTypeMod::ProcessorSubType processorSubType)	{
		ProcessorRow *row = new ProcessorRow(*this);
			
		row->setModeId(modeId);
			
		row->setProcessorType(processorType);
			
		row->setProcessorSubType(processorSubType);
	
		return row;				
	}
	


ProcessorRow* ProcessorTable::newRow(ProcessorRow* row) {
	return new ProcessorRow(*this, *row);
}

ProcessorRow* ProcessorTable::newRowCopy(ProcessorRow* row) {
	return new ProcessorRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.processorId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a ProcessorRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	ProcessorRow* ProcessorTable::add(ProcessorRow* x) {
			 
		ProcessorRow* aRow = lookup(
				
		x->getModeId()
				,
		x->getProcessorType()
				,
		x->getProcessorSubType()
				
		);
		if (aRow) return aRow;
			

			
		// Autoincrement processorId
		x->setProcessorId(Tag(size(), TagType::Processor));
						
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
		return x;
	}
		
		





	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @param x a pointer on the row to be appended.
	 * @returns a pointer on x.
	 * @throws DuplicateKey
	 
	 * @throws UniquenessViolationException
	 
	 */
	ProcessorRow*  ProcessorTable::checkAndAdd(ProcessorRow* x)  {
	 
		 
		if (lookup(
			
			x->getModeId()
		,
			x->getProcessorType()
		,
			x->getProcessorSubType()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table ProcessorTable");
		
		
		
		if (getRowByKey(
	
			x->getProcessorId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "ProcessorTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
		return x;	
	}	







	

	//
	// ====> Methods returning rows.
	//	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of ProcessorRow
	 */
	vector<ProcessorRow *> ProcessorTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a ProcessorRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	ProcessorRow* ProcessorTable::getRowByKey(Tag processorId)  {
	ProcessorRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->processorId != processorId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param modeId.
 	 		
 * @param processorType.
 	 		
 * @param processorSubType.
 	 		 
 */
ProcessorRow* ProcessorTable::lookup(Tag modeId, ProcessorTypeMod::ProcessorType processorType, ProcessorSubTypeMod::ProcessorSubType processorSubType) {
		ProcessorRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(modeId, processorType, processorSubType)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	ProcessorTableIDL *ProcessorTable::toIDL() {
		ProcessorTableIDL *x = new ProcessorTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<ProcessorRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void ProcessorTable::fromIDL(ProcessorTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			ProcessorRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *ProcessorTable::toFITS() const  {
		throw ConversionException("Not implemented","Processor");
	}

	void ProcessorTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","Processor");
	}

	string ProcessorTable::toVOTable() const {
		throw ConversionException("Not implemented","Processor");
	}

	void ProcessorTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","Processor");
	}

	
	string ProcessorTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<ProcessorTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/ProcessorTable\" xsi:schemaLocation=\"http://Alma/XASDM/ProcessorTable http://almaobservatory.org/XML/XASDM/2/ProcessorTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<ProcessorRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</ProcessorTable> ");
		return buf;
	}

	
	void ProcessorTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<ProcessorTable")) 
			error();
		// cout << "Parsing a ProcessorTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "ProcessorTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		ProcessorRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a ProcessorRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"ProcessorTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"ProcessorTable");	
			}
			catch (...) {
				// cout << "Unexpected error in ProcessorTable::checkAndAdd called from ProcessorTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</ProcessorTable>")) 
			error();
	}

	
	void ProcessorTable::error()  {
		throw ConversionException("Invalid xml document","Processor");
	}
	
	
	string ProcessorTable::toMIME() {
		EndianOSStream eoss;
		
		string UID = getEntity().getEntityId().toString();
		string execBlockUID = getContainer().getEntity().getEntityId().toString();
		
		// The MIME Header
		eoss <<"MIME-Version: 1.0";
		eoss << "\n";
		eoss << "Content-Type: Multipart/Related; boundary='MIME_boundary'; type='text/xml'; start= '<header.xml>'";
		eoss <<"\n";
		eoss <<"Content-Description: Correlator";
		eoss <<"\n";
		eoss <<"alma-uid:" << UID;
		eoss <<"\n";
		eoss <<"\n";		
		
		// The MIME XML part header.
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: text/xml; charset='ISO-8859-1'";
		eoss <<"\n";
		eoss <<"Content-Transfer-Encoding: 8bit";
		eoss <<"\n";
		eoss <<"Content-ID: <header.xml>";
		eoss <<"\n";
		eoss <<"\n";
		
		// The MIME XML part content.
		eoss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		eoss << "\n";
		eoss<< "<ASDMBinaryTable  xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'  xsi:noNamespaceSchemaLocation='ASDMBinaryTable.xsd' ID='None'  version='1.0'>\n";
		eoss << "<ExecBlockUID>\n";
		eoss << execBlockUID  << "\n";
		eoss << "</ExecBlockUID>\n";
		eoss << "</ASDMBinaryTable>\n";		

		// The MIME binary part header
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: binary/octet-stream";
		eoss <<"\n";
		eoss <<"Content-ID: <content.bin>";
		eoss <<"\n";
		eoss <<"\n";	
		
		// The MIME binary content
		entity.toBin(eoss);
		container.getEntity().toBin(eoss);
		eoss.writeInt((int) privateRows.size());
		for (unsigned int i = 0; i < privateRows.size(); i++) {
			privateRows.at(i)->toBin(eoss);	
		}
		
		// The closing MIME boundary
		eoss << "\n--MIME_boundary--";
		eoss << "\n";
		
		return eoss.str();	
	}

	
	void ProcessorTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "Processor");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			ProcessorRow* aRow = ProcessorRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "Processor");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "Processor");
		} 		 	
	}

	
	void ProcessorTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Processor.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Processor");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Processor");
		}
		else {
			// write the XML
			string fileName = directory + "/Processor.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Processor");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Processor");
		}
	}

	
	void ProcessorTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/Processor.bin";
		else
			tablename = directory + "/Processor.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "Processor");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"Processor");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"Processor");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
	void ProcessorTable::autoIncrement(string key, ProcessorRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  processorId to Tag(0).
			x->setProcessorId(Tag(0,  TagType::Processor));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  processorId to Tag(n).
			x->setProcessorId(Tag(n, TagType::Processor));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
