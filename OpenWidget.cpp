#include "OpenWidget.h"
#include "QResizeEvent"
#include "ShaderHelper.h"
#include "PreDef.h"
#include "mainwindow.h"
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

	glClearStencil(0x0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	// we should init shader first, so we can get attri location from shader
	InitShaders();

	// test load model
	AssetImport::Instance().LoadModel("./models/02.obj");
	AssetImport::Instance().LoadModel("./models/plane.obj");
	Model *pMod = ModelMgr::Instance().FindModelByName("Plane001");
	if (Q_NULLPTR != pMod) {
		QMatrix4x4 mat;
		mat.translate(0, 30, 0);
		mat.rotate(180, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod->SetWroldMat(mat);
	}

// 	glEnable(GL_SCISSOR_TEST);
// 	glScissor(0, 0, 200, 200);
}

void OpenWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}

void OpenWidget::paintGL()
{
	static const GLfloat black[] = { 0.278f, 0.278f, 0.278f, 0.0f };
	glClearBufferfv(GL_COLOR, 0, black);
	glClear(GL_DEPTH_BUFFER_BIT);

	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
// 	drawSphere();

	glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
// 	drawTori();

	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);
		if (nullptr == mod) {
			continue;
		}
		
		auto meshNum = mod->GetMeshNum();
		for (unsigned int j = 0; j < meshNum; ++j)
		{
			Mesh *mesh = mod->GetMesh(j);
			if (nullptr == mesh) {
				continue;
			}

#ifdef ENABLE_TEX
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh->GetTexture1());
#ifdef ENABLE_NORMALMAP
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mesh->GetTextureNormalMap());
#endif
#endif
			glBindVertexArray(mesh->GetVao());


			QMatrix4x4 matVP = m_cam->GetVPMatrix();
			QMatrix4x4 matModel = mod->GetWorldMat();
// 			static QMatrix4x4 matModel;
// 			matModel.rotate(0.1f, QVector3D(0, 1, 0));
			matVP = matVP * matModel;
			glUniformMatrix4fv(m_matMVPLoc, 1, GL_FALSE, matVP.data());
			glUniformMatrix4fv(m_matWorldLoc, 1, GL_FALSE, matModel.data());
			glUniform3f(m_worldCamPosLoc, m_cam->GetCamPos().x(), m_cam->GetCamPos().y(), m_cam->GetCamPos().z());

			// multi instances test
			const static int instanceNum = 10;
// 			if (mesh->GetInstancesBufferId() != 0)
			{
// 				glBindBuffer(GL_ARRAY_BUFFER, mesh->GetInstancesBufferId());
				glBindBuffer(GL_TEXTURE_BUFFER, mesh->GetTextureBuffer1());
				void *pBuf = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
// 				pBuf = (GLchar*)pBuf + mesh->GetMultiInstanceModelMatrixOffset();

				if (nullptr != pBuf)
				{
					int xOffset = 0;
					GLfloat *pStartAddr = (GLfloat*)pBuf;
					for (int i = 0; i < instanceNum; ++i)
					{
						// set the different matrix to instances
						QMatrix4x4 modelMat;
						modelMat.setToIdentity();
						modelMat.translate(xOffset, 0, 0);
	
						memcpy_s(pStartAddr, sizeof(GLfloat) * 16, modelMat.data(), sizeof(GLfloat) * 16);
						pStartAddr += 16;

						xOffset += 25;
					}
	
					glUnmapBuffer(GL_ARRAY_BUFFER);
					// 					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindBuffer(GL_TEXTURE_BUFFER, 0);
				}
			}

			// Draw element(with indices)
			// 	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
// 			glDrawElements(GL_TRIANGLES, mesh->GetIndicesNum(), GL_UNSIGNED_INT, 0);
			glDrawElementsInstanced(GL_TRIANGLES, mesh->GetIndicesNum(), GL_UNSIGNED_INT, 0, instanceNum);
		}
	}
}

void OpenWidget::InitShaders()
{
	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/triangle.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/triangle.frag"}
	};
	GLuint program = ShaderHelper::Instance().LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));
	if (program != 0) {
		AddTipInfo(Q8("读取shader文件成功----"));
	}
	else {
		AddTipInfo(Q8("读取shader文件失败----"));
	}
	ShaderHelper::Instance().Use();

	m_matMVPLoc = ShaderHelper::Instance().GetUniformLocation("mat_mvp");
	m_matWorldLoc = ShaderHelper::Instance().GetUniformLocation("mat_world");
	m_worldCamPosLoc = ShaderHelper::Instance().GetUniformLocation("worldCamPos");

	auto texId = ShaderHelper::Instance().GetUniformLocation("tex");
	auto normalMapId = ShaderHelper::Instance().GetUniformLocation("normalMap");
	glUniform1i(texId, 0);
	glUniform1i(normalMapId, 1);

	if (-1 == m_matMVPLoc || -1 == m_matWorldLoc
		|| -1 == m_worldCamPosLoc) {
		AddTipInfo(Q8("查询uniform失败！"));
	}
}

GLuint OpenWidget::ReloadShaders()
{
	ShaderHelper::Instance().Unuse();
	ShaderHelper::ShaderInfo info[] = {
	{GL_VERTEX_SHADER, "./shaders/triangle.vert"},
	{GL_FRAGMENT_SHADER, "./shaders/triangle.frag"}
	};
	GLuint program = ShaderHelper::Instance().LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));
	if (program != 0) {
		AddTipInfo(Q8("读取shader文件成功----"));
	}
	else {
		AddTipInfo(Q8("读取shader文件失败----"));
	}

	ShaderHelper::Instance().Use();

	return program;
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

void OpenWidget::AddTipInfo(QString info)
{
	if (Q_NULLPTR != m_mainObj) {
		QMetaObject::invokeMethod(m_mainObj, "AddInfo", Q_ARG(QString, info));
	}
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
