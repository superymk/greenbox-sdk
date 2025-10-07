// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "GUI_SPMgr.h"
#include "ui_GUI_SPMgr.h"

GUI_SPMgr::GUI_SPMgr(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GUI_SPMgr)
{
    ui->setupUi(this);
}

GUI_SPMgr::~GUI_SPMgr()
{
    delete ui;
}
