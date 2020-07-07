#include "OpenWidget.h"
#include "QResizeEvent"
#include "QRandomGenerator"
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
	, m_offScreenFbo(0), m_shadowMapFbo(0), m_shadowTexWidth(4096), m_shadowTexHeight(4096)
	, m_shaderHelperPtr(nullptr)
	, m_gBufferFbo(0), m_gBufferPosTex(0), m_gBufferNormalTex(0), m_gBufferAlbedoTex(0), m_gBufferSkyboxTex(0)
	, m_gBufferDepthTex(0)
	, m_ssaoFbo(0), m_ssaoTex(0)
	, m_ssaoBlurFbo(0), m_ssaoBlurTex(0)
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

GLuint vao_geom;
void OpenWidget::TestGeometryPoints()
{
	glGenVertexArrays(1, &vao_geom);
	glBindVertexArray(vao_geom);

	const float vertices[4][2] = {
		{-0.5f, 0.5f}, {0.5f, 0.5f},
		{-0.5f, -0.5f}, {0.5f, -0.5f}
	};

	const float colors[4][3] = {
		{1, 0, 0}, {0, 1, 0},
		{1, 1, 0}, {1, 1, 1},
	};

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(vertices));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GLuint vao_quad = 0;
GLuint texId = 0;
void OpenWidget::initializeGL()
{
	initializeOpenGLFunctions();

	m_shaderHelperPtr = &ShaderHelper::Instance();
	TestGeometryPoints();
	// test for a quad
	{
		const GLfloat g_vertices[6][2] = {
			{-0.95f, -0.95f}, {0.92f, -0.95f}, {-0.95f, 0.92f}, // first triangle
			{0.95f, -0.92f}, {0.95f, 0.95f}, {-0.92f, 0.95f}, // second triangle
		};
// 		const GLfloat g_vertices[6][2] = {
// 			{-1, -1}, {1, -1}, {-1, 1}, // first triangle
// 			{1, -1}, {1, 1}, {-1, 1}, // second triangle
// 		};

		const GLfloat g_uvs[6][2] = {
			{0, 1}, {1, 1}, {0, 0}, //
			{1, 1}, {1, 0}, {0, 0}
		};

		glGenVertexArrays(1, &vao_quad);
		glBindVertexArray(vao_quad);

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

// 	glEnable(GL_CULL_FACE);
// 	glCullFace(GL_BACK);
// 	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
// 	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	TextureMgr::Instance();

	// load the light box
// 	auto lightNum = LightMgr::Instance().GetCurLightNum();
// 	for (int i = 0; i < lightNum; ++i)
// 	{
// 		auto &lightInfo = LightMgr::Instance().GetLightInfo(i);
// 		if (lightInfo.isEnabled) {
// 			QMatrix4x4 matModel;
// 			matModel.translate(lightInfo.pos);
// 			matModel.scale(0.1f);
// 			auto mod = m_assimpPtr->LoadModelWithModelMatrixAndShaderType("./models/LightBox.obj", matModel, ShaderHelper::Diffuse);
// 			lightInfo.SetModel(mod);
// 		}
// 	}

	// test load model
// 	m_assimpPtr->LoadModel("./models/WaterWave/water.obj");
// 	m_assimpPtr->LoadModel("./models/Box001.obj");
// 	m_assimpPtr->LoadModel("./models/plane2.obj");
// 	m_assimpPtr->LoadModel("./models/plane3.obj");
// 	m_assimpPtr->LoadModel("./models/teapot.obj");
// 	m_assimpPtr->LoadModel("./models/dva/001.obj");

	QMatrix4x4 matModel;
// 	matModel.translate(QVector3D(0, 0, 30));
	matModel.rotate(-90, QVector3D(1, 0, 0));
	matModel.scale(20);
	auto mod = m_assimpPtr->LoadModelWithModelMatrixAndShaderType("./models/test.fbx", matModel, ShaderHelper::Diffuse);

// 	m_assimpPtr->LoadModel("./models/skybox.obj");
// 	m_assimpPtr->LoadModel("./models/Box002.obj");

	Model *pMod = m_modelMgrPtr->FindModelByName("Plane001");
	if (Q_NULLPTR != pMod) {
// 		pMod->EnableProjTex();
		QMatrix4x4 mat;
// 		mat.translate(0, 30, 0);
// 		mat.rotate(90, QVector3D(0, 0, 1));
// 		mat.scale(40, 40, 40);
		pMod->SetWroldMat(mat);
	}
	else {
		pMod = m_modelMgrPtr->FindModelByName("water");
		if (Q_NULLPTR != pMod)
		{
			pMod->SetNormalMapTexture("./textures/waterNormal.jpg");
			QMatrix4x4 mat;
			mat.scale(40, 40, 40);
			pMod->SetWroldMat(mat);
		}
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
		mat.translate(60, 96, 0);
// 		mat.scale(2, 2, 2);
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
	}

	// toufa body shitimoface eye
	Model *pToufa = m_modelMgrPtr->FindModelByName("toufa");
	if (Q_NULLPTR != pToufa) {
		pToufa->SetNormalMapTexture("./models/dva/Map__11_Normal_Bump.png");
// 		pToufa->SetNormalDebugEnable(true);
	}

	Model *pBody = m_modelMgrPtr->FindModelByName("body");
	if (Q_NULLPTR != pBody) {
		pBody->SetNormalMapTexture("./models/dva/Map__15_Normal_Bump.png");
// 		pBody->SetNormalDebugEnable(true);
	}

	Model *pFace = m_modelMgrPtr->FindModelByName("shitimoface");
	if (Q_NULLPTR != pFace) {
		pFace->SetNormalMapTexture("./models/dva/Map__13_Normal_Bump.png");
// 		pFace->SetNormalDebugEnable(true);
	}

	Model *pEye = m_modelMgrPtr->FindModelByName("eye");
	if (Q_NULLPTR != pEye) {
		pEye->SetNormalMapTexture("./models/dva/Map__18_Normal_Bump.png");
// 		pEye->SetNormalDebugEnable(true);
	}
}

void OpenWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	m_cam->SetAspectRatio((float)w / (float)h);
}

void OpenWidget::ClearAndReset()
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

// 	static const GLfloat black[] = { 0.278f, 0.278f, 0.278f, 0.0f };
	static const GLfloat black[] = { 0, 0, 0, 0 };
	glClearBufferfv(GL_COLOR, 0, black);
	glClearBufferfv(GL_COLOR, 1, black);
	glClearBufferfv(GL_COLOR, 2, black);
	glClearBufferfv(GL_COLOR, 3, black);
	glClearBufferfv(GL_COLOR, 4, black);
	glClearBufferfv(GL_COLOR, 5, black);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	++fps;
	if (GetTickCount() - time >= 1000) {
		setWindowTitle(QString("fps:[%1]").arg(fps));
		time = GetTickCount();
		fps = 0;
	}
}

void OpenWidget::UpdateDynamicLightsInfo()
{
	QVector3D camPos = m_cam->GetCamPos().toVector3D();

	auto lightNum = m_lightMgrPtr->GetCurLightNum();
// 	float radius = 20.0f;
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
}

void OpenWidget::UpdateAllLightsInfo()
{
	auto lightNum = m_lightMgrPtr->GetCurLightNum();
	for (int i = 0; i < lightNum; ++i)
	{
		auto &lightInfo = m_lightMgrPtr->GetLightInfo(i);
		m_shaderHelperPtr->SetLightsInfo(lightInfo, i);
	}
}

void OpenWidget::paintGL()
{
	//----test geometry shader
// 	QMatrix4x4 matVP = m_cam->GetVPMatrix();
// 	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
// 	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
// 	QMatrix4x4 matView = m_cam->GetViewMatrix();
// 	QVector3D camPos = m_cam->GetCamPos().toVector3D();
// 	Model *pMode = m_modelMgrPtr->FindModelByName("body");
// 	if (nullptr != pMode) {
// 		ClearAndReset();
// 		pMode->SetShaderType(ShaderHelper::Default);
// 		pMode->Draw(matVP, pMode->GetWorldMat(), camPos, matProj, matView, matOrtho);
// 		pMode->SetShaderType(ShaderHelper::Geometry);
// 		pMode->Draw(matVP, pMode->GetWorldMat(), camPos, matProj, matView, matOrtho);
// 	}
// 	return;

	// test quad lines
// 	m_shaderHelperPtr->SetShaderType(ShaderHelper::FrameBuffer1);
// 	glBindVertexArray(vao_quad);
// 	static unsigned char idx[] = { 0,1,1,2,2,0, 3,4,4,5,5,3 };
// 	glDrawElements(GL_LINE_LOOP, 12, GL_UNSIGNED_BYTE, idx);
// 	return;

	auto modelNum = ModelMgr::Instance().GetModelNum();
	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
	QMatrix4x4 matView = m_cam->GetViewMatrix();
	QVector3D camPos = m_cam->GetCamPos().toVector3D();
	matVP = m_cam->GetOrthographicMatrix() * m_cam->GetLightViewMatrix();

	//-----Shadow render pass
	CreateShadowMapFrameBufferTexture();
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		qDebug() << "check frame buffer failed";
		return;
	}
	ClearAndReset();
	glViewport(0, 0, m_shadowTexWidth, m_shadowTexHeight);

	for (unsigned int i = 0; i < modelNum; ++i)	{
		Model *mod = ModelMgr::Instance().GetModel(i);
		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);// after handle one pass, you should restore the original pass, it's not necessary(mainly because of my function flow)

	//--------Deferred rendering g-buffer handle pass
// 	CreateGBufferFrameBufferTextures();
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		qDebug() << "check frame buffer failed";
		return;
	}
	ClearAndReset();
	glViewport(0, 0, size().width(), size().height());

	// update light info dynamicly
	UpdateDynamicLightsInfo();
	matVP = m_cam->GetVPMatrix();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);
// 		mod->SetDrawType(Mesh::Line);

// 		mod->SetShaderType(ShaderHelper::GBufferGeometry);
		if (mod->GetModelName().compare("water") == 0) {
			continue;
		}
		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}

	return;// test skeleton anim

	//----test add water wave during the deferred rendering g-buffer phase
	Model *pWater = ModelMgr::Instance().FindModelByName("water");
	if (Q_NULLPTR != pWater) {
		pWater->SetShaderType(ShaderHelper::Water);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);

		QMatrix4x4 matModel = pWater->GetWorldMat();
		pWater->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
		SwitchShader(ShaderHelper::GBufferGeometry);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down

	//--------SSAO buffer handle pass
	CreateSSAOFrameBufferTextures();
	SwitchShader(ShaderHelper::SSAO);
	ClearAndReset();

	glBindVertexArray(vao_quad);
	m_shaderHelperPtr->SetSSAOSamples(m_ssaoSampleVec);
	m_shaderHelperPtr->SetMVPMatrix(matVP, matVP, matView, matProj);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_gBufferPosTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_gBufferNormalTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, m_ssaoNoiseTex);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down

	//--------SSAO blur pass
	CreateSSAOBlurFrameBufferTextures();
	SwitchShader(ShaderHelper::SSAOBlur);
	ClearAndReset();
	glBindVertexArray(vao_quad);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, m_ssaoTex);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down

	//-------Draw normal debug after blur
	CreateGBufferFrameBufferTextures();
	Model *pMode = m_modelMgrPtr->FindModelByName("body");
	if (nullptr != pMode) {
		pMode->SetShaderType(ShaderHelper::Geometry);
		pMode->Draw(matVP, pMode->GetWorldMat(), camPos, matProj, matView, matOrtho);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down

	//-------Final draw pass
	glViewport(0, 0, size().width(), size().height());

// 	SwitchShader(ShaderHelper::FrameBuffer1);
// 	glBindVertexArray(vao_quad);
// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, m_ssaoBlurTex);
// 	glDrawArrays(GL_TRIANGLES, 0, 6);
// 	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);

// 	DrawOffScreenTexture();
// 	DrawWaterWaveWithOffScreenTexture();

// 	DrawShadowMapTexture_ForTest();
// 	DrawOriginalSceneWithShadow();

	//--------Deferred rendering(last pass)
	DrawDeferredShading();
}

float OpenWidget::lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void OpenWidget::GenerateHemisphereSamplers()
{
	m_ssaoSampleVec.clear();
	m_noiseVec.clear();
	QRandomGenerator rand;
	rand.bounded(1.0);

	for (unsigned int i = 0; i < ShaderHelper::ssaoSampleNum; ++i)
	{
		float x = float(rand.generateDouble() * 2.0 - 1.0);
		float y = float(rand.generateDouble() * 2.0 - 1.0);
		float z = float(rand.generateDouble());

		float scale = (qAbs(x) + qAbs(y)) / 2.0 / z;
// 		while (scale > 1)
// 		{
// 			x = float(rand.generateDouble() * 2.0 - 1.0);
// 			y = float(rand.generateDouble() * 2.0 - 1.0);
// 			z = float(rand.generateDouble());
// 
// 			scale = (qAbs(x) + qAbs(y)) / 2.0 / z;
// 		}

		QVector3D sample = {
			x,// [-1, 1]
			y,
			z,// [0, 1], only generate the normal oriented dir sample vec
		};

		scale = (float)i / ShaderHelper::ssaoSampleNum;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;// use the scale to focus more vecs near the original area
		m_ssaoSampleVec.append(sample);
	}

// 	int w = size().width(), h = size().height();
	static const int noiseNum = 4 * 4;
	for (unsigned int i = 0; i < noiseNum; ++i)
	{
		m_noiseVec.append(float(rand.generateDouble() * 2.0 - 1.0));
		m_noiseVec.append(float(rand.generateDouble() * 2.0 - 1.0));
		m_noiseVec.append(0);// rotate with z(normal)
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

void OpenWidget::CreateOffScreenFrameBufferTexture()
{
	if (0 == m_offScreenFbo)
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
		glTextureParameteri(m_offScreenTexId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_offScreenTexId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		glTextureParameteri(m_offScreenTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(m_offScreenTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
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
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_offScreenFbo);
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
	glBindVertexArray(vao_quad);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_offScreenTexId);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
// 	glEnable(GL_DEPTH_TEST);
}

void OpenWidget::DrawWaterWaveWithOffScreenTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);

	// check if we are good to go
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	// clear the framebuffer
	ClearAndReset();

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_offScreenTexId);

	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
	QMatrix4x4 matView = m_cam->GetViewMatrix();
	QVector3D camPos = m_cam->GetCamPos().toVector3D();
	auto modelNum = ModelMgr::Instance().GetModelNum();
	for (unsigned int i = 0; i < modelNum; ++i)
	{
		Model *mod = ModelMgr::Instance().GetModel(i);

		if (mod->GetModelName().compare("water") == 0) {
			mod->SetShaderType(ShaderHelper::Water);
		}
		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}

	glBindVertexArray(0);
}

void OpenWidget::CreateShadowMapFrameBufferTexture()
{
	if (0 == m_shadowMapFbo)
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		static float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMapTexId, 0);
	// 	glDrawBuffer(GL_NONE);
	// 	glReadBuffer(GL_NONE);
	
		glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
}

void OpenWidget::CreateGBufferFrameBufferTextures()
{
	if (0 == m_gBufferFbo) {
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_originalFbo);
		if (m_originalFbo != 1) {
			AddTipInfo(Q8("GBuffer帧缓存创建错误，应该在主窗口创建之后再创建"));
		}

		glCreateFramebuffers(1, &m_gBufferFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFbo);

		auto wndSize = size();
		glGenTextures(1, &m_gBufferPosTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferPosTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wndSize.width(), wndSize.height(), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// This ensures we don't accidentally oversample position/depth values in screen-space outside the texture's default coordinate region
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gBufferPosTex, 0);

		glGenTextures(1, &m_gBufferNormalTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferNormalTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wndSize.width(), wndSize.height(), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gBufferNormalTex, 0);

		//----Use GL_RGBA16F to enable the HDR effect with albedo and skybox
		glGenTextures(1, &m_gBufferAlbedoTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wndSize.width(), wndSize.height(), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gBufferAlbedoTex, 0);

		glGenTextures(1, &m_gBufferSkyboxTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferSkyboxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wndSize.width(), wndSize.height(), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_gBufferSkyboxTex, 0);

		//----render buffer for depth
// 		GLuint rb;
// 		glGenRenderbuffers(1, &rb);
// 		glBindRenderbuffer(GL_RENDERBUFFER, rb);
// 		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, wndSize.width(), wndSize.height());
// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
// 		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glGenTextures(1, &m_gBufferDepthTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferDepthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, wndSize.width(), wndSize.height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_gBufferDepthTex, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFbo);

	// tell opengl which color attachments we'll use(of the framebuffer) for rendering
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);
}

void OpenWidget::DrawDeferredShading()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);

	// check if we are good to go
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	// clear the framebuffer
	ClearAndReset();

	SwitchShader(ShaderHelper::DefferredRendering);
	// update light info dynamicly
	UpdateAllLightsInfo();
	UpdateDynamicLightsInfo();
	m_shaderHelperPtr->SetAmbientSpecularColor(QVector3D(1, 1, 1), QVector3D(1, 1, 1));
	auto camPos = m_cam->GetCamPos().toVector3D();
	m_shaderHelperPtr->SetCamWorldPos(camPos);
	QMatrix4x4 matLightVP = m_cam->GetOrthographicMatrix() * m_cam->GetLightViewMatrix();
	m_shaderHelperPtr->SetLightVPMat(matLightVP);
	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
	QMatrix4x4 matView = m_cam->GetViewMatrix();
	m_shaderHelperPtr->SetMVPMatrix(matLightVP, matLightVP, matView, matProj);
	m_shaderHelperPtr->SetAmbientSpecularColor(((MainWindow*)m_mainObj)->GetAmbientColor(), ((MainWindow*)m_mainObj)->GetSpecularColor());


	glBindVertexArray(vao_quad);
// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, m_shadowMapTexId);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_shadowMapTexId);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_gBufferPosTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_gBufferNormalTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, m_gBufferSkyboxTex);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, m_ssaoTex);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, m_ssaoBlurTex);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	//----Test draw water wave effect after deferred rendering
// 	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBufferFbo);
// 	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_originalFbo);
// 	auto s = size();
// 	int w = s.width(), h = s.height();
// 	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
// 	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
// 
// 	glActiveTexture(GL_TEXTURE3);
// 	glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
// 	QMatrix4x4 matVP = m_cam->GetVPMatrix();
// 	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
// 	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
// 	QMatrix4x4 matView = m_cam->GetViewMatrix();
// 	auto modelNum = ModelMgr::Instance().GetModelNum();
// 	for (unsigned int i = 0; i < modelNum; ++i)
// 	{
// 		Model *mod = ModelMgr::Instance().GetModel(i);
// 
// 		if (mod->GetModelName().compare("water") != 0) {
// 			continue;
// 		}
// 		mod->SetShaderType(ShaderHelper::Water);
// 		QMatrix4x4 matModel = mod->GetWorldMat();
// 		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
// 	}

	glBindVertexArray(0);
}

void OpenWidget::CreateSSAOFrameBufferTextures()
{
	if (0 == m_ssaoFbo) {
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_originalFbo);
		if (m_originalFbo != 1) {
			AddTipInfo(Q8("SSAO帧缓存创建错误，应该在主窗口创建之后再创建"));
		}

		auto s = size();
		GLuint w = s.width(), h = s.height();
		glCreateFramebuffers(1, &m_ssaoFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFbo);

		glGenTextures(1, &m_ssaoTex);
		glBindTexture(GL_TEXTURE_2D, m_ssaoTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RED, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoTex, 0);

		// create the noise texture for ssao frame buffer
		GenerateHemisphereSamplers();

		glGenTextures(1, &m_ssaoNoiseTex);
		glBindTexture(GL_TEXTURE_2D, m_ssaoNoiseTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, m_noiseVec.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFbo);
}

void OpenWidget::CreateSSAOBlurFrameBufferTextures()
{
	if (0 == m_ssaoBlurFbo) {
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_originalFbo);
		if (m_originalFbo != 1) {
			AddTipInfo(Q8("SSAO帧缓存创建错误，应该在主窗口创建之后再创建"));
		}

		auto s = size();
		GLuint w = s.width(), h = s.height();
		glCreateFramebuffers(1, &m_ssaoBlurFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFbo);

		glGenTextures(1, &m_ssaoBlurTex);
		glBindTexture(GL_TEXTURE_2D, m_ssaoBlurTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoBlurTex, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFbo);
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
	glBindVertexArray(vao_quad);
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

	ClearAndReset();
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
