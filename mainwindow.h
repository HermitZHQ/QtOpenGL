#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class OpenWidget;
class QTimer;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	Q_INVOKABLE void AddInfo(QString info);

protected:
	virtual void resizeEvent(QResizeEvent *event) override;

public slots:
	void OnBtnReloadShaders();
	void OnBtnStartClicked();
	void OnSliderMouseSpeedChanged(int value);

private:
    Ui::MainWindow *ui;
	OpenWidget						*m_openWidgetPtr;
	QLayout							*m_layoutV;
	QTimer							*m_delayStartTimer;

};
#endif // MAINWINDOW_H
