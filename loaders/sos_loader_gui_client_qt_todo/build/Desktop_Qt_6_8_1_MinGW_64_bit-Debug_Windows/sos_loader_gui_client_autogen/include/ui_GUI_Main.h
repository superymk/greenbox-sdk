/********************************************************************************
** Form generated from reading UI file 'GUI_Main.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GUI_MAIN_H
#define UI_GUI_MAIN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Frm_Main
{
public:
    QAction *actionGreenBox_Control_Monitor;
    QAction *actionAbout;
    QAction *actionSecApp_Task_Manager;
    QAction *actionShow;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_2;
    QPushButton *btn_config_sos;
    QHBoxLayout *horizontalLayout;
    QPushButton *btn_run_sos;
    QPushButton *btn_stop_sos;
    QLabel *label;
    QMenuBar *menubar;
    QMenu *menuTasks;
    QMenu *menuLog;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Frm_Main)
    {
        if (Frm_Main->objectName().isEmpty())
            Frm_Main->setObjectName("Frm_Main");
        Frm_Main->resize(800, 800);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Frm_Main->sizePolicy().hasHeightForWidth());
        Frm_Main->setSizePolicy(sizePolicy);
        Frm_Main->setMinimumSize(QSize(800, 800));
        Frm_Main->setMaximumSize(QSize(800, 800));
        actionGreenBox_Control_Monitor = new QAction(Frm_Main);
        actionGreenBox_Control_Monitor->setObjectName("actionGreenBox_Control_Monitor");
        QFont font;
        font.setFamilies({QString::fromUtf8("Segoe UI")});
        actionGreenBox_Control_Monitor->setFont(font);
        actionAbout = new QAction(Frm_Main);
        actionAbout->setObjectName("actionAbout");
        actionAbout->setFont(font);
        actionSecApp_Task_Manager = new QAction(Frm_Main);
        actionSecApp_Task_Manager->setObjectName("actionSecApp_Task_Manager");
        actionSecApp_Task_Manager->setFont(font);
        actionShow = new QAction(Frm_Main);
        actionShow->setObjectName("actionShow");
        actionShow->setFont(font);
        actionShow->setShortcutVisibleInContextMenu(true);
        centralwidget = new QWidget(Frm_Main);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        btn_config_sos = new QPushButton(centralwidget);
        btn_config_sos->setObjectName("btn_config_sos");
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Arial")});
        font1.setPointSize(16);
        font1.setBold(true);
        btn_config_sos->setFont(font1);

        verticalLayout_2->addWidget(btn_config_sos);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        btn_run_sos = new QPushButton(centralwidget);
        btn_run_sos->setObjectName("btn_run_sos");
        btn_run_sos->setFont(font1);
        btn_run_sos->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    color: #00AE00; /* Change to desired text color */\n"
"}"));

        horizontalLayout->addWidget(btn_run_sos);

        btn_stop_sos = new QPushButton(centralwidget);
        btn_stop_sos->setObjectName("btn_stop_sos");
        QPalette palette;
        QBrush brush(QColor(255, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(202, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Text, brush1);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush1);
        QBrush brush2(QColor(202, 0, 0, 128));
        brush2.setStyle(Qt::SolidPattern);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Active, QPalette::PlaceholderText, brush2);
#endif
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Inactive, QPalette::PlaceholderText, brush2);
#endif
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Disabled, QPalette::PlaceholderText, brush2);
#endif
        btn_stop_sos->setPalette(palette);
        btn_stop_sos->setFont(font1);
        btn_stop_sos->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    color: #CA0000; /* Change to desired text color */\n"
"}"));

        horizontalLayout->addWidget(btn_stop_sos);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addLayout(verticalLayout_2);


        gridLayout->addLayout(verticalLayout, 1, 0, 1, 1);

        label = new QLabel(centralwidget);
        label->setObjectName("label");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(90);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Arial")});
        label->setFont(font2);
        label->setPixmap(QPixmap(QString::fromUtf8(":/resources/Greenbox.png")));
        label->setScaledContents(true);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        Frm_Main->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Frm_Main);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 18));
        menuTasks = new QMenu(menubar);
        menuTasks->setObjectName("menuTasks");
        menuLog = new QMenu(menubar);
        menuLog->setObjectName("menuLog");
        Frm_Main->setMenuBar(menubar);
        statusbar = new QStatusBar(Frm_Main);
        statusbar->setObjectName("statusbar");
        Frm_Main->setStatusBar(statusbar);

        menubar->addAction(menuTasks->menuAction());
        menubar->addAction(menuLog->menuAction());
        menuTasks->addAction(actionGreenBox_Control_Monitor);
        menuTasks->addAction(actionSecApp_Task_Manager);
        menuTasks->addSeparator();
        menuTasks->addAction(actionAbout);
        menuLog->addAction(actionShow);

        retranslateUi(Frm_Main);

        QMetaObject::connectSlotsByName(Frm_Main);
    } // setupUi

    void retranslateUi(QMainWindow *Frm_Main)
    {
        Frm_Main->setWindowTitle(QCoreApplication::translate("Frm_Main", "GreenBox", nullptr));
        actionGreenBox_Control_Monitor->setText(QCoreApplication::translate("Frm_Main", "GreenBox Control Monitor", nullptr));
#if QT_CONFIG(shortcut)
        actionGreenBox_Control_Monitor->setShortcut(QCoreApplication::translate("Frm_Main", "Alt+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAbout->setText(QCoreApplication::translate("Frm_Main", "About", nullptr));
#if QT_CONFIG(shortcut)
        actionAbout->setShortcut(QCoreApplication::translate("Frm_Main", "Alt+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSecApp_Task_Manager->setText(QCoreApplication::translate("Frm_Main", "SecApp Task Manager", nullptr));
#if QT_CONFIG(shortcut)
        actionSecApp_Task_Manager->setShortcut(QCoreApplication::translate("Frm_Main", "Alt+M", nullptr));
#endif // QT_CONFIG(shortcut)
        actionShow->setText(QCoreApplication::translate("Frm_Main", "Show", nullptr));
#if QT_CONFIG(shortcut)
        actionShow->setShortcut(QCoreApplication::translate("Frm_Main", "Alt+S", nullptr));
#endif // QT_CONFIG(shortcut)
        btn_config_sos->setText(QCoreApplication::translate("Frm_Main", "Configure", nullptr));
        btn_run_sos->setText(QCoreApplication::translate("Frm_Main", "Run", nullptr));
        btn_stop_sos->setText(QCoreApplication::translate("Frm_Main", "Stop", nullptr));
        label->setText(QString());
        menuTasks->setTitle(QCoreApplication::translate("Frm_Main", "Tasks", nullptr));
        menuLog->setTitle(QCoreApplication::translate("Frm_Main", "Log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Frm_Main: public Ui_Frm_Main {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUI_MAIN_H
