#ifndef GUI_LOG_H
#define GUI_LOG_H

#include <QMainWindow>

namespace Ui {
class GUI_Log;
}

class GUI_Log : public QMainWindow
{
    Q_OBJECT

public:
    explicit GUI_Log(QWidget *parent = nullptr);
    ~GUI_Log();

private:
    Ui::GUI_Log *ui;
};

#endif // GUI_LOG_H
