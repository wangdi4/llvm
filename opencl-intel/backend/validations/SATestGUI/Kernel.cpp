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