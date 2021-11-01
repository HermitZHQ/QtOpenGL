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
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QListWidget *list_info;
    QSlider *slid_MouseSpeed;
    QLabel *label;
    QLabel *label_2;
    QSlider *slid_quant;
    QSlider *slid_water;
    QLabel *label_3;
    QSlider *slid_blur;
    QLabel *label_4;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(642, 443);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        list_info = new QListWidget(centralwidget);
        list_info->setObjectName(QString::fromUtf8("list_info"));
        list_info->setGeometry(QRect(0, 160, 641, 281));
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
        label_2->setGeometry(QRect(370, 40, 101, 21));
        slid_quant = new QSlider(centralwidget);
        slid_quant->setObjectName(QString::fromUtf8("slid_quant"));
        slid_quant->setGeometry(QRect(190, 40, 160, 22));
        slid_quant->setMinimum(1);
        slid_quant->setMaximum(100);
        slid_quant->setValue(5);
        slid_quant->setOrientation(Qt::Horizontal);
        slid_water = new QSlider(centralwidget);
        slid_water->setObjectName(QString::fromUtf8("slid_water"));
        slid_water->setGeometry(QRect(190, 70, 160, 22));
        slid_water->setMinimum(1);
        slid_water->setMaximum(100);
        slid_water->setValue(50);
        slid_water->setOrientation(Qt::Horizontal);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(370, 70, 101, 21));
        slid_blur = new QSlider(centralwidget);
        slid_blur->setObjectName(QString::fromUtf8("slid_blur"));
        slid_blur->setGeometry(QRect(190, 10, 160, 22));
        slid_blur->setMinimum(1);
        slid_blur->setMaximum(359);
        slid_blur->setValue(200);
        slid_blur->setOrientation(Qt::Horizontal);
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(370, 10, 101, 21));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);
        QObject::connect(slid_MouseSpeed, SIGNAL(valueChanged(int)), MainWindow, SLOT(OnSliderMouseSpeedChanged(int)));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QApplication::translate("MainWindow", "MouseSpeed", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "strength", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "sample num", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "angle", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
