/********************************************************************************
** Form generated from reading UI file 'GUI_SPMgr.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GUI_SPMGR_H
#define UI_GUI_SPMGR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GUI_SPMgr
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QTableWidget *tableWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *btn_soft_kill_sp;
    QLabel *label_2;

    void setupUi(QMainWindow *GUI_SPMgr)
    {
        if (GUI_SPMgr->objectName().isEmpty())
            GUI_SPMgr->setObjectName("GUI_SPMgr");
        GUI_SPMgr->resize(600, 600);
        centralwidget = new QWidget(GUI_SPMgr);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        QFont font;
        font.setFamilies({QString::fromUtf8("Arial")});
        font.setPointSize(12);
        label->setFont(font);

        verticalLayout->addWidget(label);

        tableWidget = new QTableWidget(centralwidget);
        tableWidget->setObjectName("tableWidget");
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Arial")});
        tableWidget->setFont(font1);

        verticalLayout->addWidget(tableWidget);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        btn_soft_kill_sp = new QPushButton(centralwidget);
        btn_soft_kill_sp->setObjectName("btn_soft_kill_sp");
        btn_soft_kill_sp->setFont(font);

        horizontalLayout->addWidget(btn_soft_kill_sp);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");

        horizontalLayout->addWidget(label_2);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);

        GUI_SPMgr->setCentralWidget(centralwidget);

        retranslateUi(GUI_SPMgr);

        QMetaObject::connectSlotsByName(GUI_SPMgr);
    } // setupUi

    void retranslateUi(QMainWindow *GUI_SPMgr)
    {
        GUI_SPMgr->setWindowTitle(QCoreApplication::translate("GUI_SPMgr", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("GUI_SPMgr", "Running SecProcesses:", nullptr));
        btn_soft_kill_sp->setText(QCoreApplication::translate("GUI_SPMgr", "End SecProcess", nullptr));
        label_2->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class GUI_SPMgr: public Ui_GUI_SPMgr {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUI_SPMGR_H
