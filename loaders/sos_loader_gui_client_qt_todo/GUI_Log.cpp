// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "GUI_Log.h"
#include "ui_GUI_Log.h"

GUI_Log::GUI_Log(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GUI_Log)
{
    ui->setupUi(this);
}

GUI_Log::~GUI_Log()
{
    delete ui;
}
