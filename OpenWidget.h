#pragma once
#include "QOpenGLWidget"
#include "QOpenGLFunctions_4_5_Core"
#include "QOpenGLFunctions_4_5_Compatibility"
#include "QGLWidget"
#include "QTimer"

class Camera;
class Mesh;
class OpenWidget : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
public:
	OpenWidget();
	~OpenWidget();

	void SetMainWndObj(QObject *obj) {
		m_mainObj = obj;
	}
	GLuint ReloadShaders();
	void ChangeMouseMoveSpeed(int value);
	void UpdateKeys();

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

	void InitShaders();

	void AddTipInfo(QString info);
	virtual void moveEvent(QMoveEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
	QObject						*m_mainObj;
	GLint						m_matMVPLoc;
	GLint						m_matWorldLoc;
	GLint						m_worldCamPosLoc;
	Camera						*m_cam;

	QTimer						m_updateKeyTimer;
	QVector<Qt::Key>			m_pressedKeyVec;
};

