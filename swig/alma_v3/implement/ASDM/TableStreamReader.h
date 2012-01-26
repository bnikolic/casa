#ifndef TABLE_STREAM_READER_H
#define TABLE_STREAM_READER_H
#include "Misc.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "ASDM.h"
#include "Entity.h"
#include "EndianStream.h"
#include "ConversionException.h"

namespace asdm {
  /**
   * A generic class to read a file containing an ASDM table as a stream.
   *
   * The purpose of this class is to provide the programmer with a way to read a file containing an ASDM table by successive
   * slices. Each slice is transformed into the in memory representation of the rows which have been read. A slice can be specified
   * in two ways :
   * <ul>
   * <li> a number of rows. See methods nextNRows.</li>
   * <li> a number of bytes. See methods untilNBytes.</li>
   * </ul>
   *
   * <b>Remarks</b>
   * <ul>
   *<li> This class has been meant as an alternative to the historical ASDM methods <code>getXXXX</code> (e.g. <code>getAntenna</code> of 
   * <code>getPointing</code>) which load entirely the tables into memory. </li>
   *<li> This class allows to read a table completely independantly of the ASDM it belongs to and of the other tables of the ASDM. </li>
   *</ul>
   *
   * @param T The parameter T must be the type of one of the tables which can be found in an ASDM (e.g. AntennaTable)
   * @param R The parameter R must be the type of the rows which make the content of a table of type T (e.g. AntennaRow is T is AntennaTable).
   */
  template<class T, class R> class TableStreamReader {
  public:
    /**
     * An empty constructor.
     */
    TableStreamReader(){currentState = S_CLOSED;}

    /**
     * The destructor.
     */
    virtual ~TableStreamReader(){;}

    /**
     * Opens a file expected to contain an ASDM table of type T with rows of type R.
     *
     * @param directory the path to the directory containing the file.
     */
    void open(const std::string& directory){
      checkState(T_OPEN, "TableStreamReader::open");
      // Open the file.
      tablePath = directory + "/"+ T::name() + ".bin";
      tableFile.open(tablePath.c_str(), ios::in|ios::binary);
      if (!tableFile.is_open())
	throw asdm::ConversionException("Could not open file " + tablePath, T::name());
      
      // Determine the size of the file.
      struct stat filestatus;
      stat( tablePath.c_str(), &filestatus);
      fileSizeInBytes = filestatus.st_size;
      cout << "fileSizeInBytes=" << fileSizeInBytes << endl;

      // Locate the xmlPartMIMEHeader.
      std::string xmlPartMIMEHeader = "CONTENT-ID: <HEADER.XML>\n\n";
      CharComparator comparator(&tableFile, 10000);
      std::istreambuf_iterator<char> BEGIN(tableFile.rdbuf());
      std::istreambuf_iterator<char> END;
      std::istreambuf_iterator<char> it = std::search(BEGIN, END, xmlPartMIMEHeader.begin(), xmlPartMIMEHeader.end(), comparator);
      if (tableFile.tellg() > 10000) { 
	tableFile.seekg(0);
	xmlPartMIMEHeader = "CONTENT-ID: <HEADER.XML>\r\n\r\n";
	it = BEGIN;
	it = std::search(BEGIN, END, xmlPartMIMEHeader.begin(), xmlPartMIMEHeader.end(), comparator);
	if (tableFile.tellg() > 10000) 
	  throw asdm::ConversionException("failed to detect the beginning of the XML header.", T::name());
      }
      // Locate the binaryPartMIMEHeader while accumulating the characters of the xml header.	
      std::string binPartMIMEHeader = "--MIME_BOUNDARY\nCONTENT-TYPE: BINARY/OCTET-STREAM\nCONTENT-ID: <CONTENT.BIN>\n\n";
      std::string xmlHeader;
      CharCompAccumulator compaccumulator(&xmlHeader, &tableFile, 100000);
      ++it;
       it = std::search(it, END, binPartMIMEHeader.begin(), binPartMIMEHeader.end(), compaccumulator);
      if (it == END) 
	throw asdm::ConversionException("failed to detect the beginning of the binary part", T::name());
      ++it;
      cout << xmlHeader << endl;
      //
      // We have the xmlHeader , let's parse it.
      //
      xmlDoc *doc;
      doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
      cout << "xml header parsed in memory" << endl;
      if ( doc == NULL ) 
	throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Main");
          
      xmlNode* root_element = xmlDocGetRootElement(doc);
      if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
	throw asdm::ConversionException("Failed to parse the xmlHeader into a DOM structure.", T::name());
    
      const ByteOrder* byteOrder;
      if ( std::string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
	// Then it's an "old fashioned" MIME file for tables.
	// Just try to deserialize it with Big_Endian for the bytes ordering.
	byteOrder = asdm::ByteOrder::Big_Endian;       
	attributesSeq = T::defaultAttributesNamesInBin();
      }
      else if (std::string(T::name()+"Table").compare((const char*) root_element->name) == 0) {
	// It's a new (and correct) MIME file for tables.
	//
	// 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
	//
	xmlNode* bulkStoreRef = 0;
	xmlNode* child = root_element->children;
      
	// Skip the two first children (Entity and ContainerEntity).
	bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
	if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (std::string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
	  throw asdm::ConversionException ("Could not find the element '/"+T::name()+"Table/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", T::name());
      	
	// We found BulkStoreRef, now look for its attribute byteOrder.
	_xmlAttr* byteOrderAttr = 0;
	for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	    byteOrderAttr = attr;
	    break;
	  }
      
	if (byteOrderAttr == 0) 
	  throw asdm::ConversionException("Could not find the element '/"+T::name()+"Table/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", T::name());
      
	string byteOrderValue = string((const char*) byteOrderAttr->children->content);
	if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
	  throw asdm::ConversionException("No valid value retrieved for the element '/"+T::name()+"Table/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", T::name());
		
	//
	// 2nd) Look for the Attributes element and grab the names of the elements it contains.
	//
	xmlNode* attributes = bulkStoreRef->next;
	if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
	  throw asdm::ConversionException ("Could not find the element '/"+T::name()+"Table/Attributes'. Invalid XML header '"+ xmlHeader + "'.", T::name());
 
 	xmlNode* childOfAttributes = attributes->children;
 	
 	while ( childOfAttributes != 0 && (childOfAttributes->type == XML_ELEMENT_NODE) ) {
	  attributesSeq.push_back(string((const char*) childOfAttributes->name));
	  childOfAttributes = childOfAttributes->next;
	}
      }
      // Create an EndianIFStream from the substring containing the binary part.
      eifs = asdm::EndianIFStream (&tableFile, byteOrder);
    
      cout << "About to read entity" << endl;
      asdm::Entity entity = Entity::fromBin((EndianIStream &)eifs);
    
      // We do nothing with that but we have to read it.
      cout << "About to read containerEntity" << endl;
      asdm::Entity containerEntity = Entity::fromBin((EndianIStream &)eifs);

      // Let's read numRows but ignore it and rely on the value specified in the ASDM.xml file.    
      int numRows = ((EndianIStream &)eifs).readInt();
      cout << "numRows =" << numRows << endl;
      
      // Memorize the starting point of rows.
      whereRowsStart = tableFile.tellg();

      // Update the state
      currentState = S_OPENED;
    }

    /**
     * Repositions the read head to the beginning of the table.
     * The internal storage containing the result of the last read operation is cleared.
     */
    void reset() {
      checkState(T_RESET, "TableStreamReader::reset");
      clear();
      tableFile.seekg(whereRowsStart);
    }

    /**
     * Reads at most n rows in the file, creates as many memory representations of these rows and return the reference of a vector of pointers on these representations.
     *
     * @param nRows the maximum number of rows to read.
     * @return a constant reference to a vector of pointers on instances of R.
     */
    const std::vector<R*>& nextNRows(unsigned int nRows) {
      checkState(T_READ, "TableStreamReader::nextNRows"); 
      clear();
      unsigned int nread = 0;
      while ( hasRows() && nread < nRows ) {
	rows.push_back(R::fromBin((EndianIStream&) eifs, (T&) asdm.getTable(T::name()), attributesSeq));
	nread++;
      }
      return rows;
    }

    /**
     * Reads as many rows as possible in the file, keeps their in memory representation until the number of read bytes is greater or equal
     * a number of bytes, n,  passed as a parameter to the method
     *
     * @param n the number of read bytes which is used as an upper limit to the read operation.
     * @return a constant reference to a vector of pointers on instances of R.
     */
    
    const std::vector<R*>& untilNBytes(unsigned int nBytes) {
      checkState(T_READ, "TableStreamReader::untilNBytes"); 
      clear();
      off_t whereAmI  = tableFile.tellg();
      if (!hasRows()) return rows;

      do {
	rows.push_back(R::fromBin((EndianIStream&) eifs, (T&) asdm.getTable(T::name()), attributesSeq));
      }
      while (((tableFile.tellg() - whereAmI) < nBytes) && hasRows());
      return rows;
    }

    /**
     * Returns true if the end of the file has not been reached.
     */
    bool hasRows() {
      checkState(T_CHECK, "TableStreamReader::hasRows");
      return tableFile.tellg() < (fileSizeInBytes - 19);
    }

    /**
     * Releases all the resources allocated to the instance which can be reused with another file.
     */
    void close() {
      checkState(T_CLOSE, "TableStreamReader::close"); 
      clear();
      if (tableFile.is_open()) tableFile.close();

      // Update the state.
      currentState = S_CLOSED;
    }
    
  private:
    std::string			tablePath;
    std::ifstream		tableFile;
    off_t                       fileSizeInBytes;
    asdm::EndianIFStream	eifs;
    std::vector<std::string>	attributesSeq;
    asdm::ASDM                  asdm;
    std::vector<R*>		rows;    

    streampos whereRowsStart;

    enum State {S_CLOSED, S_OPENED};
    enum Transition {T_OPEN, T_CHECK, T_RESET, T_READ, T_CLOSE};
    State currentState;

    void checkState(Transition t, const string& methodName) const {
      switch (currentState) {
      case S_CLOSED:
	if (t == T_OPEN) return;

      case S_OPENED:
	if (t == T_CHECK || t == T_RESET || t == T_READ || t == T_CLOSE) return;
      }
      throw ConversionException("Invalid call of method '" + methodName + "' in the current context.", T::name());
    }
    /**
     * Empty the local storage containing the rows obtained during the last read operation.
     */
    void clear() {
      for (unsigned int i = 0; i < rows.size(); i++)
	if (rows[i]) delete rows[i];
      rows.clear();
    }
  };
} // end namespace asdm
#endif
