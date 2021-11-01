#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "OpenWidget.h"
#include <QtOpenGL/QGLFormat>
#include "QLayout"
#include "QVBoxLayout"
#include "QResizeEvent"
#include "QPushButton"
#include "ShaderHelper.h"
#include "QTimer"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, m_openWidgetPtr(Q_NULLPTR), m_layoutV(Q_NULLPTR)
	, m_ambientColor(1, 1, 1, 1), m_specularColor(1, 1, 1, 1)
    , m_melt_threshold(0.0f)
{
	ui->setupUi(this);

	m_delayStartTimer = new QTimer(this);
	connect(m_delayStartTimer, &QTimer::timeout, this, &MainWindow::Start);
	m_delayStartTimer->setSingleShot(true);
	m_delayStartTimer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}

Q_INVOKABLE void MainWindow::AddInfo(QString info)
{
	ui->list_info->addItem(info);
	ui->list_info->scrollToBottom();

	if (ui->list_info->count() > 1000) {
		ui->list_info->clear();
	}
}

void MainWindow::resizeEvent(QResizeEvent *event)
{

}

QVector4D MainWindow::GetAmbientColor()
{
	return m_ambientColor;
}

QVector4D MainWindow::GetSpecularColor()
{
	return m_specularColor;
}

void MainWindow::Start()
{
	m_openWidgetPtr = new OpenWidget();
	m_openWidgetPtr->resize(800, 600);
	// 	m_openWidgetPtr->resize(580, 490);
	m_openWidgetPtr->move(515, 108);
	m_openWidgetPtr->SetMainWndObj(this);
	m_openWidgetPtr->show();
	auto geo = m_openWidgetPtr->geometry();

	move(0, 0);
}

void MainWindow::OnBtnSetSpecular()
{
// 	m_specularColor[0] = ui->slid_R->value() / 255.0f;
// 	m_specularColor[1] = ui->slid_G->value() / 255.0f;
// 	m_specularColor[2] = ui->slid_B->value() / 255.0f;
// 	m_specularColor[3] = ui->slid_A->value() / 255.0f;
}

void MainWindow::OnBtnSetAmbient()
{
// 	m_ambientColor[0] = ui->slid_R->value() / 255.0f;
// 	m_ambientColor[1] = ui->slid_G->value() / 255.0f;
// 	m_ambientColor[2] = ui->slid_B->value() / 255.0f;
// 	m_ambientColor[3] = ui->slid_A->value() / 255.0f;
}

void MainWindow::OnBtnStartDynamicCloud()
{
    if (Q_NULLPTR == m_openWidgetPtr) {
        return;
    }

    m_openWidgetPtr->StartDynamicCloud();
}

void MainWindow::OnSliderMouseSpeedChanged(int value)
{
	if (Q_NULLPTR == m_openWidgetPtr) {
		return;
	}

	m_openWidgetPtr->ChangeMouseMoveSpeed(value);
}

void MainWindow::OnSliderMeltChanged(int value)
{
    if (Q_NULLPTR == m_openWidgetPtr) {
        return;
    }

    m_openWidgetPtr->ChangeMeltThreshold(value / 100.0f);
}

void MainWindow::on_slid_quant_valueChanged(int value)
{
    if (Q_NULLPTR == m_openWidgetPtr) {
        return;
    }

    m_openWidgetPtr->ChangeInkQuant(value);
}

void MainWindow::on_slid_water_valueChanged(int value)
{
    if (Q_NULLPTR == m_openWidgetPtr) {
        return;
    }

    m_openWidgetPtr->ChangeInkWater(value);
}

void MainWindow::on_slid_blur_valueChanged(int value)
{
    if (Q_NULLPTR == m_openWidgetPtr) {
        return;
    }

    m_openWidgetPtr->ChangeMeltThreshold(value);
}
