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

#ifndef BUFFERCONVERTER_H
#define BUFFERCONVERTER_H
#include <QString>
#include <QVector>
#include <QStringList>
#include "Buffer.h"
#include <CL/cl_platform.h>

#include "FloatOperations.h"

namespace Validation
{
namespace GUI
{

/**
 * @brief The BufferConverter class
 * @detailed This class implements all work with buffer and it's data
 */
class BufferConverter
{
public:
    BufferConverter();
    /**
     * @brief BufferConverter constructor
     * @param Buffer* object from DataManager/BufferConverter.h
     */
    BufferConverter(Buffer*);
    /**
     * @brief lenght
     * @return the lenght of buffer. If buffer contains 2 vecors of float 4, the length is 2
     */
    const int lenght();
    /**
     * @brief width
     * @return the width of vector inside the buffer. If buffer contains 2 vecors of float 4, the width is 4
     */
    const int width();
    /**
     * @brief typeName
     * @return type name of buffer (in type:size format)
     */
    const QString typeName(){return m_typeName;}
    /**
     * @brief stringAt
     * @return value in decimal from buffer
     */
    const QString stringAt(const int, const int);
    /**
     * @brief hexstringAt
     * @return value in hex from buffer
     * @TODO add checking NEAT
     */
    const QString hexStringAt(const int, const int);
    /**
     * @brief hexfstringAt
     * @return value in float from buffer
     * @TODO add checking NEAT
     */
    const QString hexfStringAt(const int, const int);
    /**
     * @brief set value to buffer from string in decimal format
     * @TODO add checking NEAT
     */
    const void setVal(const int,const int,const QString);
    /**
     * @brief set value to buffer from string in hex format
     * @TODO add checking NEAT
     */
    void setHexVal(int,int,QString);
    /**
     * @brief set value to buffer from string in float hex format
     * @TODO add checking NEAT
     */
    void setFloatHexVal(int,int,QString);
private:
    QString m_typeName;
    int m_numOfElements, m_vectSize;
    TypeVal m_type;

    Buffer* m_pBuffer; // BufferConverter is not an owner of this buffer, so we can't delete it in dtor
    /**
     * @brief bufData - pointer to raw data from bufer
     */
    void* bufData;
    /**
     * @brief return value from buffer
     * @TODO add checking NEAT
     */
    template<class T> T at(int,int,T*);
    /**
     * @brief return string of value
     * @TODO add checking NEAT
     */
    template<class T> static QString toString(T);
    /**
     * @brief return hexstring of value
     * @TODO add checking NEAT
     */
    template<class T> static QString toHex(T arg);
    /**
     * @brief return float hexstring of value
     * @TODO add checking NEAT
     */
    template<class T> static void fromHex(QString str, T* arg);

};

}
}

#endif // BUFFER_H
