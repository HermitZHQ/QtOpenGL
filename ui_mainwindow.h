/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
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
    QLabel *label_2;
    QSlider *slid_start_r;
    QSlider *slid_end_r;
    QSlider *slid_map_start_r;
    QSlider *slid_map_end_r;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;

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
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(200, 20, 101, 21));
        slid_start_r = new QSlider(centralwidget);
        slid_start_r->setObjectName(QString::fromUtf8("slid_start_r"));
        slid_start_r->setGeometry(QRect(20, 20, 160, 22));
        slid_start_r->setMinimum(1);
        slid_start_r->setMaximum(1000);
        slid_start_r->setValue(380);
        slid_start_r->setOrientation(Qt::Horizontal);
        slid_end_r = new QSlider(centralwidget);
        slid_end_r->setObjectName(QString::fromUtf8("slid_end_r"));
        slid_end_r->setGeometry(QRect(20, 50, 160, 22));
        slid_end_r->setMinimum(1);
        slid_end_r->setMaximum(1000);
        slid_end_r->setValue(600);
        slid_end_r->setOrientation(Qt::Horizontal);
        slid_map_start_r = new QSlider(centralwidget);
        slid_map_start_r->setObjectName(QString::fromUtf8("slid_map_start_r"));
        slid_map_start_r->setGeometry(QRect(20, 80, 160, 22));
        slid_map_start_r->setMinimum(1);
        slid_map_start_r->setMaximum(1000);
        slid_map_start_r->setValue(190);
        slid_map_start_r->setOrientation(Qt::Horizontal);
        slid_map_end_r = new QSlider(centralwidget);
        slid_map_end_r->setObjectName(QString::fromUtf8("slid_map_end_r"));
        slid_map_end_r->setGeometry(QRect(20, 110, 160, 22));
        slid_map_end_r->setMinimum(1);
        slid_map_end_r->setMaximum(1000);
        slid_map_end_r->setValue(450);
        slid_map_end_r->setOrientation(Qt::Horizontal);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(200, 50, 101, 21));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(200, 80, 101, 21));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(200, 110, 101, 21));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "start_r", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "end_r", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "map_start_r", nullptr));
        label_5->setText(QApplication::translate("MainWindow", "map_end_r", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
