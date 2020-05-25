/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *btn_Specular;
    QPushButton *btn_Ambient;
    QListWidget *list_info;
    QSlider *slid_MouseSpeed;
    QLabel *label;
    QLabel *label_2;
    QSlider *slid_R;
    QSlider *slid_G;
    QSlider *slid_B;
    QSlider *slid_A;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(626, 389);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        btn_Specular = new QPushButton(centralwidget);
        btn_Specular->setObjectName(QString::fromUtf8("btn_Specular"));
        btn_Specular->setGeometry(QRect(190, 130, 111, 28));
        btn_Ambient = new QPushButton(centralwidget);
        btn_Ambient->setObjectName(QString::fromUtf8("btn_Ambient"));
        btn_Ambient->setGeometry(QRect(190, 100, 111, 28));
        list_info = new QListWidget(centralwidget);
        list_info->setObjectName(QString::fromUtf8("list_info"));
        list_info->setGeometry(QRect(310, 0, 321, 391));
        slid_MouseSpeed = new QSlider(centralwidget);
        slid_MouseSpeed->setObjectName(QString::fromUtf8("slid_MouseSpeed"));
        slid_MouseSpeed->setGeometry(QRect(10, 40, 160, 22));
        slid_MouseSpeed->setMinimum(1);
        slid_MouseSpeed->setMaximum(100);
        slid_MouseSpeed->setValue(15);
        slid_MouseSpeed->setOrientation(Qt::Horizontal);
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 91, 21));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 70, 101, 21));
        slid_R = new QSlider(centralwidget);
        slid_R->setObjectName(QString::fromUtf8("slid_R"));
        slid_R->setGeometry(QRect(10, 100, 160, 22));
        slid_R->setMinimum(0);
        slid_R->setMaximum(255);
        slid_R->setValue(230);
        slid_R->setOrientation(Qt::Horizontal);
        slid_G = new QSlider(centralwidget);
        slid_G->setObjectName(QString::fromUtf8("slid_G"));
        slid_G->setGeometry(QRect(10, 130, 160, 22));
        slid_G->setMinimum(0);
        slid_G->setMaximum(255);
        slid_G->setValue(230);
        slid_G->setOrientation(Qt::Horizontal);
        slid_B = new QSlider(centralwidget);
        slid_B->setObjectName(QString::fromUtf8("slid_B"));
        slid_B->setGeometry(QRect(10, 160, 160, 22));
        slid_B->setMinimum(0);
        slid_B->setMaximum(255);
        slid_B->setValue(230);
        slid_B->setOrientation(Qt::Horizontal);
        slid_A = new QSlider(centralwidget);
        slid_A->setObjectName(QString::fromUtf8("slid_A"));
        slid_A->setGeometry(QRect(10, 190, 160, 22));
        slid_A->setMinimum(0);
        slid_A->setMaximum(255);
        slid_A->setValue(230);
        slid_A->setOrientation(Qt::Horizontal);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);
        QObject::connect(slid_MouseSpeed, SIGNAL(valueChanged(int)), MainWindow, SLOT(OnSliderMouseSpeedChanged(int)));
        QObject::connect(btn_Ambient, SIGNAL(clicked()), MainWindow, SLOT(OnBtnSetAmbient()));
        QObject::connect(btn_Specular, SIGNAL(clicked()), MainWindow, SLOT(OnBtnSetSpecular()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        btn_Specular->setText(QApplication::translate("MainWindow", "SetSpecular", nullptr));
        btn_Ambient->setText(QApplication::translate("MainWindow", "SetAmbient", nullptr));
        label->setText(QApplication::translate("MainWindow", "MouseSpeed", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "Color", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
