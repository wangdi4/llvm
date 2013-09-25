/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.
\*****************************************************************************/
#ifndef KERNEL_H
#define KERNEL_H
#include "KernelInfo.h"
#include <QString>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QDebug>
#include <QVector>
#include <QFileInfo>
#include <QMessageBox>
#include <QCoreApplication>
#include "BufferConverter.h"
#include "XMLDataReader.h"
#include "XMLDataWriter.h"
#include "BinaryDataReader.h"
#include "BinaryDataWriter.h"
#include "IDataReader.h"
#include "IDataWriter.h"
#include "BufferContainerList.h"
#include "Buffer.h"
#include "BufferContainer.h"
#include <string>

namespace Validation
{
namespace GUI
{

/**
 * @brief The Kernel class
 * @detailed Work with kernels - save/load buffers, return pathes to buffer-files
 * @TODO add KernelInfo member and create getters and setters to its values (WorkOffset, etc)
 */
class Kernel
{
public:
    Kernel();
    /**
     * @brief Kernel constructor
     * @param info - struct with information about kernel, whitch got from *.cfg
     */
    Kernel(KernelInfo info);
    /**
     * @brief Kernel destructor
     */
    ~Kernel();
    /**
     * @brief return kernel name
     * @return
     */
    QString name() {return info.name;}
    /**
     * @brief inFileType
     * @return type of *.in file
     */
    DataFileType inFileType() {return info.infileType;}
    /**
     * @brief inFilePath
     * @return path to *.in file
     */
    QString inFilePath() {return info.infile;}
    /**
     * @brief inFileName
     * @return name of *.in file
     */
    QString inFileName() {return getFileName(inFilePath());}
    /**
     * @brief refFileType
     * @return type of *.in file
     */
    DataFileType refFileType() {return info.reffileType;}
    /**
     * @brief refFilePath
     * @return path to *.ref file
     */
    QString refFilePath() {return info.reffile;}
    /**
     * @brief refFileName
     * @return name of *.ref file
     */
    QString refFileName() {return getFileName(refFilePath());}
    /**
     * @brief neatFileType
     * @return type of *.in file
     */
    DataFileType neatFileType() {return info.neatfileType;}
    /**
     * @brief neatFilePath
     * @return path to *.neat file
     */
    QString neatFilePath() {return info.neatfile;}
    /**
     * @brief neatFileName
     * @return name of *.neat file
     */
    QString neatFileName() {return getFileName(neatFilePath());}
    /**
     * @brief getInBuffers
     * @return container with buffers from *.in file
     */
    QVector<BufferConverter*>* getInBuffers() {return inBuffers;}
    /**
     * @brief getRefBuffers
     * @return container with buffers from *.ref file
     */
    QVector<BufferConverter*>* getRefBuffers() {return refBuffers;}
    /**
     * @brief getNeatBuffers
     * @return container with buffers from *.near file
     */
    QVector<BufferConverter*>* getNeatBuffers() {return neatBuffers;}
    /**
     * @brief save buffers to *.in file
     */
    void saveInBuffer();
    /**
     * @brief save buffers to *.ref file
     */
    void saveRefBuffer();
    /**
     * @brief save buffers to *.neat file
     */
    void saveNeatBuffer();
private:
    Kernel( const Kernel& );
    const Kernel& operator=( const Kernel& );

    BufferContainerList *inBufferContainerList, *refBufferContainerList, *neatBufferContainerList;
    KernelInfo info;
    QDomDocument *indom, *refdom, *neatdom;
    QFile *infile, *reffile, *neatfile;
    QVector<BufferConverter*> *inBuffers, *refBuffers, *neatBuffers;
    /**
     * @brief reading *.in or *.ref or *.neat file and create buffer's objects
     */
    void parseFile(QFile *_file, QVector<BufferConverter*> *buffers, QString &qstr, DataFileType &fileType, BufferContainerList ** bufferContainerList);
    /**
     * @brief create filename from full path to file
     * @param path
     * @return filename
     */
    QString getFileName(QString path);
};


}
}
#endif // KERNEL_H
