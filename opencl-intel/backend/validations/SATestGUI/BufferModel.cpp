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
#include "BufferModel.h"

namespace Validation
{
namespace GUI
{

BufferModel::BufferModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_pBuf(NULL)
{
}

BufferModel::BufferModel(BufferConverter *m_pBuf)
{
    this->m_pBuf = m_pBuf;
    m_tableMode = 0;
    m_valueMode = 0;
}

int BufferModel::columnCount(const QModelIndex &parent) const
{
    return m_pBuf->width();
}
int BufferModel::rowCount(const QModelIndex &parent) const
{
    return m_pBuf->lenght();
}
QVariant BufferModel::data(const QModelIndex &i, int role) const
{
    QString str;
        if (role == Qt::DisplayRole)
    {
        qDebug()<<"ask data!";
        switch (m_tableMode) {
        case 0:
            str = m_pBuf->stringAt(i.row(),i.column());
            break;
        case 1:
            str = m_pBuf->hexStringAt(i.row(),i.column());
            break;
        case 2:
            str = m_pBuf->hexfStringAt(i.row(),i.column());
            break;
        default:
            break;
        }
        return QVariant(str);
        }
    else
        return QVariant();

}

void BufferModel::setViewForTable(int i)
{
    this->m_tableMode = i;
}

void BufferModel::setViewForValue(int i)
{
    this->m_valueMode = i;
}

QString BufferModel::value(QModelIndex &i)
{
    switch (m_valueMode) {
    case 0:
        return m_pBuf->stringAt(i.row(),i.column());
        break;
    case 1:
        return m_pBuf->hexStringAt(i.row(),i.column());
        break;
    case 2:
        return m_pBuf->hexfStringAt(i.row(),i.column());
        break;
    default:
        return QString("buffermodel::error!");
        break;
    }

}

void BufferModel::setValue(QModelIndex &index, QString val)
{
    switch (m_valueMode) {
    case 0:
        m_pBuf->setVal(index.row(), index.column(), val);
        break;
    case 1:
        m_pBuf->setHexVal(index.row(), index.column(), val);
        break;
    case 2:
        m_pBuf->setFloatHexVal(index.row(), index.column(), val);
        break;
    default:
        break;
    }
}

}
}