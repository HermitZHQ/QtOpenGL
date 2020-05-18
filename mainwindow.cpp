#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "OpenWidget.h"
#include <QtOpenGL/QGLFormat>
#include "QLayout"
#include "QVBoxLayout"
#include "QResizeEvent"
#include "QPushButton"
#include "ShaderHelper.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, m_openWidgetPtr(Q_NULLPTR), m_layoutV(Q_NULLPTR)
{
	ui->setupUi(this);

	// bind this to shaderHelper
	ShaderHelper::Instance().SetMainWindow(this);

	m_openWidgetPtr = new OpenWidget();
	m_openWidgetPtr->resize(800, 600);
	m_openWidgetPtr->SetMainWndObj(this);
	m_openWidgetPtr->show();
	auto geo = m_openWidgetPtr->geometry();

	move(geo.x(), geo.y() + geo.height());
}

MainWindow::~MainWindow()
{
    delete ui;
}

Q_INVOKABLE void MainWindow::AddInfo(QString info)
{
	ui->list_info->addItem(info);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{

}

void MainWindow::OnBtnReloadShaders()
{
	if (Q_NULLPTR == m_openWidgetPtr) {
		return;
	}

	m_openWidgetPtr->ReloadShaders();
}

void MainWindow::OnSliderMouseSpeedChanged(int value)
{
	if (Q_NULLPTR == m_openWidgetPtr) {
		return;
	}

	m_openWidgetPtr->ChangeMouseMoveSpeed(value);
}
