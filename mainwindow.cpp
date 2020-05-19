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
{
	ui->setupUi(this);

	m_delayStartTimer = new QTimer(this);
	connect(m_delayStartTimer, &QTimer::timeout, this, &MainWindow::OnBtnStartClicked);
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

void MainWindow::OnBtnStartClicked()
{
	m_openWidgetPtr = new OpenWidget();
	m_openWidgetPtr->resize(800, 600);
	m_openWidgetPtr->move(600, 100);
	m_openWidgetPtr->SetMainWndObj(this);
	m_openWidgetPtr->show();
	auto geo = m_openWidgetPtr->geometry();

	move(geo.x(), geo.y() + geo.height());
}

void MainWindow::OnSliderMouseSpeedChanged(int value)
{
	if (Q_NULLPTR == m_openWidgetPtr) {
		return;
	}

	m_openWidgetPtr->ChangeMouseMoveSpeed(value);
}
