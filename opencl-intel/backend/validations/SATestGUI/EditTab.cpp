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
