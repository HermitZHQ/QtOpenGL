#include "OpenWidget.h"
#include "QResizeEvent"
#include "ShaderHelper.h"
#include "PreDef.h"
#include "Camera.h"
#include "AssetImport.h"
#include "Mesh.h"
#include "Model.h"
#include "ModelMgr.h"


OpenWidget::OpenWidget()
	:m_mainObj(Q_NULLPTR), m_cam(Q_NULLPTR)
	, m_matWorldLoc(0), m_worldCamPosLoc(0), m_query(0), m_sampleNum(0)
{
	m_cam = new Camera();

	connect(&m_updateKeyTimer, &QTimer::timeout, this, &OpenWidget::UpdateKeys);
	m_updateKeyTimer.setInterval(1);
	m_updateKeyTimer.start();
}

OpenWidget::~OpenWidget()
{
	if (Q_NULLPTR != m_cam) {
		delete m_cam;
	}
}

void OpenWidget::initializeGL()
{
	initializeOpenGLFunctions();

	glEnable(GL_CULL_FACE);
// 	glCullFace(GL_BACK);
// 	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// test load model
	AssetImport::Instance().LoadModel("./models/Box001.obj");
	AssetImport::Instance().LoadModel("./models/Box002.obj");
	AssetImport::Instance().LoadModel("./models/plane.obj");
	AssetImport::Instance().LoadModel("./models/plane2.obj");
	AssetImport::Instance().LoadModel("./models/plane3.obj");
// 	AssetImport::Instance().LoadModel("./models/dva/001.obj");
	Model *pMod = ModelMgr::Instance().FindModelByName("Plane001");
	if (Q_NULLPTR != pMod) {
		QMatrix4x4 mat;
// 		mat.translate(0, 30, 0);
// 		mat.rotate(180, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod->SetWroldMat(mat);
	}
	Model *pMod2 = ModelMgr::Instance().FindModelByName("Plane002");
	if (Q_NULLPTR != pMod2) {
		QMatrix4x4 mat;
		mat.translate(-50, 0, 0);
		mat.rotate(90, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod2->SetWroldMat(mat);
	}
	Model *pMod3 = ModelMgr::Instance().FindModelByName("Plane003");
	if (Q_NULLPTR != pMod3) {
		QMatrix4x4 mat;
		mat.translate(50, 0, 0);
		mat.rotate(270, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod3->SetWroldMat(mat);
	}


	Model *pBox001 = ModelMgr::Instance().FindModelByName("Box001");
	if (Q_NULLPTR != pBox001) {
		QMatrix4x4 mat;
		mat.translate(0, 15, 0);
		pBox001->SetWroldMat(mat);
	}
	Model *pBox002 = ModelMgr::Instance().FindModelByName("Box002");
	if (Q_NULLPTR != pBox002) {
		QMatrix4x4 mat;
		mat.translate(0, -37, 0);
		pBox002->SetWroldMat(mat);
	}
}

void OpenWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}

void OpenWidget::paintClearAndReset()
{
	static GLuint fps = 0;
	static GLuint time = GetTickCount();

	// set the default value at the beginning, otherwise may cause the unexpected error
	glColorMask(1, 1, 1, 1);
	glStencilMask(0xff);
	glStencilFunc(GL_ALWAYS, 1, 0xff);
	glDepthMask(1);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	static const GLfloat black[] = { 0.278f, 0.278f, 0.278f, 0.0f };
	glClearBufferfv(GL_COLOR, 0, black);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	++fps;
	if (GetTickCount() - time >= 1000) {
		setWindowTitle(QString("fps:[%1]").arg(fps));
		time = GetTickCount();
		fps = 0;
	}
}

void OpenWidget::paintGL()
{
	paintClearAndReset();

	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QVector3D camPos = m_cam->GetCamPos().toVector3D();

	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);

		if (0 == i)	{
			// get the obj mask
			glStencilMask(0);
			glStencilFunc(GL_ALWAYS, 1, 0xff);

			glEnable(GL_CLIP_PLANE0);
			SwitchShader(ShaderHelper::ShaderPlaneClip);
		}
		else {
			glDisable(GL_CLIP_PLANE0);
			glStencilMask(0x0);
			glStencilFunc(GL_ALWAYS, 0, 0xff);
			SwitchShader(ShaderHelper::ShaderDefault);
		}


		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos);

		if (modelNum - 1 == i) {
			Model *pBox = ModelMgr::Instance().FindModelByName("Box001");

			SwitchShader(ShaderHelper::ShaderPlaneClip);
			glFrontFace(GL_CW);
			glEnable(GL_CLIP_PLANE0);
			glStencilMask(0xff);
			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glColorMask(0, 0, 0, 0);

			pBox->Draw(matVP, pBox->GetWorldMat(), camPos);
			glFrontFace(GL_CCW);
			glDisable(GL_CLIP_PLANE0);
			glColorMask(1, 1, 1, 1);

			glStencilMask(0);
			glStencilFunc(GL_EQUAL, 1, 0xff);
			SwitchShader(ShaderHelper::ShaderPureColor);
			pBox->Draw(matVP, pBox->GetWorldMat(), camPos);
		}
	}
}

void OpenWidget::SwitchShader(ShaderHelper::eShaderType type)
{
	ShaderHelper::Instance().SetShaderType(type);
}

void OpenWidget::BeginGetOcclusionSampleNum()
{
	m_query = 0;
	m_sampleNum = 0;
	glCreateQueries(GL_ANY_SAMPLES_PASSED, 1, &m_query);
	glBeginQuery(GL_ANY_SAMPLES_PASSED, m_query);
}

void OpenWidget::EndGetOcclusionSampleNum()
{
	glEndQuery(GL_ANY_SAMPLES_PASSED);

	GLint queryReady = 0, count = 100;
	do
	{
		glGetQueryObjectiv(m_query, GL_QUERY_RESULT_AVAILABLE, &queryReady);
	} while (!queryReady && --count);

	if (queryReady) {
		GLint sNum = 0;
		glGetQueryObjectiv(m_query, GL_QUERY_RESULT, &sNum);
		m_sampleNum += sNum;
	}
	else {
		m_sampleNum += 0;
	}

	glDeleteQueries(1, &m_query);
}

void OpenWidget::ChangeMouseMoveSpeed(int value)
{
	if (Q_NULLPTR != m_cam) {
		m_cam->SetMouseMoveSpeed(value);
	}
}

void OpenWidget::UpdateKeys()
{
	if (m_pressedKeyVec.contains(Qt::Key_W)) {
		m_cam->MoveForward();
	}
	if (m_pressedKeyVec.contains(Qt::Key_S)) {
		m_cam->MoveBackward();
	}
	if (m_pressedKeyVec.contains(Qt::Key_A)) {
		m_cam->MoveLeft();
	}
	if (m_pressedKeyVec.contains(Qt::Key_D)) {
		m_cam->MoveRight();
	}
	if (m_pressedKeyVec.contains(Qt::Key_Q)) {
		m_cam->MoveDown();
	}
	if (m_pressedKeyVec.contains(Qt::Key_E)) {
		m_cam->MoveUp();
	}

	update();
}

void OpenWidget::moveEvent(QMoveEvent *event)
{
	Q_UNUSED(event);
	if (Q_NULLPTR != m_mainObj) {
		auto geo = geometry();

		((MainWindow*)m_mainObj)->move(geo.x(), geo.y() + geo.height());
	}
}

void OpenWidget::keyPressEvent(QKeyEvent *event)
{
	if (!m_pressedKeyVec.contains((Qt::Key)event->key())) {
		m_pressedKeyVec.append((Qt::Key)event->key());
	}
}

void OpenWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (m_pressedKeyVec.contains((Qt::Key)event->key())) {
		m_pressedKeyVec.removeAll((Qt::Key)event->key());
	}
}

void OpenWidget::mousePressEvent(QMouseEvent *event)
{
	if (!m_cam->GetCamRotateEnable() && event->button() == Qt::MouseButton::RightButton) {
		m_cam->SetCamRotateEnable(true);
		m_cam->SetRotateStartPos(event->pos());
	}
}

void OpenWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MouseButton::RightButton) {
		m_cam->SetCamRotateEnable(false);
	}
}

void OpenWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (m_cam->GetCamRotateEnable()) {
		m_cam->SetRotateCurPos(event->pos());
		m_cam->UpdateRotateInfo();

		update();
	}
}
