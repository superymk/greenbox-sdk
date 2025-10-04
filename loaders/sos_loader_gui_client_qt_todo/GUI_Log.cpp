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
