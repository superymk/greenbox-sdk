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
