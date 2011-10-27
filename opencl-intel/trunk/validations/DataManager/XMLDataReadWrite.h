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

File Name:  XMLDataReadWrite.h

\*****************************************************************************/
#ifndef __XML_DATAREADWRITE_H__
#define __XML_DATAREADWRITE_H__

#include <string>
#include <limits>
#include <fstream>

#define TIXML_USE_STL
#include "tinyxml.h"

#include "llvm/System/DataTypes.h"

#include "IDataReader.h"
#include "IDataWriter.h"

#include "IBufferContainer.h"
#include "IMemoryObject.h"

namespace Validation
{
    /// @brief base interface for classes which perform read/write operations
    /// of IContainer objects to/from XML file using TinyXML library
    class IXMLReadWriteBase
    {
    public:
        /// @brief specifier of read/write operation
        enum  RWOperationType
        {
            READ = 0,
            WRITE = 1
        };

        /// @brief Performs read/write operation for object with IContainer interface to XML node
        /// @param [inout]	pContainer - interface to object to be read/written
        /// @param [inout]	pXML -  pointer to XML node containing object
        /// @param [in]	    rwtype - read or write operation
        /// @return		    pointer to IContainer interface of resulting object
        virtual IContainer* ReadWrite(IContainer* pContainer, TiXmlElement* pXml, const RWOperationType rwtype) = 0;
    };

    /// @brief helper structure to travel along sibling XML nodes
    struct AddGetXMLNode
    {
    public:
        /// @brief ctor
        /// @param [in]     pXml - pointer to parent XML node
        /// @param [in]     str  - name of child node
        /// @param [in]	    rwtype - read or write operation
        AddGetXMLNode(TiXmlElement * pXml, const std::string& str, IXMLReadWriteBase::RWOperationType rwtype)
            : m_parentXML(pXml), m_str(str), m_rwtype(rwtype), m_CurXML(NULL) {}

        /// @brief Get next sibling XML node.
        /// Depending on read/write operation function either creates or travels along specified sibling nodes
        /// @return         next sibling XML node
        TiXmlElement * GetNext()
        {
            if(IXMLReadWriteBase::WRITE == m_rwtype)
            {
                m_CurXML = new TiXmlElement(m_str);
                m_parentXML->LinkEndChild(m_CurXML);
            }
            else
            {
                if(m_CurXML == NULL)
                {
                    m_CurXML = m_parentXML->FirstChildElement(m_str);
                }
                else
                {
                    m_CurXML = m_CurXML->NextSiblingElement(m_str);
                }

                if(NULL == m_CurXML)
                    throw Exception::IOError("XML node " + m_str + " not found");
            }
            return m_CurXML;
        }
    private:
        /// parent XML node
        TiXmlElement * m_parentXML;
        /// node name
        std::string m_str;
        /// type of read/write operation
        IXMLReadWriteBase::RWOperationType m_rwtype;
        /// current XML node
        TiXmlElement * m_CurXML;
    };


    /// @brief Reader/writer from data file in XML format
    class XMLBufferContainerListReadWrite:
        private IXMLReadWriteBase
    {
    public:
        /// @brief XMLBufferContainerListReaderctor.
        /// Opens "fileName" data file and reads it into internal data structure.
        XMLBufferContainerListReadWrite(){}

        /// @brief dtor does nothing
        virtual ~XMLBufferContainerListReadWrite() {}
        /// const defining floating point representation precision of stringstream used for saving FP data
        static const int32_t StreamPrecision;
        /// version of XML node. To distinguish between different version of writers/readers
        static const std::string XMLFileFormatVersion;
        /// @brief read/write data to IContainer
        /// @param [inout] - pContainer object supporting IBufferContainerList interface
        /// @param [inout] - pXml pointer to XML node to read/write from
        /// @param [in]	    rwtype - read or write operation
        /// @throws Exception::InvalidArgument, Exception::IOError
        /// @return IBufferContainerList  interface to resulting object
        virtual IContainer* ReadWriteBufferContainerList(IContainer *pContainer, TiXmlElement* pXml, const RWOperationType rwtype);

    private:
        /// @brief read data to IContainer. Implementation of IXMLReadWriteBase::ReadWrite interface
        virtual IContainer* ReadWrite(IContainer *, TiXmlElement* , const RWOperationType );
        /// hide copy constructor
        XMLBufferContainerListReadWrite(const XMLBufferContainerListReadWrite& ) : IXMLReadWriteBase() {}
        /// hide assignment operator
        void operator =(const XMLBufferContainerListReadWrite&){}

    };

}
#endif // __XML_DATAREADWRITE_H__
