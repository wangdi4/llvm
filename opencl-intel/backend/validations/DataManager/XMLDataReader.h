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

#ifndef __XML_DATA_READER_H__
#define __XML_DATA_READER_H__

#include "Exception.h"
#include "IBufferContainerList.h"
#include "IDataReader.h"
#include "XMLDataReadWrite.h"
#include "tinyxml_wrapper.h"
#include "llvm/Support/DataTypes.h"
#include <fstream>
#include <limits>
#include <string>

namespace Validation {

/// @brief IBufferContainerList object reader from data file in XML format
/// Implements IDataReader interface
class XMLBufferContainerListReader : public IDataReader {
public:
  /// @brief XMLBufferContainerListReaderctor.
  /// Opens "fileName" data file and reads it into internal data structure
  /// @param fileName - name of file
  /// @throws Exception::InvalidArgument
  XMLBufferContainerListReader(const std::string &fileName)
      : m_fileName(fileName) {
    if (fileName.empty())
      throw Exception::InvalidArgument(
          "XMLBufferContainerListReader"
          "::XMLBufferContainerListReader file name is empty");
  }

  /// @brief dtor does nothing
  virtual ~XMLBufferContainerListReader() {}

  XMLBufferContainerListReader(const XMLBufferContainerListReader &) = delete;
  XMLBufferContainerListReader &
  operator=(const XMLBufferContainerListReader &) = delete;

  /// @brief read data to IBufferContainerList object from XML node
  /// @param [INOUT] - pContainer pointer to object with
  ///        IBufferContainerList interface
  /// @throws Exception::InvalidArgument, Exception::IOError
  void Read(IContainer *pContainer) override {
    IBufferContainerList *pBCL =
        static_cast<IBufferContainerList *>(pContainer);
    if (NULL == pBCL)
      throw Exception::InvalidArgument("XMLBufferContainerListReader"
                                       "::Read Input object is NULL");

    TiXmlDocument XMLDoc;
    if (!XMLDoc.LoadFile(m_fileName.c_str())) {
      std::stringstream ss;
      ss << "at line " << XMLDoc.ErrorRow();
      ss << ", column " << XMLDoc.ErrorCol();
      ss << ", " << XMLDoc.ErrorDesc() << std::endl;
      throw Exception::IOError(ss.str());
    }

    TiXmlHandle h(&XMLDoc);
    TiXmlElement *pXMLNode = h.FirstChild("ICSCData").ToElement();
    if (NULL == pXMLNode)
      throw Exception::IOError(
          "XMLBufferContainerListReader::"
          "XMLBufferContainerListReader cannot open node ");

    XMLBufferContainerListReadWrite rw;
    rw.ReadWriteBufferContainerList(pBCL, pXMLNode, IXMLReadWriteBase::READ);
  }

private:
  /// file name
  const std::string m_fileName;
};

} // namespace Validation

#endif // __XML_DATA_READER_H__
