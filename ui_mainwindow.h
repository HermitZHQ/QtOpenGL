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
    QPushButton *btn_reloadShader;
    QPushButton *btn_Start;
    QListWidget *list_info;
    QSlider *horizontalSlider;
    QLabel *label;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(619, 262);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        btn_reloadShader = new QPushButton(centralwidget);
        btn_reloadShader->setObjectName(QString::fromUtf8("btn_reloadShader"));
        btn_reloadShader->setGeometry(QRect(10, 50, 111, 28));
        btn_Start = new QPushButton(centralwidget);
        btn_Start->setObjectName(QString::fromUtf8("btn_Start"));
        btn_Start->setGeometry(QRect(10, 10, 111, 28));
        list_info = new QListWidget(centralwidget);
        list_info->setObjectName(QString::fromUtf8("list_info"));
        list_info->setGeometry(QRect(0, 90, 621, 171));
        horizontalSlider = new QSlider(centralwidget);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(140, 40, 160, 22));
        horizontalSlider->setMinimum(1);
        horizontalSlider->setMaximum(100);
        horizontalSlider->setValue(15);
        horizontalSlider->setOrientation(Qt::Horizontal);
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(140, 10, 91, 21));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);
        QObject::connect(btn_reloadShader, SIGNAL(clicked()), MainWindow, SLOT(OnBtnReloadShaders()));
        QObject::connect(horizontalSlider, SIGNAL(valueChanged(int)), MainWindow, SLOT(OnSliderMouseSpeedChanged(int)));
        QObject::connect(btn_Start, SIGNAL(clicked()), MainWindow, SLOT(OnBtnStartClicked()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        btn_reloadShader->setText(QApplication::translate("MainWindow", "ReloadShader", nullptr));
        btn_Start->setText(QApplication::translate("MainWindow", "Start", nullptr));
        label->setText(QApplication::translate("MainWindow", "MouseSpeed", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
