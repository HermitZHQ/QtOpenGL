#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QVector4D"

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
	QVector4D GetAmbientColor();
	QVector4D GetSpecularColor();

protected:
	virtual void resizeEvent(QResizeEvent *event) override;

public slots:
	void Start();
	void OnBtnSetSpecular();
	void OnBtnSetAmbient();
    void OnBtnStartDynamicCloud();
	void OnSliderMouseSpeedChanged(int value);
    void OnSliderMeltChanged(int value);

private:
    Ui::MainWindow *ui;
	OpenWidget						*m_openWidgetPtr;
	QLayout							*m_layoutV;
	QTimer							*m_delayStartTimer;
	QVector4D						m_ambientColor;
	QVector4D						m_specularColor;
    float                           m_melt_threshold;
};
#endif // MAINWINDOW_H
