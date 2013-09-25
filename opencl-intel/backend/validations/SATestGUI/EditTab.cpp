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
#include "EditTab.h"

namespace Validation
{
namespace GUI
{
EditTab::EditTab(ConfigManager* cfg, QString path)
{
    /**
     * @TODO add validator (see QValidator class for help)
     */
        this->path = path;
    this->cfg = cfg;
    type = Tab::Editor;
    textEdit = new QTextEdit(this);
    QFont font("Courier New");
    textEdit->setFont(font);
    highlighter = new OCLHighlighter();
    highlighter->setDocument(textEdit->document());
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    this->setLayout(layout);
    textEdit->setText(cfg->getFilePlainText(path));

}

void EditTab::save()
{
    cfg->saveFilePlainText(path,textEdit->toPlainText());
}

}
}