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
#ifndef BUFFERMODEL_H
#define BUFFERMODEL_H

#include <QAbstractTableModel>
#include "BufferConverter.h"
#include <QDebug>

namespace Validation
{
namespace GUI
{

/**
 * @brief The BufferModel class
 * @detailed need for deduplicate data in memory - table in BufferTab get data from Buffer object directly
 */
class BufferModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    /**
     * @brief BufferModel empty constructor
     * @param parent
     */
    explicit BufferModel(QObject *parent = 0);
    /**
     * @brief BufferModel constructor
     * @param buf - Buffer object
     */
    BufferModel(BufferConverter* buf);
    /**
     * @brief columnCount
     * @param parent
     * @return count of column in buffer
     */
    int columnCount(const QModelIndex &parent) const;
    /**
     * @brief rowCount
     * @param parent
     * @return count of row in buffer
     */
    int rowCount(const QModelIndex &parent) const;
    /**
     * @brief data
     * @param index in table
     * @param role
     * @return data from i and j from table in buffer
     */
    QVariant data(const QModelIndex &index, int role) const;
    /**
     * @brief setViewForTable - set variant of representation for table (dec, hex, float hex)
     */
    void setViewForTable(int);
    /**
     * @brief setViewForValue - set variant of representation for value-line (dec, hex, float hex)
     */
    void setViewForValue(int);
    /**
     * @brief value - get value for value-line from buffer with indexes from table
     * @return value in selected view (dec, hex, floathex)
     */
    QString value(QModelIndex &);
    /**
     * @brief setValue set value in buffer in index from table in selected view
     * @param index
     * @param val
     */
    void setValue(QModelIndex &index, QString val);
signals:

public slots:
private:
    BufferConverter* m_pBuf;
    int m_tableMode;
    int m_valueMode;


};

}
}

#endif // BUFFERMODEL_H
