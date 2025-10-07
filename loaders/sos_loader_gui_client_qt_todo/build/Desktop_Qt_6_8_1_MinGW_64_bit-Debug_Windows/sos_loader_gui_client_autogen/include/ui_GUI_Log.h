/********************************************************************************
** Form generated from reading UI file 'GUI_Log.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GUI_LOG_H
#define UI_GUI_LOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GUI_Log
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QTextEdit *t_log;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *GUI_Log)
    {
        if (GUI_Log->objectName().isEmpty())
            GUI_Log->setObjectName("GUI_Log");
        GUI_Log->resize(500, 300);
        centralwidget = new QWidget(GUI_Log);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        t_log = new QTextEdit(centralwidget);
        t_log->setObjectName("t_log");

        verticalLayout->addWidget(t_log);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        GUI_Log->setCentralWidget(centralwidget);
        menubar = new QMenuBar(GUI_Log);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 500, 18));
        GUI_Log->setMenuBar(menubar);
        statusbar = new QStatusBar(GUI_Log);
        statusbar->setObjectName("statusbar");
        GUI_Log->setStatusBar(statusbar);

        retranslateUi(GUI_Log);

        QMetaObject::connectSlotsByName(GUI_Log);
    } // setupUi

    void retranslateUi(QMainWindow *GUI_Log)
    {
        GUI_Log->setWindowTitle(QCoreApplication::translate("GUI_Log", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GUI_Log: public Ui_GUI_Log {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUI_LOG_H
