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

#include "ConfigTab.h"

namespace Validation
{
namespace GUI
{

ConfigTab::ConfigTab(ConfigManager* cfg)
{
    this->cfg = cfg;
    type=Tab::Configuration;
    layout = new QFormLayout(this);
    dirs = new QTextEdit(this);
    layout->addRow(new QLabel("Include Dirs"),dirs);
    compFlags = new QLineEdit(this);
    layout->addRow(new QLabel("Compilation flags"),compFlags);
    this->setLayout(layout);
}

}
}
