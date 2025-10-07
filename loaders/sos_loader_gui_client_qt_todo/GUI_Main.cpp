// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "GUI_Main.h"
#include "./ui_GUI_Main.h"

Frm_Main::Frm_Main(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Frm_Main)
{
    ui->setupUi(this);
}

Frm_Main::~Frm_Main()
{
    delete ui;
}

void Frm_Main::on_btn_run_sos_clicked()
{

}


void Frm_Main::on_btn_stop_sos_clicked()
{

}


void Frm_Main::on_btn_config_sos_clicked()
{

}

