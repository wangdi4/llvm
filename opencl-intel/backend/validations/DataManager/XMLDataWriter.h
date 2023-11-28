// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __XML_DATA_WRITER_H__
#define __XML_DATA_WRITER_H__

#include "Exception.h"
#include "IBufferContainerList.h"
#include "IDataWriter.h"
#include "XMLDataReadWrite.h"
#include "tinyxml_wrapper.h"
#include "llvm/Support/DataTypes.h"
#include <limits>
#include <sstream>
#include <string>

namespace Validation {
/// @brief IBufferContainerList object writer to data file in XML format
/// Implements IDataReader interface
class XMLBufferContainerListWriter : public IDataWriter {
public:
  /// @brief XMLBufferContainerListReaderctor.
  /// Opens "fileName" data file and reads it into internal data structure
  /// @param [in] - fileName name of file to write to
  XMLBufferContainerListWriter(const std::string &fileName)
      : m_fileName(fileName) {
    if (fileName.empty())
      throw Exception::InvalidArgument(
          "XMLBufferContainerListWriter"
          "::XMLBufferContainerListWriter file name is empty");
  }
  /// dtor
  virtual ~XMLBufferContainerListWriter() {}

  XMLBufferContainerListWriter(const XMLBufferContainerListWriter &) = delete;
  XMLBufferContainerListWriter &
  operator=(const XMLBufferContainerListWriter &) = delete;

  /// @brief write data from IBufferContainerList object to XML node
  /// @param [IN] - pContainer pointer to object with IBufferContainerList
  ///        interface
  /// @throws Exception::InvalidArgument, Exception::IOError
  virtual void Write(const IContainer *pContainer) override {
    IBufferContainerList *pBCL = const_cast<IBufferContainerList *>(
        static_cast<const IBufferContainerList *>(pContainer));
    if (NULL == pBCL)
      throw Exception::InvalidArgument("XMLBufferContainerListWriter"
                                       "::Write() Input object is NULL");

    TiXmlDocument XMLDoc;

    // create declaration for XML file
    XMLDoc.LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", ""));

    // Create ICSCData node
    TiXmlElement *pXMLNode = new TiXmlElement("ICSCData");
    XMLDoc.LinkEndChild(pXMLNode);

    XMLBufferContainerListReadWrite rw;
    rw.ReadWriteBufferContainerList(pBCL, pXMLNode, IXMLReadWriteBase::WRITE);

    XMLDoc.SaveFile(m_fileName);
  };

private:
  /// file name to write to
  const std::string m_fileName;
};

} // namespace Validation

#endif // __XML_DATA_WRITER_H__
