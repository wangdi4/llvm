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

#ifndef TAB_H
#define TAB_H

#include <QWidget>

namespace Validation
{
namespace GUI
{

/**
 * @brief The Tab class
 * @detailed parent-class for all tabs
 */
class Tab : public QWidget
{
    Q_OBJECT
public:
    explicit Tab(QWidget *parent = 0);
    /**
     * @brief The TabType enum uses to identificate tab
     */
    enum TabType
    {
        Editor,
        KernelInfo,
        Buf,
        Configuration
    }type;

signals:

public slots:

};

}
}

#endif // TAB_H
