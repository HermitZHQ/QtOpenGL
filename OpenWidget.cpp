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


OpenWidget::OpenWidget()
	:m_mainObj(Q_NULLPTR), m_cam(Q_NULLPTR)
	, m_matWorldLoc(0), m_worldCamPosLoc(0), m_query(0), m_sampleNum(0)
	, m_fb()
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

GLuint vao = 0;
GLuint texId = 0;
void OpenWidget::initializeGL()
{
	initializeOpenGLFunctions();

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

	// test load model
	AssetImport::Instance().LoadModel("./models/Box001.obj");
	AssetImport::Instance().LoadModel("./models/Box002.obj");
	AssetImport::Instance().LoadModel("./models/plane.obj");
	AssetImport::Instance().LoadModel("./models/plane2.obj");
	AssetImport::Instance().LoadModel("./models/plane3.obj");
// 	AssetImport::Instance().LoadModel("./models/teapot.obj");
	AssetImport::Instance().LoadModel("./models/skybox.obj");
// 	AssetImport::Instance().LoadModel("./models/dva/001.obj");
	Model *pMod = ModelMgr::Instance().FindModelByName("Plane001");
	if (Q_NULLPTR != pMod) {
// 		pMod->EnableProjTex();
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
		pBox001->EnableProjTex();
// 		pBox001->SetDrawType(Mesh::Point);
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

	Model *pTeapot = ModelMgr::Instance().FindModelByName("defaultobject");
	if (Q_NULLPTR != pTeapot) {
		QMatrix4x4 mat;
		mat.translate(20, 0, 0);
		mat.scale(5);
		pTeapot->SetWroldMat(mat);
	}	
	Model *pSkybox = ModelMgr::Instance().FindModelByName("skybox");
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
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
// 	auto wndSize = size();
// 	glViewport(0, 0, wndSize.width(), wndSize.height());
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}
	paintClearAndReset();

	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
	QMatrix4x4 matView = m_cam->GetViewMatrix();
	QVector3D camPos = m_cam->GetCamPos().toVector3D();

	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);
		SwitchShader(ShaderHelper::Default);

// 		SwitchShader(ShaderHelper::Decal);
		if (mod->GetModelName().compare("Box001") == 0) {
// 			SwitchShader(ShaderHelper::Decal);
		}
		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);

		if (modelNum - 1 == i) {
			Model *pBox = ModelMgr::Instance().FindModelByName("Box001");

// 			SwitchShader(ShaderHelper::PlaneClip);
// 			glFrontFace(GL_CW);
// 			glEnable(GL_CLIP_PLANE0);
// 			glStencilMask(0xff);
// 			glStencilFunc(GL_ALWAYS, 1, 0xff);
// 			glColorMask(0, 0, 0, 0);
// 
// 			pBox->Draw(matVP, pBox->GetWorldMat(), camPos);
// 			glFrontFace(GL_CCW);
// 			glDisable(GL_CLIP_PLANE0);
// 			glColorMask(1, 1, 1, 1);
// 
// 			glStencilMask(0);
// 			glStencilFunc(GL_EQUAL, 1, 0xff);
// 			SwitchShader(ShaderHelper::PureColor);
// 			pBox->Draw(matVP, pBox->GetWorldMat(), camPos);
		}
	}

	//-----------------------------------------------
	// test the frame buffer
	DrawOffScreenTexture();
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
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_oldFb);
	if (m_oldFb != 1) {
		AddTipInfo(Q8("帧缓存创建错误，应该在主窗口创建之后再创建"));
	}
	glCreateFramebuffers(1, &m_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fb);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_texOffscreenId);
	glBindTexture(GL_TEXTURE_2D, m_texOffscreenId);
	auto wndSize = this->size();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wndSize.width(), wndSize.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTextureParameteri(m_texOffscreenId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_texOffscreenId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// bind texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texOffscreenId, 0);

	// bind render buffer for depth test and stencil test
	GLuint rb = 0;
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wndSize.width(), wndSize.height());
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb);

	glBindFramebuffer(GL_FRAMEBUFFER, m_oldFb);
}

void OpenWidget::DrawOffScreenTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_oldFb);
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
	glBindTexture(GL_TEXTURE_2D, m_texOffscreenId);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
// 	glEnable(GL_DEPTH_TEST);
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
