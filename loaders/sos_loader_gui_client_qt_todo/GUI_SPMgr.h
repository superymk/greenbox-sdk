#ifndef GUI_SPMGR_H
#define GUI_SPMGR_H

#include <QMainWindow>

namespace Ui {
class GUI_SPMgr;
}

class GUI_SPMgr : public QMainWindow
{
    Q_OBJECT

public:
    explicit GUI_SPMgr(QWidget *parent = nullptr);
    ~GUI_SPMgr();

private:
    Ui::GUI_SPMgr *ui;
};

#endif // GUI_SPMGR_H
