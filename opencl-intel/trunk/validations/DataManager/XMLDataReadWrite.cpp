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

File Name:  XMLDataReadWrite.cpp

\*****************************************************************************/
#include <string>
#include <iostream>

#include "Exception.h"
#include "XMLDataReader.h"

#include "BufferContainerList.h"
#include "BufferContainer.h"
#include "Buffer.h"
#include "Image.h"
#include "NEATValue.h"
#include "FloatOperations.h"

using namespace std;

namespace Validation
{
    /// XML node version for XMLBufferContainerListReadWrite object
    const std::string XMLBufferContainerListReadWrite::XMLFileFormatVersion = "0.1";

    /// set number precision for the reading stream
    const int32_t XMLBufferContainerListReadWrite::StreamPrecision = 17;  

    /// @brief Reads/writes typed value from string stream.
    template<typename T>
    inline void ReadWriteValueToStr(std::stringstream& isrt, T *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        if(IXMLReadWriteBase::READ == rwtype)
        {
            isrt >> *data ;
        }
        else
        {
            isrt << *data << " ";
        }
    }

    template<typename T>
    inline void ReadWriteValueToStr8_t(std::stringstream& isrt, T *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        if(IXMLReadWriteBase::READ == rwtype)
        {
            int32_t _t;
            isrt >> _t;
            *data = (T) _t;
        }
        else
        {
            isrt << (int32_t) *data << " ";
        }
    }

    template<>
    inline void ReadWriteValueToStr<int8_t>(std::stringstream& isrt, int8_t *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        ReadWriteValueToStr8_t<int8_t>(isrt, data, rwtype);
    }

    template<>
    inline void ReadWriteValueToStr<uint8_t>(std::stringstream& isrt, uint8_t *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        ReadWriteValueToStr8_t<uint8_t>(isrt, data, rwtype);
    }

    template<typename T>
    inline void ReadWriteValueToStrFloat(std::stringstream& isrt, T *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        if(IXMLReadWriteBase::READ == rwtype)
        {
            // Read value to the string.
            std::string value;
            isrt >> value;

            // Check if "value" is special.
            if ("INF" == value)
            {
                *data = std::numeric_limits<T>::infinity();
            }
            else if ("-INF" == value)
            {
                *data = -std::numeric_limits<T>::infinity();
            }
            else if ("NaN" == value)
            {
                *data  = std::numeric_limits<T>::quiet_NaN();
            }
            else
            {
                // Usual floating point value. Treat it as usual.
                std::stringstream tmp(value);
                tmp >> *data;
            }
        }
        else
        {   // read branch
            if (Utils::IsPInf(*data))                      isrt << "INF";
            else if (Utils::IsNInf(*data))                 isrt << "-INF";
            else if (Utils::IsNaN(*data))                  isrt << "NaN";
            // Otherwise treat it as usual.
            else isrt << *data;
            // add space
            isrt << ' ';
        }
    }

    template<>
    inline void ReadWriteValueToStr<float>(std::stringstream& isrt, float *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        ReadWriteValueToStrFloat<float>(isrt, data, rwtype);
    }

    template<>
    inline void ReadWriteValueToStr<double>(std::stringstream& isrt, double *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        ReadWriteValueToStrFloat<double>(isrt, data, rwtype);
    }

    template<>
    inline void ReadWriteValueToStr<CFloat16>(std::stringstream& isrt, CFloat16 *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        ReadWriteValueToStrFloat<CFloat16>(isrt, data, rwtype);
    }
    ///@brief helper template function to read/write attribute of XML node
    /// @param el   element to be written/read as attribute of XML node.
    /// @param xml  XML node.
    /// @param name Attribute name.
    /// @param rwtype   Operation type: read or write.
    template<typename T>
    inline void XMLReadWriteAttr(T* el, TiXmlElement * xml, const std::string& name, const IXMLReadWriteBase::RWOperationType rwtype)
    {
        if( IXMLReadWriteBase::READ == rwtype )
        {
            if(TIXML_SUCCESS != xml->QueryValueAttribute(name, el))
                throw Exception::IOError("Error reading attribute with name " + name);
        }
        else if( IXMLReadWriteBase::WRITE == rwtype )
        {   // WRITE
            xml->SetAttribute(name, *el);
        }
    }

    /// @brief Write/read type description from/to XML node as attribute.
    template<>
    inline void XMLReadWriteAttr<TypeDesc>(TypeDesc* el, TiXmlElement * xml, const std::string& pref, const IXMLReadWriteBase::RWOperationType rwtype)
    {
        std::string typeOfElement = (IXMLReadWriteBase::READ == rwtype) ? "" : el->TypeToString();
        XMLReadWriteAttr(&typeOfElement, xml, pref+"type", rwtype);

        std::size_t sizeOfElement = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->GetSizeInBytes();
        XMLReadWriteAttr((int32_t*)&sizeOfElement, xml, pref+"type_size_in_bytes", rwtype);

        if(IXMLReadWriteBase::READ == rwtype)
        {
            el->SetType(TypeValWrapper::ValueOf(typeOfElement));
            el->SetTypeAllocSize(sizeOfElement);
        }

        if (TypeValWrapper(TypeValWrapper::ValueOf(typeOfElement)).IsAggregate() ||
            TypeValWrapper(TypeValWrapper::ValueOf(typeOfElement)).IsPointer())
        {
            uint64_t numOfSubElements = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->GetNumberOfElements();
            XMLReadWriteAttr(&numOfSubElements, xml, typeOfElement+"Width", rwtype);

            uint64_t numOfSubTypes = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->GetNumOfSubTypes();
            XMLReadWriteAttr(&numOfSubTypes, xml, typeOfElement+"_number_of_subtypes", rwtype);

            TypeDesc subElement = (IXMLReadWriteBase::READ == rwtype) ? TypeDesc() : el->GetSubTypeDesc(0);
            XMLReadWriteAttr(&subElement, xml, typeOfElement+"0", rwtype);

            if(IXMLReadWriteBase::READ == rwtype)
            {
                el->SetNumberOfElements(numOfSubElements);
                el->SetNumOfSubTypes(numOfSubTypes);
                el->SetSubTypeDesc(0, subElement);
            }
            if (TypeValWrapper(TypeValWrapper::ValueOf(typeOfElement)).IsStruct())
            {
                uint64_t offset = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->GetSubTypeDesc(0).GetOffsetInStruct();
                XMLReadWriteAttr(&offset, xml, typeOfElement+"0_offset", rwtype);

                if(IXMLReadWriteBase::READ == rwtype)
                {
                    subElement.SetOffsetInStruct(offset);
                    el->SetSubTypeDesc(0, subElement);
                }
                for (uint64_t i = 1; i < numOfSubTypes; ++i)
                {
                    subElement = (IXMLReadWriteBase::READ == rwtype) ? TypeDesc() : el->GetSubTypeDesc(i);
                    ostringstream oss;
                    oss << i;
                    XMLReadWriteAttr(&subElement, xml, typeOfElement+"_sub"+oss.str()+"_", rwtype);

                    offset = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->GetSubTypeDesc(i).GetOffsetInStruct();
                    XMLReadWriteAttr(&offset, xml, typeOfElement+"_element"+oss.str()+"_offset", rwtype);
                    if(IXMLReadWriteBase::READ == rwtype)
                    {
                        subElement.SetOffsetInStruct(offset);
                        el->SetSubTypeDesc(i, subElement);
                    }
                }
            }
        }
    }

    /// @brief Write/read buffer description from/to XML node as attribute.
    template<>
    inline void XMLReadWriteAttr<BufferDesc>(BufferDesc* el, TiXmlElement * xml, const std::string&, const IXMLReadWriteBase::RWOperationType rwtype)
    {
        std::size_t numOfElements = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->NumOfElements();
        TypeDesc elemDesc = (IXMLReadWriteBase::READ == rwtype) ? TypeDesc() : el->GetElementDescription();
        bool isNEAT = (IXMLReadWriteBase::READ == rwtype) ? false : el->IsNEAT();
        XMLReadWriteAttr((int32_t*)&numOfElements, xml, "length", rwtype);
        XMLReadWriteAttr(&elemDesc, xml, "", rwtype);
        XMLReadWriteAttr(&isNEAT, xml, "neat", rwtype);

        if(IXMLReadWriteBase::READ == rwtype)
        {
            el->SetNumOfElements(numOfElements);
            el->SetElementDecs(elemDesc);
            el->SetNeat(isNEAT);
        }
    }

    /// @brief Write/read image description from/to XML node as attribute.
    template<>
    inline void XMLReadWriteAttr<ImageDesc>(ImageDesc* el, TiXmlElement * xml, const std::string&, const IXMLReadWriteBase::RWOperationType rwtype)
    {
        size_t numOfDimensions = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->GetNumOfDimensions();
        ImageSizes imageSizes = (IXMLReadWriteBase::READ == rwtype) ? ImageSizes() : el->GetSizes();
        string dataTypeStr = (IXMLReadWriteBase::READ == rwtype) ? "" : el->DataTypeToString();
        string orderStr = (IXMLReadWriteBase::READ == rwtype) ? "" : el->OrderToString();
        size_t pixelSize = (IXMLReadWriteBase::READ == rwtype) ? 0 : el->GetElementSize();
        bool isNEAT = (IXMLReadWriteBase::READ == rwtype) ? false : el->IsNEAT();
        XMLReadWriteAttr((size_t*)&numOfDimensions, xml, "dimensions", rwtype);
        XMLReadWriteAttr((int64_t*)&imageSizes.width, xml, "width", rwtype);
        XMLReadWriteAttr((int64_t*)&imageSizes.height, xml, "height", rwtype);
        XMLReadWriteAttr((int64_t*)&imageSizes.depth, xml, "depth", rwtype);
        XMLReadWriteAttr((int64_t*)&imageSizes.row, xml, "row_size", rwtype);
        XMLReadWriteAttr((int64_t*)&imageSizes.slice, xml, "slice_size", rwtype);
        XMLReadWriteAttr(&dataTypeStr, xml, "type", rwtype);
        XMLReadWriteAttr(&orderStr, xml, "order", rwtype);
        XMLReadWriteAttr((size_t*)&pixelSize, xml, "element_size", rwtype);
        XMLReadWriteAttr(&isNEAT, xml, "neat", rwtype);

        if(IXMLReadWriteBase::READ == rwtype)
        {
            *el = ImageDesc(numOfDimensions, 
                            imageSizes, 
                            ImageChannelDataTypeValWrapper::ValueOf(dataTypeStr),
                            ImageChannelOrderValWrapper::ValueOf(orderStr),
                            isNEAT);
            
        }
    }

    /// template function to read/write buffer as sequence of vector.
    /// @param  typename T is dataType of Buffer Elements
    template<typename T>
    inline void ReadWriteBufferVector(IMemoryObject * pB, TiXmlElement *pXml, IXMLReadWriteBase::RWOperationType rwtype)
    {
        BufferDesc desc = GetBufferDescription(pB->GetMemoryObjectDesc());
        BufferAccessor<T> ba(*pB);

        AddGetXMLNode addgetVecNode(pXml, "Vector", rwtype);

        // loop over vectors
        for(uint32_t cntVec = 0; cntVec < desc.NumOfElements(); ++cntVec)
        {
            TiXmlElement * xmlVec = addgetVecNode.GetNext();

            int32_t vecInd = (int32_t) cntVec;
            XMLReadWriteAttr(&vecInd, xmlVec, "id", rwtype);

            if(vecInd != (int32_t) cntVec)
                throw Exception::IOError("Bad ID in Vector node");


            std::stringstream VecStr;
            // set stream precision
            VecStr.precision(XMLBufferContainerListReadWrite::StreamPrecision);

            if (IXMLReadWriteBase::READ == rwtype)  VecStr.str(xmlVec->GetText());

            if (desc.GetElementDescription().IsAggregate() && !desc.GetElementDescription().IsStruct()) {
                for(int32_t cntElem = 0; cntElem < (int32_t) desc.SizeOfVector(); ++cntElem)
                {
                    ReadWriteValueToStr<T>(VecStr, &ba.GetElem(cntVec, cntElem), rwtype);
                }
            } else {
                ReadWriteValueToStr<T>(VecStr, &ba.GetElem(cntVec, 0), rwtype);
            }

            if(IXMLReadWriteBase::WRITE == rwtype) xmlVec->LinkEndChild(new TiXmlText(VecStr.str().c_str()));

        }
    }

    /// template function to read/write buffer as sequence of vector.
    /// @param  typename T is dataType of Buffer Elements
    inline void ReadWriteImageBinary(IMemoryObject * pB, TiXmlElement *pXml, IXMLReadWriteBase::RWOperationType rwtype)
    {
        ImageDesc desc = GetImageDescription(pB->GetMemoryObjectDesc());
        size_t imageSize = desc.GetImageSizeInBytes();

        std::stringstream imageSrt;
        if (IXMLReadWriteBase::READ == rwtype) imageSrt.str(pXml->GetText());

        uint8_t *pData = reinterpret_cast<uint8_t*>(pB->GetDataPtr());
        for (size_t i = 0; i < imageSize; ++i)
        {
            ReadWriteValueToStr(imageSrt, &pData[i], rwtype);
        }
        if(IXMLReadWriteBase::WRITE == rwtype) pXml->LinkEndChild(new TiXmlText(imageSrt.str().c_str()));
    }

    template<typename T>
    inline void ReadWriteNEATValueToStr(std::stringstream& isrt, NEATValue *data, IXMLReadWriteBase::RWOperationType rwtype)
    {
        if( IXMLReadWriteBase::READ == rwtype )
        {
            std::string StatusStr; 
            isrt >> StatusStr ;
            if("ACCURATE" == StatusStr) 
            {
                T val;
                data->SetStatus(NEATValue::ACCURATE);
                ReadWriteValueToStrFloat<T>(isrt, &val, IXMLReadWriteBase::READ);
                data->SetAccurateVal<T>(val);
            }
            else if("UNKNOWN" == StatusStr )
            {
                data->SetStatus( NEATValue::UNKNOWN ); 
            }
            else if("UNWRITTEN" == StatusStr )
            {
                data->SetStatus( NEATValue::UNWRITTEN );
            }
            else if("ANY" == StatusStr )
            {
                data->SetStatus( NEATValue::ANY );      
            }
            else if( "INTERVAL" == StatusStr )
            {
                T minval, maxval;
                data->SetStatus( NEATValue::INTERVAL );      
                ReadWriteValueToStrFloat<T>(isrt, &minval, IXMLReadWriteBase::READ);
                ReadWriteValueToStrFloat<T>(isrt, &maxval, IXMLReadWriteBase::READ);
                data->SetIntervalVal<T>(minval, maxval);
            }
            else
            {
                throw Exception::IOError("Invalid input NEAT state variable");
            }
        }
        else
        {
            switch (data->GetStatus()) {
                    case NEATValue::ACCURATE:
                        {
                            isrt << "ACCURATE" << " ";
                            T* val = const_cast<T*>(data->GetAcc<T>());
                            ReadWriteValueToStrFloat<T>(isrt, val, IXMLReadWriteBase::WRITE);
                            break;
                        }
                    case NEATValue::UNKNOWN:
                        isrt << "UNKNOWN" << " ";
                        break;
                    case NEATValue::UNWRITTEN:
                        isrt << "UNWRITTEN" << " ";
                        break;
                    case NEATValue::ANY:
                        isrt << "ANY" << " ";
                        break;
                    case NEATValue::INTERVAL:
                        {
                            isrt << "INTERVAL" << " "; 
                            T* min = const_cast<T*>(data->GetMin<T>());
                            T* max = const_cast<T*>(data->GetMax<T>());
                            ReadWriteValueToStrFloat<T>(isrt, min, IXMLReadWriteBase::WRITE);
                            ReadWriteValueToStrFloat<T>(isrt, max, IXMLReadWriteBase::WRITE);
                            break;
                        }
                    default:
                      throw Exception::InvalidArgument("XMLDataReadWrite::ReadWriteNEATValueToStr: Unsupported status of NEAT value");
            }
        }
    }

    /// template function to read/write NEAT buffer
    /// @param  typename T is dataType of Buffer Elements
    template<typename T>
    inline void ReadWriteBufferVectorNEAT(IMemoryObject * pB, TiXmlElement *pXml, IXMLReadWriteBase::RWOperationType rwtype)
    {
        BufferDesc desc = GetBufferDescription(pB->GetMemoryObjectDesc());
        BufferAccessor<NEATValue> ba(*pB);

        AddGetXMLNode addgetVecNode(pXml, "Vector", rwtype);

        // loop over vectors
        for(uint32_t cntVec = 0; cntVec < desc.NumOfElements(); ++cntVec)
        {
            TiXmlElement * xmlVec = addgetVecNode.GetNext();

            int32_t vecInd = (int32_t) cntVec;
            XMLReadWriteAttr(&vecInd, xmlVec, "id", rwtype);

            if(vecInd != (int32_t) cntVec)
                throw Exception::IOError("Bad ID in Vector node");

            std::stringstream VecStr;
            // set stream precision
            VecStr.precision(XMLBufferContainerListReadWrite::StreamPrecision);

            if (IXMLReadWriteBase::READ == rwtype)  VecStr.str(xmlVec->GetText());

            if (desc.GetElementDescription().IsAggregate() && !desc.GetElementDescription().IsStruct()) {
                for(int32_t cntElem = 0; cntElem < (int32_t) desc.SizeOfVector(); ++cntElem)
                {
                    ReadWriteNEATValueToStr<T>(VecStr, &ba.GetElem(cntVec, cntElem), rwtype);
                }
            } else {
                ReadWriteNEATValueToStr<T>(VecStr, &ba.GetElem(cntVec, 0), rwtype);
            }

            if(IXMLReadWriteBase::WRITE == rwtype) xmlVec->LinkEndChild(new TiXmlText(VecStr.str().c_str()));

        }
    }

    class XMLImageReadWrite
        : public IXMLReadWriteBase
    {
    public:
        XMLImageReadWrite(IBufferContainer *pBC)
            : m_pBC(pBC)
        {
            if (NULL == m_pBC)
                throw Exception::InvalidArgument("XMLImageReadWrite::XMLImageReadWrite constructor NULL input");
        }
        virtual IContainer* ReadWrite (IContainer* pContainer, TiXmlElement* pXml, const RWOperationType rwtype)
        {
            IMemoryObject* pIm = static_cast<Image*> (pContainer);

            ImageDesc desc;

            if(WRITE == rwtype) desc = GetImageDescription(pIm->GetMemoryObjectDesc());
            XMLReadWriteAttr(&desc, pXml, "ImageDesc", rwtype);

            // Once buffer description is filled buffer is created.
            pIm = (READ == rwtype) ? m_pBC->CreateImage(desc) : pIm;

            if(false == desc.IsNEAT())
            {  // not NEAT case
                ReadWriteImageBinary(pIm, pXml, rwtype);
            }
            else
            {   // NEAT
                throw Exception::NotImplemented("XMLImageReadWrite::ReadWrite "
                    "NEAT image read/write operations has not implemented yet.");
            }
            return pIm;
        }
    private:
        IBufferContainer * m_pBC;
    };

    /// class for read/write Buffer
    class XMLBufferReadWrite
        : public IXMLReadWriteBase
    {
    public:
        /// @brief ctor takes input parent IBufferContainer object
        /// It is passed in as argument since it is needed to create IMemoryObject objects
        XMLBufferReadWrite(IBufferContainer *pBC)
            : m_pBC(pBC)
        {
            if(NULL == m_pBC)
                throw Exception::InvalidArgument("XMLBufferReadWrite::XMLBufferReadWrite constructor NULL input");
        }

        /// @brief read write Buffer data to IContainer
        virtual IContainer* ReadWrite(IContainer* pContainer, TiXmlElement* pXml , const RWOperationType rwtype)
        {
            IMemoryObject * pB = static_cast<IMemoryObject*>(pContainer);

            BufferDesc desc;

            if(WRITE == rwtype) desc = GetBufferDescription(pB->GetMemoryObjectDesc());
            XMLReadWriteAttr(&desc, pXml, "BufferDesc", rwtype);

            // Once buffer description is filled buffer is created.
            pB = (READ == rwtype) ? m_pBC->CreateBuffer(desc) : pB;

            TypeVal elemType = (desc.GetElementDescription().IsAggregate() && !desc.GetElementDescription().IsStruct()) ? 
                desc.GetElementDescription().GetSubTypeDesc(0).GetType() :
                desc.GetElementDescription().GetType();

            if(false == desc.IsNEAT())
            {  // not NEAT case
#define CASE_READWRITEBUFFER(_str, _type) case(_str): ReadWriteBufferVector<_type>(pB, pXml, rwtype); break;

                switch(elemType)
                {
                    CASE_READWRITEBUFFER(TCHAR,    int8_t);
                    CASE_READWRITEBUFFER(TUCHAR,    uint8_t);
                    CASE_READWRITEBUFFER(TSHORT,   int16_t);
                    CASE_READWRITEBUFFER(TUSHORT,   uint16_t);
                    CASE_READWRITEBUFFER(TINT,   int32_t);
                    CASE_READWRITEBUFFER(TUINT,   uint32_t);
                    CASE_READWRITEBUFFER(TLONG,   int64_t);
                    CASE_READWRITEBUFFER(TULONG,   uint64_t);
                    CASE_READWRITEBUFFER(THALF,   CFloat16);
                    CASE_READWRITEBUFFER(TFLOAT,   float);
                    CASE_READWRITEBUFFER(TDOUBLE,   double);
                    // TODO: add support for data types
                default:
                    // Error! Unsupported vector length
                    throw Exception::IOError("XMLReader::ReadBuffer Unsupported data types");
                }
            }
#define CASE_NEATREADWRITEBUFFER(_str, _type) case(_str): ReadWriteBufferVectorNEAT<_type>(pB, pXml, rwtype); break;
            else
            {   // NEAT 
                switch(elemType)
                {
                    CASE_NEATREADWRITEBUFFER(THALF,   CFloat16);
                    CASE_NEATREADWRITEBUFFER(TFLOAT,   float);
                    CASE_NEATREADWRITEBUFFER(TDOUBLE,   double);
                default:
                    // Error! Unsupported vector length
                    throw Exception::IOError("XMLReader::ReadBuffer Unsupported data types");
                }

            }
            return pB;
        }
#undef CASE_NEATREADWRITEBUFFER
#undef CASE_READWRITEBUFFER
    private:
        IBufferContainer * m_pBC;
    };

    /// Memory object reader/writer
    class XMLMemoryObjectReadWrite
        : public IXMLReadWriteBase
    {
    public:
        XMLMemoryObjectReadWrite(IBufferContainer *pBC)
            : m_pBC(pBC)
        {
            if(NULL == m_pBC)
                throw Exception::InvalidArgument("XMLBufferReadWrite::XMLBufferReadWrite constructor NULL input");
        }
        /// @brief read write Buffer data to IContainer
        virtual IContainer* ReadWrite(IContainer* pContainer, TiXmlElement* pXml , const RWOperationType rwtype)
        {
            IMemoryObject * pB = static_cast<IMemoryObject*>(pContainer);
            if (Buffer::GetBufferName() == pB->GetName())
            {
                pXml = (IXMLReadWriteBase::READ == rwtype) ? pXml : new TiXmlElement("Buffer");
                XMLBufferReadWrite rw(m_pBC);
                rw.ReadWrite(pContainer, pXml, rwtype);
            }
            if (Image::GetImageName() == pB->GetName())
            {
                pXml = (IXMLReadWriteBase::READ == rwtype) ? pXml : new TiXmlElement("Image");
                XMLImageReadWrite rw(m_pBC);
                rw.ReadWrite(pContainer, pXml, rwtype);
            }
            return pB;
        }
    private:
        IBufferContainer * m_pBC;
    };

    /// class for read/write BufferContainer
    class XMLBufferContainerReadWrite
        : public IXMLReadWriteBase
    {
    public:
        /// @brief read data to IContainer
        virtual IContainer* ReadWrite(IContainer *pContainer, TiXmlElement* pXml , const RWOperationType rwtype)
        {
            IBufferContainer * pBC = static_cast<IBufferContainer*>(pContainer);
            // Obtain container size - number of objects in container.
            int32_t size_BC = (WRITE == rwtype) ? (int32_t) pBC->GetMemoryObjectCount() : 0;
            XMLReadWriteAttr(&size_BC, pXml, "size", rwtype);

            IMemoryObject * pB = (READ == rwtype) ? NULL : pBC->GetMemoryObject(0);
            TiXmlElement* memObjElem;
            if (READ == rwtype)
            {
                // Read first xml element
                memObjElem = pXml->FirstChildElement();
            }
            else
            {
                if (Buffer::GetBufferName() == pB->GetName())
                {
                    memObjElem = new TiXmlElement("Buffer");
                }
                else if (Image::GetImageName() == pB->GetName())
                {
                    memObjElem = new TiXmlElement("Image");
                }
                else
                {
                    throw Exception::InvalidArgument("XMLBufferContainerReadWrite: Unknown object to write");
                }
                pXml->LinkEndChild(memObjElem);
            }
            if (string("Buffer") == memObjElem->ValueStr())
            {
                XMLBufferReadWrite rw(pBC);
                rw.ReadWrite(pB, memObjElem, rwtype);
            }
            if (string("Image") == memObjElem->ValueStr())
            {
                XMLImageReadWrite rw(pBC);
                rw.ReadWrite(pB, memObjElem, rwtype);
            }

            for (int32_t cnt = 1; cnt < size_BC; ++cnt)
            {
                pB = (READ == rwtype) ? NULL : pBC->GetMemoryObject(cnt);
                if (READ == rwtype)
                {
                    // Read first xml element
                    memObjElem = memObjElem->NextSiblingElement();
                }
                else
                {
                    if (Buffer::GetBufferName() == pB->GetName())
                    {
                        memObjElem = new TiXmlElement("Buffer");
                    }
                    if (Image::GetImageName() == pB->GetName())
                    {
                        memObjElem = new TiXmlElement("Image");
                    }
                    pXml->LinkEndChild(memObjElem);
                }
                if (string("Buffer") == memObjElem->ValueStr())
                {
                    XMLBufferReadWrite rw(pBC);
                    rw.ReadWrite(pB, memObjElem, rwtype);
                }
                if (string("Image") == memObjElem->ValueStr())
                {
                    XMLImageReadWrite rw(pBC);
                    rw.ReadWrite(pB, memObjElem, rwtype);
                }
            }
            return pBC;
        }
    };

    IContainer* XMLBufferContainerListReadWrite::ReadWriteBufferContainerList(IContainer *pContainer, TiXmlElement* pXml , const RWOperationType rwtype)
    {
        IBufferContainerList * pBCL = dynamic_cast<IBufferContainerList*>(pContainer);
        if(NULL == pBCL)
            throw Exception::InvalidArgument("Invalid input/output IContainer object. Cannot cast to IBufferContainerList*");

        AddGetXMLNode addget(pXml, "BufferContainerList", rwtype);
        ReadWrite(pBCL, addget.GetNext(), rwtype);
        return pBCL;
    }


    /// @brief read data to IContainer
    IContainer* XMLBufferContainerListReadWrite::ReadWrite(IContainer *pContainer, TiXmlElement* pXml, const RWOperationType rwtype)
    {
        IBufferContainerList * pBCL = static_cast<IBufferContainerList*>(pContainer);
        int32_t size_BCL = (WRITE == rwtype) ? (int32_t) pBCL->GetBufferContainerCount() : 0;

        XMLReadWriteAttr(&size_BCL, pXml, "size", rwtype);

        std::string version = XMLBufferContainerListReadWrite::XMLFileFormatVersion;
        XMLReadWriteAttr(&version, pXml, "version", rwtype);

        if (XMLBufferContainerListReadWrite::XMLFileFormatVersion != version)
            throw Exception::IOError("Unsupported version of XML node");

        AddGetXMLNode h(pXml, "BufferContainer", rwtype);
        for (int32_t cnt = 0; cnt < size_BCL; ++cnt)
        {
            XMLBufferContainerReadWrite rw;
            IBufferContainer * pBC = (READ == rwtype) ? pBCL->CreateBufferContainer() : pBCL->GetBufferContainer(cnt);
            rw.ReadWrite(static_cast<IContainer*>(pBC), h.GetNext(), rwtype);
        }
        return pBCL;
    }
} // End of Validation namespace
