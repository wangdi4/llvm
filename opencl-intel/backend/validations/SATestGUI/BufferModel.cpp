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
