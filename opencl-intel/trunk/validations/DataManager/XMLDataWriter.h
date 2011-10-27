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

File Name:  XMLDataWriter.h

\*****************************************************************************/
#ifndef __XML_DATA_WRITER_H__
#define __XML_DATA_WRITER_H__

#include <string>
#include <sstream>
#include <limits>
#include "llvm/System/DataTypes.h"

#define TIXML_USE_STL
#include "tinyxml.h"


#include "Exception.h"
#include "IDataWriter.h"
#include "IBufferContainerList.h"
#include "XMLDataReadWrite.h"


namespace Validation
{
    /// @brief IBufferContainerList object writer to data file in XML format
    /// Implements IDataReader interface
    class XMLBufferContainerListWriter : public IDataWriter
    {
    public:
        /// @brief XMLBufferContainerListReaderctor.
        /// Opens "fileName" data file and reads it into internal data structure
        /// @param [in] - fileName name of file to write to
        XMLBufferContainerListWriter(const std::string& fileName): 
          m_fileName(fileName)
        {
            if(fileName.empty())
                throw Exception::InvalidArgument("XMLBufferContainerListWriter"
                "::XMLBufferContainerListWriter file name is empty");
        }
        /// dtor
        virtual ~XMLBufferContainerListWriter(){}

        /// @brief write data from IBufferContainerList object to XML node
        /// @param [IN] - pContainer pointer to object with IBufferContainerList
        ///        interface
        /// @throws Exception::InvalidArgument, Exception::IOError
        virtual void Write(const IContainer * pContainer)
        {
            IBufferContainerList * pBCL = const_cast<IBufferContainerList *>
                (dynamic_cast<const IBufferContainerList*>(pContainer));
            if(NULL == pBCL)
                throw Exception::InvalidArgument("XMLBufferContainerListWriter"
                "::Write() Input object is NULL");

            TiXmlDocument XMLDoc;

            // create declaration for XML file
            XMLDoc.LinkEndChild( new TiXmlDeclaration("1.0", "UTF-8", ""));

            // Create ICSCData node
            TiXmlElement * pXMLNode = new TiXmlElement( "ICSCData" );
            XMLDoc.LinkEndChild(pXMLNode);

            XMLBufferContainerListReadWrite rw;
            rw.ReadWriteBufferContainerList(pBCL, pXMLNode, 
              IXMLReadWriteBase::WRITE);

            XMLDoc.SaveFile(m_fileName);
        };

    private:
        /// hide copy constructor
        XMLBufferContainerListWriter(const XMLBufferContainerListWriter& ) : 
           IDataWriter() {}
        /// hide assignment operator
        void operator =(const XMLBufferContainerListWriter&){}

    private:
        /// file name to write to
        const std::string m_fileName;
    };

} // End of Validation namespace

#endif // __XML_DATA_WRITER_H__

