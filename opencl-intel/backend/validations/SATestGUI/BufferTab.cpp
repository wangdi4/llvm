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