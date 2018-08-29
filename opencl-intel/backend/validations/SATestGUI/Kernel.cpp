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

#include "Kernel.h"
#include <QtAlgorithms>

namespace Validation
{
namespace GUI
{
Kernel::Kernel()
{}
Kernel::Kernel(KernelInfo info)
{
    this->info = info;
    infile = new QFile(info.infile);
    reffile = new QFile(info.reffile);
    neatfile = new QFile(info.neatfile);
    inBuffers = new QVector<BufferConverter*>;
    refBuffers = new QVector<BufferConverter*>;
    neatBuffers = new QVector<BufferConverter*>;

    inBufferContainerList = NULL;
    refBufferContainerList = NULL;
    neatBufferContainerList = NULL;

    qDebug()<<"reading in";
    parseFile(infile, inBuffers, info.infile, info.infileType, &inBufferContainerList);
    qDebug()<<"reading ref";
    parseFile(reffile, refBuffers, info.reffile, info.reffileType, &refBufferContainerList);
    qDebug()<<"reading neat";
    parseFile(neatfile, neatBuffers, info.neatfile, info.neatfileType, &neatBufferContainerList);

}


static void saveBuffer(const QString &qstr, const DataFileType &fileType, const BufferContainerList *bufferContainerList) {
    IDataWriter * bufferWriter;
    std::string str = qstr.toLocal8Bit().constData();
    if(fileType == Xml) {
        bufferWriter = new XMLBufferContainerListWriter(str);
    } else if(fileType == Binary) {
        bufferWriter = new BinaryContainerListWriter(str);
    }
    bufferWriter->Write(bufferContainerList);
    delete bufferWriter;
}

void Kernel::saveInBuffer()
{
    qDebug()<<"saving in buffer";
    saveBuffer(inFilePath(), inFileType(), inBufferContainerList);
}

void Kernel::saveRefBuffer()
{
    qDebug()<<"saving ref buffer";
    saveBuffer(refFilePath(), refFileType(), refBufferContainerList);
}

void Kernel::saveNeatBuffer()
{
    qDebug()<<"saving neat buffer";
    saveBuffer(neatFilePath(), neatFileType(), neatBufferContainerList);
}


void Kernel::parseFile(QFile *_file, QVector<BufferConverter*> *buffers, QString &qstr, DataFileType &fileType, BufferContainerList ** bufferContainerList) {

    if(_file->exists() && _file->size()!=0){
        IDataReader * bufferReader;
        std::string str = qstr.toLocal8Bit().constData();
        if(fileType == Xml) {
            bufferReader = new XMLBufferContainerListReader(str);
        } else if (fileType == Binary) {
            bufferReader = new BinaryContainerListReader(str);
        }

        *bufferContainerList = new BufferContainerList();
        bufferReader->Read(*bufferContainerList);
        BufferContainer* bufferContainer = static_cast<BufferContainer*>( (*bufferContainerList)->GetBufferContainer(0) );

        int buffersSize = bufferContainer->GetMemoryObjectCount();

        for(int i =0 ; i< buffersSize; i++)  {
            IMemoryObject* object = bufferContainer->GetMemoryObject(i);
            Buffer* buffer = static_cast<Buffer*>(object);
            BufferConverter* buf = new BufferConverter(buffer);
            buffers->append(buf);
        }
        delete bufferReader;
    }
}


QString Kernel::getFileName(QString path)
{
    QFileInfo info(path);
    return info.fileName();
}

Kernel::~Kernel() {
    delete infile;
    delete reffile;
    delete neatfile;

    qDeleteAll(*inBuffers);
    delete inBuffers;
    qDeleteAll(*refBuffers);
    delete refBuffers;
    qDeleteAll(*inBuffers);
    delete neatBuffers;

    if(inBufferContainerList)
        delete inBufferContainerList;
    if(refBufferContainerList)
        delete refBufferContainerList;
    if(neatBufferContainerList)
        delete neatBufferContainerList;
}

}
}
