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
#ifndef FILETREE_H
#define FILETREE_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QAction>
#include <QHash>
#include "OpenCLProgramConfiguration.h"
#include "ConfigManager.h"
#include "Entry.h"

namespace Validation
{
namespace GUI
{

/**
 * @brief The FileTree class
 * @detailed This widget show stucture of SATest project
 */
class FileTree : public QTreeWidget
{
    Q_OBJECT
public:
    /**
     * @brief FileTree default constructor
     * @param parent
     */
    explicit FileTree(QWidget *parent = 0);
    /**
     * @brief setConfig - add pointer to ConfigMAnager object
     */
    void setConfig(ConfigManager*);

signals:
    /**
     * @brief showRawFile - emits when user click to "Show Raw file" option in context menu
     */
    void showRawFile(Entry);
    /**
     * @brief showBuffer - emits when user clicks to buffer item in tree
     */
    void showBuffer(Entry);
    /**
     * @brief showConfig - emits when user clicks to config item in tree
     */
    void showConfig(Entry);
    /**
     * @brief showKernelInfo - emits when user clicks to kernel item in tree
     */
    void showKernelInfo(Entry);
    /**
     * @brief showRunVariants - emits when user clicks to run options item in tree
     */
    void showRunVariants(Entry);

public slots:
private:
    QString filepath;
    QString dirpath;
    ConfigManager *cfg;
    QAction* rawAction;
    QHash<QTreeWidgetItem*,Entry> entries;
    Entry getEntry(QTreeWidgetItem*);
    void addEntry(QTreeWidgetItem*, Entry::Type, QString path);
    void addEntry(QTreeWidgetItem *, Entry::Type, Entry::BufferType, int kernelId, int bufferNum);
    void addEntry(QTreeWidgetItem *, Entry::Type, int kernelId);
private slots:
    void catchDoubleClicked(QTreeWidgetItem*,int);
    void editRawFileRequest();

};

}
}

#endif // FILETREE_H
