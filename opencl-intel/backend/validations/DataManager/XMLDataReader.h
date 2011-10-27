/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  XMLDataReader.h

\*****************************************************************************/
#ifndef __XML_DATA_READER_H__
#define __XML_DATA_READER_H__

#include <string>
#include <limits>
#include <fstream>

#define TIXML_USE_STL
#include "tinyxml.h"

#include "llvm/System/DataTypes.h"

#include "IDataReader.h"
#include "IBufferContainerList.h"
#include "XMLDataReadWrite.h"
#include "Exception.h"

namespace Validation
{

    /// @brief IBufferContainerList object reader from data file in XML format
    /// Implements IDataReader interface
    class XMLBufferContainerListReader: public IDataReader
    {
    public:
        /// @brief XMLBufferContainerListReaderctor.
        /// Opens "fileName" data file and reads it into internal data structure
        /// @param fileName - name of file
        /// @throws Exception::InvalidArgument
        XMLBufferContainerListReader(const std::string& fileName): 
          m_fileName(fileName)
        {
            if(fileName.empty())
                throw Exception::InvalidArgument("XMLBufferContainerListReader"
                "::XMLBufferContainerListReader file name is empty");
        }

        /// @brief dtor does nothing
        virtual ~XMLBufferContainerListReader() {}

        /// @brief read data to IBufferContainerList object from XML node
        /// @param [INOUT] - pContainer pointer to object with 
        ///        IBufferContainerList interface
        /// @throws Exception::InvalidArgument, Exception::IOError
        void Read(IContainer * pContainer)
        {
            IBufferContainerList * pBCL = 
              dynamic_cast<IBufferContainerList*>(pContainer);
            if(NULL == pBCL)
                throw Exception::InvalidArgument("XMLBufferContainerListReader"
                "::Read Input object is NULL");

            TiXmlDocument XMLDoc;
            if (!XMLDoc.LoadFile(m_fileName.c_str()))
                throw Exception::IOError("XMLBufferContainerListReader::"
                "XMLBufferContainerListReader cannot open file");


            TiXmlHandle h(&XMLDoc);
            TiXmlElement* pXMLNode = h.FirstChild("ICSCData").ToElement();
            if (NULL == pXMLNode)
                throw Exception::IOError("XMLBufferContainerListReader::"
                "XMLBufferContainerListReader cannot open node ");

            XMLBufferContainerListReadWrite rw;
            rw.ReadWriteBufferContainerList(pBCL, pXMLNode, 
              IXMLReadWriteBase::READ);
        }

    private:
        /// hide copy constructor
        XMLBufferContainerListReader(const XMLBufferContainerListReader& ) : 
           IDataReader() {}
        /// hide assignment operator
        void operator =(const XMLBufferContainerListReader&){}
        /// file name
        const std::string m_fileName;
    };

} // End of Validation namespace

#endif // __XML_DATA_READER_H__

