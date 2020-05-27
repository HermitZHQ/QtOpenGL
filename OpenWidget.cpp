#include "OpenWidget.h"
#include "QResizeEvent"
#include "ShaderHelper.h"
#include "PreDef.h"
#include "Camera.h"
#include "AssetImport.h"
#include "Mesh.h"
#include "Model.h"
#include "ModelMgr.h"
#include "TextureMgr.h"
#include "LightMgr.h"


OpenWidget::OpenWidget()
	:m_mainObj(Q_NULLPTR), m_cam(Q_NULLPTR)
	, m_matWorldLoc(0), m_worldCamPosLoc(0), m_query(0), m_sampleNum(0)
	, m_offScreenFbo(), m_shadowTexWidth(2048), m_shadowTexHeight(2048)
	, m_shaderHelperPtr(nullptr)
{
	m_cam = new Camera();

	m_lightMgrPtr = &LightMgr::Instance();
	m_assimpPtr = &AssetImport::Instance();
	m_modelMgrPtr = &ModelMgr::Instance();

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

GLuint vao = 0;
GLuint texId = 0;
void OpenWidget::initializeGL()
{
	initializeOpenGLFunctions();

	m_shaderHelperPtr = &ShaderHelper::Instance();
	// test for a quad
	{
// 		const GLfloat g_vertices[6][2] = {
// 			{-0.9f, -0.9f}, {0.85f, -0.9f}, {-0.9f, 0.85f}, // first triangle
// 			{0.9f, -0.85f}, {0.9f, 0.9f}, {-0.85f, 0.9f}, // second triangle
// 		};
		const GLfloat g_vertices[6][2] = {
			{-1, -1}, {1, -1}, {-1, 1}, // first triangle
			{1, -1}, {1, 1}, {-1, 1}, // second triangle
		};

		const GLfloat g_uvs[6][2] = {
			{0, 1}, {1, 1}, {0, 0}, //
			{1, 1}, {1, 0}, {0, 0}
		};

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		texId = TextureMgr::Instance().LoadTexture("./textures/container.jpg");
		SwitchShader(ShaderHelper::FrameBuffer1);
		auto loc = ShaderHelper::Instance().GetUniformLocation("tex");
		glUniform1i(loc, 0);

		GLuint vbo = 0;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 12, g_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, sizeof(GLfloat) * 12, g_uvs);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 12));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glEnable(GL_CULL_FACE);
// 	glCullFace(GL_BACK);
// 	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
// 	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	TextureMgr::Instance();

	// load the light box
	auto lightNum = LightMgr::Instance().GetCurLightNum();
	for (int i = 0; i < lightNum; ++i)
	{
		auto &lightInfo = LightMgr::Instance().GetLightInfo(i);
		if (lightInfo.isEnabled) {
			QMatrix4x4 matModel;
			matModel.translate(lightInfo.pos);
			matModel.scale(0.1f);
			auto mod = m_assimpPtr->LoadModelWithModelMatrixAndShaderType("./models/LightBox.obj", matModel, ShaderHelper::Diffuse);
			lightInfo.SetModel(mod);
		}
	}

	// test load model
	m_assimpPtr->LoadModel("./models/Box001.obj");
	m_assimpPtr->LoadModel("./models/Box002.obj");
	m_assimpPtr->LoadModel("./models/plane.obj");
	m_assimpPtr->LoadModel("./models/plane2.obj");
	m_assimpPtr->LoadModel("./models/plane3.obj");
// 	m_assimpPtr->LoadModel("./models/teapot.obj");
	m_assimpPtr->LoadModel("./models/skybox.obj");
// 	m_assimpPtr->LoadModel("./models/dva/001.obj");
	Model *pMod = m_modelMgrPtr->FindModelByName("Plane001");
	if (Q_NULLPTR != pMod) {
// 		pMod->EnableProjTex();
		QMatrix4x4 mat;
// 		mat.translate(0, 30, 0);
// 		mat.rotate(180, QVector3D(0, 0, 1));
		mat.scale(40, 40, 40);
		pMod->SetWroldMat(mat);
	}
	Model *pMod2 = m_modelMgrPtr->FindModelByName("Plane002");
	if (Q_NULLPTR != pMod2) {
		QMatrix4x4 mat;
		mat.translate(-90, 0, 0);
		mat.rotate(90, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod2->SetWroldMat(mat);
	}
	Model *pMod3 = m_modelMgrPtr->FindModelByName("Plane003");
	if (Q_NULLPTR != pMod3) {
		QMatrix4x4 mat;
		mat.translate(90, 0, 0);
		mat.rotate(270, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod3->SetWroldMat(mat);
	}


	Model *pBox001 = m_modelMgrPtr->FindModelByName("Box001");
	if (Q_NULLPTR != pBox001) {
// 		pBox001->EnableProjTex();
// 		pBox001->SetDrawType(Mesh::Point);
		QMatrix4x4 mat;
		mat.translate(0, 15, 0);
		pBox001->SetWroldMat(mat);
	}
	Model *pBox002 = m_modelMgrPtr->FindModelByName("Box002");
	if (Q_NULLPTR != pBox002) {
		QMatrix4x4 mat;
		mat.translate(0, -37, 0);
		pBox002->SetWroldMat(mat);
	}

	Model *pTeapot = m_modelMgrPtr->FindModelByName("defaultobject");
	if (Q_NULLPTR != pTeapot) {
		QMatrix4x4 mat;
		mat.translate(20, 0, 0);
		mat.scale(5);
		pTeapot->SetWroldMat(mat);
	}	
	Model *pSkybox = m_modelMgrPtr->FindModelByName("skybox");
	if (Q_NULLPTR != pSkybox) {
		pSkybox->EnableSkybox();

		QMatrix4x4 mat;
// 		mat.scale(500);
		pSkybox->SetWroldMat(mat);
	}
}

void OpenWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	m_cam->SetAspectRatio((float)w / (float)h);
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
	static int i = 0;
	if (0 == i){
		++i;
		CreateOffScreenFrameBufferTexture();
		CreateShadowMapFrameBufferTexture();
	}

// 	glBindFramebuffer(GL_FRAMEBUFFER, m_offScreenFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFbo);
// 	glDrawBuffer(GL_NONE);
// 	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}
	paintClearAndReset();
	glViewport(0, 0, m_shadowTexWidth, m_shadowTexHeight);

	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
	QMatrix4x4 matView = m_cam->GetViewMatrix();
	QVector3D camPos = m_cam->GetCamPos().toVector3D();

	//for light view
	matVP = matProj * m_cam->GetLightViewMatrix();
	matView = m_cam->GetLightViewMatrix();

	// update light info dynamicly
	auto lightNum = m_lightMgrPtr->GetCurLightNum();
	float radius = 20.0f;
	static float radian = 0.1f;
	for (int i = 0; i < lightNum; ++i)
	{
		auto &lightInfo = m_lightMgrPtr->GetLightInfo(i);
		if (!lightInfo.isDirectional && !lightInfo.isPoint) {
			lightInfo.pos = camPos;
			lightInfo.dir = m_cam->GetViewDir();
			m_shaderHelperPtr->SetLightsInfo(lightInfo, i);
		}
		else if (!lightInfo.isDirectional && lightInfo.isPoint) {
// 			lightInfo.pos = QVector3D(qCos(radian) * radius, 0, qSin(radian) * radius);
// 			m_shaderHelperPtr->SetLightsInfo(lightInfo, i);
// 
// 			QMatrix4x4 matWorld;
// 			matWorld.translate(lightInfo.pos);
// 			matWorld.scale(0.1f);
// 			lightInfo.model->SetWroldMat(matWorld);
// 
// 			radius += 30.0f;
		}
	}
	radian += 0.01f;

// 	glCullFace(GL_FRONT);
	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);

		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}
// 	glCullFace(GL_BACK);

	//-----------------------------------------------
	// test the frame buffer
// 	DrawOffScreenTexture();
// 	DrawShadowMapTexture_ForTest();
	glViewport(0, 0, size().width(), size().height());
	DrawOriginalSceneWithShadow();
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

void OpenWidget::CreateOffScreenFrameBufferTexture()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_originalFbo);
	if (m_originalFbo != 1) {
		AddTipInfo(Q8("OffScreen帧缓存创建错误，应该在主窗口创建之后再创建"));
	}
	glCreateFramebuffers(1, &m_offScreenFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_offScreenFbo);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_offScreenTexId);
	glBindTexture(GL_TEXTURE_2D, m_offScreenTexId);
	auto wndSize = this->size();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wndSize.width(), wndSize.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTextureParameteri(m_offScreenTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_offScreenTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// bind texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_offScreenTexId, 0);

	// bind render buffer for depth test and stencil test
	GLuint rb = 0;
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wndSize.width(), wndSize.height());
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb);

	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
}

void OpenWidget::DrawOffScreenTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
// 	auto wndSize = size();
// 	glViewport(0, 0, wndSize.width(), wndSize.height());
	// check if we are good to go
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	// clear the framebuffer
	static const GLfloat black[] = { 0.278f, 0.278f, 0.278f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, black);

// 	glDisable(GL_DEPTH_TEST);
	SwitchShader(ShaderHelper::FrameBuffer1);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_offScreenTexId);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
// 	glEnable(GL_DEPTH_TEST);
}

void OpenWidget::CreateShadowMapFrameBufferTexture()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_originalFbo);
	if (m_originalFbo != 1) {
		AddTipInfo(Q8("ShadowMap帧缓存创建错误，应该在主窗口创建之后再创建"));
	}
	glCreateFramebuffers(1, &m_shadowMapFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFbo);

	auto wndSize = size();
	glGenTextures(1, &m_shadowMapTexId);
	glBindTexture(GL_TEXTURE_2D, m_shadowMapTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowTexWidth, m_shadowTexHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	static float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMapTexId, 0);
// 	glDrawBuffer(GL_NONE);
// 	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
}

void OpenWidget::DrawShadowMapTexture_ForTest()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);

	// check if we are good to go
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	glDrawBuffer(GL_FRONT_AND_BACK);
	glReadBuffer(GL_FRONT_AND_BACK);
	// clear the framebuffer
	glClear(GL_DEPTH_BUFFER_BIT);

	SwitchShader(ShaderHelper::FrameBuffer1);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_shadowMapTexId);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}

void OpenWidget::DrawOriginalSceneWithShadow()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);

	// check if we are good to go
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	paintClearAndReset();
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
	QMatrix4x4 matView = m_cam->GetViewMatrix();
	QVector3D camPos = m_cam->GetCamPos().toVector3D();

	//for light view
	QMatrix4x4 matLightVP = matProj * m_cam->GetLightViewMatrix();

	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);

		SwitchShader(ShaderHelper::Default);
		ShaderHelper::Instance().SetLightVPMat(matLightVP);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_shadowMapTexId);
		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}
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
	if (m_pressedKeyVec.contains(Qt::Key_Shift)) {
		m_cam->SetSpeedBoost(true);
	}
	else {
		m_cam->SetSpeedBoost(false);
	}

	update();
}

void OpenWidget::moveEvent(QMoveEvent *event)
{
	Q_UNUSED(event);
// 	if (Q_NULLPTR != m_mainObj) {
// 		auto geo = geometry();
// 
// 		((MainWindow*)m_mainObj)->move(geo.x(), geo.y() + geo.height());
// 	}
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
