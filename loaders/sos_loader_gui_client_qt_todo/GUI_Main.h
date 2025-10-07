// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Frm_Main;
}
QT_END_NAMESPACE

class Frm_Main : public QMainWindow
{
    Q_OBJECT

public:
    Frm_Main(QWidget *parent = nullptr);
    ~Frm_Main();

private slots:
    void on_btn_run_sos_clicked();

    void on_btn_stop_sos_clicked();

    void on_btn_config_sos_clicked();

private:
    Ui::Frm_Main *ui;
};
#endif // GUI_MAIN_H
