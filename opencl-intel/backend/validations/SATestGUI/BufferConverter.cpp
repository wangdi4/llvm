// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
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

#include "bufferconverter.h"
#include "RefALU.h"
#include <QDebug>

namespace Validation
{
namespace GUI
{

static QString doubleToHex(double val);
static QString floatToHex(float val);
static float parseFloat(QString str);
static double parseDouble(QString str);

const unsigned int CHAR_STR_LEN = 80;

BufferConverter::BufferConverter() :
    m_pBuffer(NULL)
{
}

BufferConverter::BufferConverter(Buffer* buf)
{

    const BufferDesc* bufdesc = static_cast<const BufferDesc*>(buf->GetMemoryObjectDesc());
    if(bufdesc)
    {
        m_pBuffer = buf;
        qDebug()<<"BufferConverter constructor";
        this->m_numOfElements = bufdesc->NumOfElements();
        this->m_vectSize = bufdesc->SizeOfVector();
        bufData = m_pBuffer->GetDataPtr();
        TypeDesc typeDescr = bufdesc->GetElementDescription();
        m_type = typeDescr.GetType();

        qDebug()<<"num of elements: "<<m_numOfElements;
        qDebug()<<"size of vector: "<<m_vectSize;

        qDebug()<<"type: "<<m_type;
        /**
         * @TODO add checking of TARRAY
         */
        if(m_type == TVECTOR)
        {

            TypeDesc subTypeDesc = typeDescr.GetSubTypeDesc(0);
            TypeVal subtype = subTypeDesc.GetType();
            m_type = subtype;
            m_typeName = QString::fromStdString(subTypeDesc.TypeToString());
            m_typeName += ":"+QString::number(m_vectSize);
            qDebug()<<"vector";
        }
        else
        {
            //parseData(data, type, m_numOfElements, m_vectSize);
            m_typeName = QString::fromStdString(typeDescr.TypeToString());
        }
        qDebug()<<qPrintable(m_typeName);

    }
}

const int BufferConverter::lenght()
{
    qDebug()<<"ask length "<<m_numOfElements;
    return m_numOfElements;

}

const int BufferConverter::width()
{
    qDebug()<<"ask width "<<m_vectSize;
    return m_vectSize;

}

const QString BufferConverter::stringAt(const int i, const int j)
{
    QString str("Error");
    switch (m_type) {
    case THALF:
        /**
         * @TODO Add covertion to QString from half
         */
        break;
    case TFLOAT:
        str = toString(at(i,j,(float*)bufData));
        break;
    case TDOUBLE:
        str = toString(at(i,j,(double*)bufData));
        break;
    case TCHAR:
        str = toString(at(i,j,(char*)bufData));
        break;
    case TSHORT:
        str = toString(at(i,j,(short*)bufData));
        break;
    case TINT:
        str = toString(at(i,j,(int*)bufData));
        break;
    case TLONG:
        str = toString(at(i,j,(long*)bufData));
        break;
    case TUCHAR:
        str = toString(at(i,j,(uchar*)bufData));
        break;
    case TUSHORT:
        str = toString(at(i,j,(ushort*)bufData));
        break;
    case TUINT:
        str = toString(at(i,j,(uint*)bufData));
        break;
    case TULONG:
        str = toString(at(i,j,(ulong*)bufData));
        break;
    default:
        break;
    }
    return str;
}

const QString BufferConverter::hexStringAt(const int i, const int j)
{
    QString str("Error");
    switch (m_type) {
    case TFLOAT:
        str = toHex(at(i,j,(float*)bufData));
        break;
    case TDOUBLE:
        str = toHex(at(i,j,(double*)bufData));
        break;
    case TCHAR:
        str = toHex(at(i,j,(char*)bufData));
        break;
    case TSHORT:
        str = toHex(at(i,j,(short*)bufData));
        break;
    case TINT:
        str = toHex(at(i,j,(int*)bufData));
        break;
    case TLONG:
        str = toHex(at(i,j,(long*)bufData));
        break;
    case TUCHAR:
        str = toHex(at(i,j,(uchar*)bufData));
        break;
    case TUSHORT:
        str = toHex(at(i,j,(ushort*)bufData));
        break;
    case TUINT:
        str = toHex(at(i,j,(uint*)bufData));
        break;
    case TULONG:
        str = toHex(at(i,j,(ulong*)bufData));
        break;
    default:
        break;
    }
    return str;
}
const QString BufferConverter::hexfStringAt(const int i, const int j)
{
    QString str("Error");
    switch (m_type) {
    case TFLOAT:
        str = floatToHex(at(i,j,(float*)bufData));
        break;
    case TDOUBLE:
        str = doubleToHex(at(i,j,(double*)bufData));
        break;
    default:
        break;
    }

    return str;
}

const void BufferConverter::setVal(const int i, const int j, const QString sval)
{
    switch (m_type) {
    case THALF:
        /**
         * @TODO add convertion from QString to half
         */
        break;
    case TFLOAT:
    {
        float val = sval.toFloat();
        ((float*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TDOUBLE:
    {
        double val = sval.toDouble();
        ((double*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TCHAR:
    {
        char val = sval.toInt();
        ((char*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    case TSHORT:
    {
        short val = sval.toShort();
        ((short*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    case TINT:
    {
        int val = sval.toInt();
        ((int*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    case TLONG:
    {
        long val = sval.toLong();
        ((long*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    case TUCHAR:
    {
        uchar val = sval.toUInt();
        ((uchar*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    case TUSHORT:
    {
        ushort val = sval.toUShort();
        ((ushort*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    case TUINT:
    {
        uint val = sval.toUInt();
        ((uint*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    case TULONG:
    {
        ulong val = sval.toULong();
        ((ulong*)bufData)[i*m_vectSize+j]=val;
    }
        break;
    default:
        break;
    }
}

void BufferConverter::setHexVal(int i, int j, QString sval)
{
    switch (m_type) {
    case THALF:
        /**
         * @TODO add convertion from hex string to half
         */
        break;
    case TFLOAT:
    {
        float val;
        fromHex(sval,&val);
        ((float*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TDOUBLE:
    {
        double val;
        fromHex(sval,&val);
        ((double*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TCHAR:
    {
        char val;
        fromHex(sval,&val);
        ((char*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TSHORT:
    {
        short val;
        fromHex(sval,&val);
        ((short*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TINT:
    {
        int val;
        fromHex(sval,&val);
        ((int*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TLONG:
    {
        long val;
        fromHex(sval,&val);
        ((long*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TUCHAR:
    {
        uchar val;
        fromHex(sval,&val);
        ((uchar*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TUSHORT:
    {
        ushort val;
        fromHex(sval,&val);
        ((ushort*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TUINT:
    {
        uint val;
        fromHex(sval,&val);
        ((uint*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TULONG:
    {
        ulong val;
        fromHex(sval,&val);
        ((ulong*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    default:
        break;
    }
}

void BufferConverter::setFloatHexVal(int i, int j, QString sval)
{
    switch (m_type) {
    case THALF:
        /**
         * @TODO add convertion from float hex string to half
         */
        break;
    case TFLOAT:
    {
        float val = parseFloat(sval);
        ((float*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    case TDOUBLE:
    {
        double val=parseDouble(sval);
        ((double*)bufData)[i*m_vectSize+j] = val;
    }
        break;
    default:
        break;
    }
}


template<class T> static
QString BufferConverter::toString(T number)
{
    return QString::number(number);
}




template<class T> static
QString BufferConverter::toHex(T arg)
{
    QString str;
    QTextStream hexstr(&str);
    const char* addr = reinterpret_cast<const char*>(&arg) ;
    hexstr << "0x" ;
    hexstr.setFieldWidth(2);
    hexstr.setRealNumberPrecision(2);
    hexstr.setPadChar('0');
    hexstr.setIntegerBase(16);
    for( int b = sizeof(arg) - 1; b >= 0; b-- )
    {
        hexstr << static_cast<unsigned>(*(addr+b) & 0xff) ;
    }
    return str;
}

template<class T> static
void BufferConverter::fromHex(QString str, T *arg)
{
    str.remove(0,2);
    T val_bytes;
    T val_revbytes;
    unsigned char* bytes = (unsigned char*)&val_bytes;
    unsigned char* revbytes = (unsigned char*)&val_revbytes;

    for(int i=0; i<sizeof(T); i++) {
        bool ok;
        int byteval = str.mid(i*2,2).toInt(&ok, 16);
        bytes[i]=byteval;
    }

    for(int i=0; i<sizeof(T); i++)
        revbytes[sizeof(T)-1-i]=bytes[i];

    *arg = *reinterpret_cast<T*>(revbytes);
}

template<class T>
T BufferConverter::at(int i, int j, T *data)
{
    T val = data[i*m_vectSize+j];
    return val;
}


/**
 * @brief parseDouble - parsing double value written in %a format
 * @param str
 * @return double value
 */
static double parseDouble(QString str)
{
    /**
     * @TODO add checking NaN and Inf
     */
    double val;
    int pointIndex = str.indexOf(".");
    int pIndex = str.indexOf("p");
    int sm=1;
    int se=1;
    if(str[0]=='-')
    {
        sm=-1;
    }
    if(str[pIndex+1]=='-')
    {
        se=-1;
    }
    QString _int, _fract, _exp;
    _int = str[pointIndex-1];
    _fract = str.mid(pointIndex+1,pIndex-pointIndex-1);
    _exp = str.mid(pIndex+2, str.size()-pIndex);

    bool ok;
    QString significand;
    significand = "0x";
    significand +=_int;
    significand +=_fract;
    unsigned long long _significand = significand.toLongLong(&ok, 16);

    int iexp = _exp.toInt(&ok,10);

    QString log1;
    log1 = "0x";
    log1+= _int;
    unsigned int _log1 = log1.toInt(&ok,16);

    QString log2;
    log2 = "0x";
    log2+= _int;
    log2+= _fract;
    unsigned long long _log2 = log2.toLongLong(&ok,16);

    val = sm*ldexp((double)_significand,(iexp*se + RefALU::ilogb((double)_log1) - RefALU::ilogb((double)_log2)));
    return val;
}
/**
 * @brief parseFloat - parsing float value written in %a format
 * @param str
 * @return float value
 */
static float parseFloat(QString str)
{
    /**
     * @TODO add checking NaN and Inf
     */
    float val;
    int pointIndex = str.indexOf(".");
    int pIndex = str.indexOf("p");
    int sm = 1;
    int se = 1;
    if(str[0] == '-')
    {
        sm = -1;
    }
    if(str[pIndex + 1] == '-')
    {
        se = -1;
    }
    QString _int, _fract, _exp;
    _int = str[pointIndex - 1];
    _fract = str.mid(pointIndex+1, pIndex - pointIndex - 1);
    _exp = str.mid(pIndex + 2, str.size() - pIndex);

    bool ok;
    QString significand;
    significand = "0x";
    significand += _int;
    significand += _fract;
    long _significand = significand.toLong(&ok, 16);

    int iexp = _exp.toInt(&ok,10);

    QString log1;
    log1 = "0x";
    log1 += _int;
    unsigned int _log1 = log1.toInt(&ok,16);
    QString log2;
    log2 = "0x";
    log2 += _int;
    log2 += _fract;
    long _log2 = log2.toLong(&ok,16);

    val = sm * ldexpf( (float)_significand, (iexp*se + RefALU::ilogb((float)_log1) - RefALU::ilogb((float)_log2) ) );

    return val;
}
/**
 * @brief floatToHex
 * @param val
 * @return string in %a format
 */
static QString floatToHex(float val)
{
    QString str;

    if(Utils::IsNaN(val)) {
        str = "NaN";
    } else if(Utils::IsPInf(val)) {
        str = "+Inf";
    } else if(Utils::IsNInf(val)) {
        str = "-Inf";
    } else {
        char hexch[CHAR_STR_LEN];
        sprintf(hexch,"%a",val);
        str = hexch;
    }
    return str;
}
/**
 * @brief doubleToHex
 * @param val
 * @return string in %a format
 */
static QString doubleToHex(double val)
{
    QString str;
    if(Utils::IsNaN(val)) {
        str = "NaN";
    } else if(Utils::IsPInf(val)) {
        str = "+Inf";
    } else if(Utils::IsNInf(val)) {
        str = "-Inf";
    } else {
        char hexch[CHAR_STR_LEN];
        sprintf(hexch,"%.13la",val);
        return QString(hexch);
    }
}

}
}
