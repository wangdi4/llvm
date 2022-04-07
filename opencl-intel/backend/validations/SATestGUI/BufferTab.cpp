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

#include "BufferTab.h"

namespace Validation
{
namespace GUI
{
BufferTab::BufferTab(BufferConverter* buf)
{
    this->buffer=buf;
    type=Tab::Buf;
    tableView = new QTableView(this);
    QString s = "QTableView {\n"\
            "selection-color: black;\n"\
            "selection-background-color: lightblue;\n"\
            "}";

    tableView->setStyleSheet(s);
    tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    model = new BufferModel(buffer);
    tableView->setModel(model);
    hexTableCB = new QComboBox();
    QStringList views;
    views<<"dec"<<"hex"<<"float hex";
    hexTableCB->addItems(views);
    layout = new QVBoxLayout(this);
    typeInfoLine = new QHBoxLayout(this);
    typeInfoLine->addWidget(new QLabel("view as"));
    typeInfoLine->addWidget(hexTableCB);
    layout->addLayout(typeInfoLine);
    layout->addWidget(new QLabel("Type: "+buffer->typeName()));
    layout->addWidget(tableView);
    editValueLine = new QHBoxLayout(this);
    editBtn = new QPushButton("Ok");
    valueLE = new QLineEdit(this);
    hexValueCB = new QComboBox(this);
    hexValueCB->addItems(views);
    editValueLine->addWidget(valueLE);
    editValueLine->addWidget(editBtn);
    editValueLine->addWidget(new QLabel  ("hex"));
    editValueLine->addWidget(hexValueCB);
    layout->addLayout(editValueLine);
    QWidget::setLayout(layout);
    connect(hexTableCB, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTableView()));
    connect(hexValueCB, SIGNAL(currentIndexChanged(int)), this, SLOT(changeValueView()));
    connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(showValue(QModelIndex)));
    connect(editBtn, SIGNAL(clicked()), this, SLOT(changeValue()));
    connect(valueLE, SIGNAL(returnPressed()), this, SIGNAL(changeValue()));
    hexTableCB->setCurrentIndex(0);
    hexValueCB->setCurrentIndex(0);
    tableView->reset();
}




void BufferTab::changeTableView()
{
    model->setViewForTable(hexTableCB->currentIndex());
    tableView->reset();
}

void BufferTab::changeValueView()
{
    model->setViewForValue(hexValueCB->currentIndex());
    showValue(tableView->currentIndex());
}

void BufferTab::showValue(QModelIndex i)
{
    valueLE->setText(model->value(i));
}

void BufferTab::changeValue()
{
    QModelIndex index = tableView->currentIndex();
    QString val = this->valueLE->text();
    this->model->setValue(index,val);
    tableView->reset();
}

}
}
