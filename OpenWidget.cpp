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
	, m_matWorldLoc(0), m_worldCamPosLoc(0)
{
	m_cam = new Camera();

	connect(&m_updateKeyTimer, &QTimer::timeout, this, &OpenWidget::UpdateKeys);
	m_updateKeyTimer.setInterval(10);
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
	glCullFace(GL_BACK);
// 	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// we should init shader first, so we can get attri location from shader
	InitShaders();

	// test load model
	AssetImport::Instance().LoadModel("./models/02.obj");
	AssetImport::Instance().LoadModel("./models/plane.obj");
	AssetImport::Instance().LoadModel("./models/plane2.obj");
	Model *pMod = ModelMgr::Instance().FindModelByName("Plane001");
	if (Q_NULLPTR != pMod) {
		QMatrix4x4 mat;
		mat.translate(0, 15, 0);
		mat.rotate(180, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod->SetWroldMat(mat);
	}
	Model *pMod2 = ModelMgr::Instance().FindModelByName("Plane002");
	if (Q_NULLPTR != pMod2) {
		QMatrix4x4 mat;
		mat.translate(-30, 0, 0);
		mat.rotate(90, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod2->SetWroldMat(mat);
	}
}

void OpenWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}

void OpenWidget::paintGL()
{
	glColorMask(1, 1, 1, 1);
	glStencilMask(0xff);
	static const GLfloat black[] = { 0.278f, 0.278f, 0.278f, 0.0f };
	glClearBufferfv(GL_COLOR, 0, black);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
// 	glClearStencil(0);

	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);
		if (nullptr == mod) {
			continue;
		}

		// use the first model to be the stencil
		if (i == 0)	{
			glDepthMask(0);
			glColorMask(0, 0, 0, 0);

			glDisable(GL_DEPTH_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glStencilMask(0xff);
		}
		else {
			glEnable(GL_DEPTH_TEST);
			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glStencilMask(0x0);
		}
		
		auto meshNum = mod->GetMeshNum();
		for (unsigned int j = 0; j < meshNum; ++j)
		{
			Mesh *mesh = mod->GetMesh(j);
			mesh->SwitchShader(Mesh::Default);

			QMatrix4x4 matVP = m_cam->GetVPMatrix();
			QMatrix4x4 matModel = mod->GetWorldMat();
			QVector3D camPos = m_cam->GetCamPos().toVector3D();
			mesh->Draw(matVP, matModel, camPos);
		}

		// scale the cube and get the outline
		if (0 == i) {
			Model *pMod = ModelMgr::Instance().FindModelByName("Box001");

			glEnable(GL_DEPTH_TEST);
			glColorMask(1, 1, 1, 1);
			glDepthMask(1);

			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glStencilMask(0x0);

			for (int i = 0; i < pMod->GetMeshNum(); ++i)
			{
				auto mesh = pMod->GetMesh(i);
				mesh->SwitchShader(Mesh::Default);

				QMatrix4x4 matVP = m_cam->GetVPMatrix();
				QMatrix4x4 matModel = mod->GetWorldMat();
				QVector3D camPos = m_cam->GetCamPos().toVector3D();
				mesh->Draw(matVP, matModel, camPos);
			}

			glDisable(GL_DEPTH_TEST);
			glDepthMask(0);
			glColorMask(1, 1, 1, 1);
			glStencilFunc(GL_NOTEQUAL, 1, 0xff);
			glStencilMask(0x0);

			for (int i = 0; i < pMod->GetMeshNum(); ++i)
			{
				auto mesh = pMod->GetMesh(i);
				mesh->SwitchShader(Mesh::Shader1);

				QMatrix4x4 matVP = m_cam->GetVPMatrix();
				QMatrix4x4 matModel = mod->GetWorldMat();
				matModel.scale(1.05, 1.05, 1.05);
				QVector3D camPos = m_cam->GetCamPos().toVector3D();
				mesh->Draw(matVP, matModel, camPos);
			}

			glEnable(GL_DEPTH_TEST);
			glDepthMask(1);
			glStencilMask(0xff);
		}
	}
}

void OpenWidget::InitShaders()
{
}

GLuint OpenWidget::ReloadShaders()
{
	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *pMod = ModelMgr::Instance().GetModel(i);
		if (nullptr == pMod) {
			continue;
		}

		auto meshNum = pMod->GetMeshNum();
		for (unsigned int j = 0; j < meshNum; ++j)
		{
			Mesh *pMesh = pMod->GetMesh(j);
			if (nullptr == pMesh) {
				continue;
			}

			pMesh->SwitchShader(Mesh::Default);
		}
	}

	return 0;
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
