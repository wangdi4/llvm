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
