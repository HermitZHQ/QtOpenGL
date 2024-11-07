/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
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
    QLabel *label_3;
    QLineEdit *edt_light_dir_x;
    QLineEdit *edt_light_dir_y;
    QLineEdit *edt_light_dir_z;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLineEdit *edt_light_pos_x;
    QLineEdit *edt_light_pos_z;
    QLineEdit *edt_light_pos_y;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(642, 443);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        btn_Specular = new QPushButton(centralwidget);
        btn_Specular->setObjectName(QString::fromUtf8("btn_Specular"));
        btn_Specular->setGeometry(QRect(370, 70, 111, 28));
        btn_Ambient = new QPushButton(centralwidget);
        btn_Ambient->setObjectName(QString::fromUtf8("btn_Ambient"));
        btn_Ambient->setGeometry(QRect(370, 40, 111, 28));
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
        label_2->setGeometry(QRect(190, 10, 101, 21));
        slid_R = new QSlider(centralwidget);
        slid_R->setObjectName(QString::fromUtf8("slid_R"));
        slid_R->setGeometry(QRect(190, 40, 160, 22));
        slid_R->setMinimum(0);
        slid_R->setMaximum(255);
        slid_R->setValue(230);
        slid_R->setOrientation(Qt::Horizontal);
        slid_G = new QSlider(centralwidget);
        slid_G->setObjectName(QString::fromUtf8("slid_G"));
        slid_G->setGeometry(QRect(190, 70, 160, 22));
        slid_G->setMinimum(0);
        slid_G->setMaximum(255);
        slid_G->setValue(230);
        slid_G->setOrientation(Qt::Horizontal);
        slid_B = new QSlider(centralwidget);
        slid_B->setObjectName(QString::fromUtf8("slid_B"));
        slid_B->setGeometry(QRect(190, 100, 160, 22));
        slid_B->setMinimum(0);
        slid_B->setMaximum(255);
        slid_B->setValue(230);
        slid_B->setOrientation(Qt::Horizontal);
        slid_A = new QSlider(centralwidget);
        slid_A->setObjectName(QString::fromUtf8("slid_A"));
        slid_A->setGeometry(QRect(190, 130, 160, 22));
        slid_A->setMinimum(0);
        slid_A->setMaximum(255);
        slid_A->setValue(230);
        slid_A->setOrientation(Qt::Horizontal);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 70, 131, 21));
        edt_light_dir_x = new QLineEdit(centralwidget);
        edt_light_dir_x->setObjectName(QString::fromUtf8("edt_light_dir_x"));
        edt_light_dir_x->setGeometry(QRect(10, 90, 31, 20));
        edt_light_dir_y = new QLineEdit(centralwidget);
        edt_light_dir_y->setObjectName(QString::fromUtf8("edt_light_dir_y"));
        edt_light_dir_y->setGeometry(QRect(10, 110, 31, 20));
        edt_light_dir_z = new QLineEdit(centralwidget);
        edt_light_dir_z->setObjectName(QString::fromUtf8("edt_light_dir_z"));
        edt_light_dir_z->setGeometry(QRect(10, 130, 31, 20));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(50, 90, 16, 21));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(50, 110, 16, 21));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(50, 130, 16, 21));
        edt_light_pos_x = new QLineEdit(centralwidget);
        edt_light_pos_x->setObjectName(QString::fromUtf8("edt_light_pos_x"));
        edt_light_pos_x->setGeometry(QRect(90, 90, 31, 20));
        edt_light_pos_z = new QLineEdit(centralwidget);
        edt_light_pos_z->setObjectName(QString::fromUtf8("edt_light_pos_z"));
        edt_light_pos_z->setGeometry(QRect(90, 130, 31, 20));
        edt_light_pos_y = new QLineEdit(centralwidget);
        edt_light_pos_y->setObjectName(QString::fromUtf8("edt_light_pos_y"));
        edt_light_pos_y->setGeometry(QRect(90, 110, 31, 20));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);
        QObject::connect(slid_MouseSpeed, SIGNAL(valueChanged(int)), MainWindow, SLOT(OnSliderMouseSpeedChanged(int)));
        QObject::connect(btn_Ambient, SIGNAL(clicked()), MainWindow, SLOT(OnBtnSetAmbient()));
        QObject::connect(btn_Specular, SIGNAL(clicked()), MainWindow, SLOT(OnBtnSetSpecular()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        btn_Specular->setText(QCoreApplication::translate("MainWindow", "SetSpecular", nullptr));
        btn_Ambient->setText(QCoreApplication::translate("MainWindow", "SetAmbient", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "MouseSpeed", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Color", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "DirLight Dir & Pos", nullptr));
        edt_light_dir_x->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        edt_light_dir_y->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        edt_light_dir_z->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "X", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Y", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Z", nullptr));
        edt_light_pos_x->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        edt_light_pos_z->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        edt_light_pos_y->setText(QCoreApplication::translate("MainWindow", "50", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
