//# MMapfdIO.h: Memory-mapped IO on a file descriptor
//#
//# Copyright (C) 2009
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
//#  $Id: MMapfdIO.h 20859 2010-02-03 13:14:15Z gervandiepen $

#ifndef CASA_MMAPFDIO_H
#define CASA_MMAPFDIO_H

//# Includes
#include <casa/IO/LargeFiledesIO.h>
#include <casa/OS/RegularFile.h>
#include <set>

namespace casa
{

// <summary>
// Memory-mapped IO on a file.
// </summary>

// <synopsis>
// Memory-mapped IO lets the OS take care of caching file segments.
// This is particularly useful for the Tiled Storage Manager which keeps a
// cache of tiles. When using memory-mapped IO it does not need to do that
// anymore.
//
// Best performance on Linux and Mac was achieved when mapping only
// the part of the file that is currently in use, but at least a couple
// of MB. Thus, using memory mapping with > 4 GB files which do not
// fit in the virtual memory address space on 32-bit system, is a non-issue,
// because only a small part of the file is mapped at any time.
//
// In the general case there is direct access to the mapped file space.
// The read and write methods copies the data into/from a buffer.
// However, to avoid the copying it is possible to get a direct pointer
// to the mapped data. This should be used with care, because writing to
// it will cause a segmentation if the file is readonly. If the file is
// writable, writing into the mapped data segment means changing the file
// contents.
// </synopsis>
//


class MMapfdIO: public LargeFiledesIO
{
public:
  // Default constructor.
  // A file can be memory-mapped using the map function.
  MMapfdIO();

  // Map the beginning of the given file descriptor into memory with 
  // read access. The map has also write access if the file is opened for
  // write. The file name is only used in possible error messages.
  MMapfdIO (int fd, const String& fileName);

  // Destructor.
  // If needed, it will flush and unmap the file, but not close it.
  ~MMapfdIO();

  // Map the beginning of the given file descriptor into memory with read
  // access. The map has also write access if the file is opened for write.
  // An exception is thrown if a file descriptor was already attached.
  // The file name is only used in possible error messages.
  void map (int fd, const String& fileName);

  // Map or remap the file. Only the required section of the file,
  // as given by the offset and length parameters, is mapped.
  // Remapping is needed if the file has grown elsewhere.
  void mapFile(Int64 offset, Int64 length);

  // Flush changed mapped data to the file.
  // Nothing is done if the file is readonly.
  void flush();

  // Write the number of bytes from the seek position on.
  // The file will be extended and remapped if writing beyond end-of-file,
  // or if the currently mapped section does not include the required
  // section. In that case possible pointers obtained using
  // <src>getXXPointer</src> are not valid anymore.
  virtual void write (uInt size, const void* buf);

  // Read <src>size</src> bytes from the File. Returns the number of bytes
  // actually read. Will throw an exception (AipsError) if the requested
  // number of bytes could not be read unless throwException is set to
  // False. Will always throw an exception if the file is not readable or
  // the system call returns an undocumented value. The file will be
  // remapped if the currently mapped section does not include the
  // required section. In that case, possible pointers obtained using
  // <src>getXXPointer</src> are not valid anymore.
  virtual Int read (uInt size, void* buf, Bool throwException=True);

  // Get a read or write pointer to the given position in the mapped file.
  // An exception is thrown if beyond end-of-file or it not writable.
  // These functions should be used with care. If the pointer is used to
  // access data beyond the file size, a segmentation fault will occur.
  // So it means that the write pointer can only be used to update the file,
  // not to extend it. The <src>seek</src> and <src>write</src> functions
  // should be used to extend a file.
  // The length parameter is the required length of validity of the returned
  // buffer. The file will be remapped if the currently mapped section does
  // not include the required section. In that case possible pointers
  // obtained from previous calls to <src>getXXPointer</src> are not valid
  // anymore.
  // <group>
  const void* getReadPointer (Int64 offset, Int64 length) const;
  void* getWritePointer (Int64 offset, Int64 length);
  // </group>

  // Get the file size.
  Int64 getFileSize() const
    { return itsFileSize; }

protected:
  // Reset the position pointer to the given value. It returns the
  // new position.
  virtual Int64 doSeek (Int64 offset, ByteIO::SeekOption);

  // Unmap the file.
  void unmapFile();

private:
  // Forbid copy constructor and assignment
  // <group>
  MMapfdIO (const MMapfdIO&);
  MMapfdIO& operator= (const MMapfdIO&);
  // </group>

  Int64  itsFileSize;       //# File size
  Int64  itsMapOffset;      //# Start of memory map in file
  Int64  itsMapSize;        //# Size of currently memory mapped region
  Int64  itsPosition;       //# Current seek position
  char*  itsPtr;            //# Pointer to memory map
  Bool   itsIsWritable;

  // Mapped regions of the file which have not been unmapped
  std::set<std::pair<char *, Int64> > itsMappedRegions;
};

} // end namespace

#endif
