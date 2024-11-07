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
#include "Cube.h"


#include <windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <string>
#include <sstream>
#pragma comment(lib, "Dbghelp.lib")

std::string TraceStack()
{
    static const int MAX_STACK_FRAMES = 10;

    void *pStack[MAX_STACK_FRAMES];

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    WORD frames = CaptureStackBackTrace(0, MAX_STACK_FRAMES, pStack, NULL);

    std::ostringstream oss;
    oss << "stack traceback: " << std::endl;
    for (WORD i = 0; i < frames; ++i) {
        DWORD64 address = (DWORD64)(pStack[i]);

        DWORD64 displacementSym = 0;
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        DWORD displacementLine = 0;
        IMAGEHLP_LINE64 line;
        //SymSetOptions(SYMOPT_LOAD_LINES);
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        if (SymFromAddr(process, address, &displacementSym, pSymbol)
            && SymGetLineFromAddr64(process, address, &displacementLine, &line)) {
            oss << "\t" << pSymbol->Name << " at " << line.FileName << ":" << line.LineNumber << "(0x" << std::hex << pSymbol->Address << std::dec << ")" << std::endl;
        }
        else {
            oss << "\terror: " << GetLastError() << std::endl;
        }
    }
    return oss.str();
}

OpenWidget::OpenWidget()
	:m_mainObj(Q_NULLPTR), m_cam(Q_NULLPTR)
	, m_matWorldLoc(0), m_worldCamPosLoc(0), m_query(0), m_sampleNum(0)
	, m_offScreenFbo(0), m_shadowMapFbo(0), m_shadowTexWidth(4096), m_shadowTexHeight(4096)
	, m_shaderHelperPtr(nullptr)
	, m_gBufferFbo(0), m_gBufferPosTex(0), m_gBufferNormalTex(0), m_gBufferAlbedoTex(0), m_gBufferSkyboxTex(0), m_gBufferAlbedo2Tex(0)
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


void GLErrorMessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    //fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
    //    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
    //    type, severity, message);

    AddTipInfo(QString("GL CALLBACK: %1 type = %2, severity = %3, message = %4, source = %5, id = %6").arg((type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "Warning")).arg(type).arg(severity).arg(message).arg(source).arg(id));
}

GLuint vao_quad = 0;
GLuint vbo_quad = 0;
GLuint texId = 0;
void OpenWidget::initializeGL()
{
	initializeOpenGLFunctions();

    // return here to test gles1.1 apis with gles1 function
    //return;

    double i = 0;

    double result = 0 / i;
    auto r = isnan(result);

    double i2 = 0;

    double result2 = 1 / i2;
    auto r2 = isnan(result2);

	m_shaderHelperPtr = &ShaderHelper::Instance();
	ChkGLErr;

	auto exts = glGetString(GL_EXTENSIONS);

// 	TestGeometryPoints();
	// test for a quad
	{
        // 这里使用的quad就是延迟渲染使用的两个三角面片，第一个中间有条裂纹，第二个是完全拼接的
		const GLfloat g_vertices[6][2] = {
			{-0.95f, -0.95f}, {0.92f, -0.95f}, {-0.95f, 0.92f}, // first triangle
			{0.95f, -0.92f}, {0.95f, 0.95f}, {-0.92f, 0.95f}, // second triangle
		};
 		//const GLfloat g_vertices[6][2] = {
 		//	{-1, -1}, {1, -1}, {-1, 1}, // first triangle
 		//	{1, -1}, {1, 1}, {-1, 1}, // second triangle
 		//};

		const GLfloat g_uvs[6][2] = {
			{0, 1}, {1, 1}, {0, 0}, //
			{1, 1}, {1, 0}, {0, 0}
		};

		glGenVertexArrays(1, &vao_quad);
		glBindVertexArray(vao_quad);
		ChkGLErr;

		texId = TextureMgr::Instance().LoadTexture("./textures/container.jpg");
		SwitchShader(ShaderHelper::FrameBuffer1);
		auto loc = ShaderHelper::Instance().GetUniformLocation("tex");
		glUniform1i(loc, 0);
		ChkGLErr;

		glGenBuffers(1, &vbo_quad);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, nullptr, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW GL_DYNAMIC_DRAW，GL_STREAM_DRAW，这三个没太搞懂，好像设置哪个都不影响后面动态修改。。。。后面懂了再来加注释
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 12, g_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, sizeof(GLfloat) * 12, g_uvs);
		ChkGLErr;

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		ChkGLErr;
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 12));
		ChkGLErr;
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// a test for 2d array textures
	QVector<QString> pathVec = {
		"",
		""
	};
	m_texArr01Id = TextureMgr::Instance().Load2DArrTextures(pathVec);
	//m_tex3d01Id = TextureMgr::Instance().Load3DTexture(pathVec);

// 	glEnable(GL_CULL_FACE);
// 	glCullFace(GL_BACK);
// 	glFrontFace(GL_CW);

// During init, enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLErrorMessageCallback, 0);


	glEnable(GL_DEPTH_TEST);
	ChkGLErr;
	glEnable(GL_STENCIL_TEST);
	ChkGLErr;
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	ChkGLErr;
	//glEnable(GL_TEXTURE_3D);
	//ChkGLErr;

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	ChkGLErr;
// 	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	TextureMgr::Instance();
	ChkGLErr;

	// load the light box
    //auto lightNum = LightMgr::Instance().GetCurLightNum();
    //for (int i = 0; i < lightNum; ++i)
    //{
    //    auto &lightInfo = LightMgr::Instance().GetLightInfo(i);
    //    if (lightInfo.isEnabled) {
    //        QMatrix4x4 matModel;
    //        matModel.translate(lightInfo.pos);
    //        matModel.scale(0.1f);
    //        auto mod = m_assimpPtr->LoadModelWithModelMatrixAndShaderType("./models/LightBox.obj", matModel, ShaderHelper::PureColor);
    //        lightInfo.SetModel(mod);
    //    }
    //}

	// test load model
    //m_assimpPtr->LoadModel("./models/WaterWave/water.obj");
    m_assimpPtr->LoadModel("./models/plane.obj");
// 
// 	m_assimpPtr->LoadModel("./models/largesphere.obj");
    //m_assimpPtr->LoadModel("./models/Box001.obj");
    //m_assimpPtr->LoadModel("./models/plane2.obj");
    //m_assimpPtr->LoadModel("./models/plane3.obj");
    //m_assimpPtr->LoadModel("./models/teapot.obj");
    //m_assimpPtr->LoadModel("./models/dva/001.obj");
    //m_assimpPtr->LoadModel("./models/Box002.obj");
    //m_assimpPtr->LoadModel("./models/Sponza/Sponza.obj");

	// create the sphere walls, left and right side----
	// ----PBR relevant
	{
// 		int leftOffset = -30, rightOffset = 30;
// 		int interval = 20;
// 		int rowNum = 3, colNum = 3;
// 		int zStartPos = interval * colNum / 2 * -1;
// 		int yStartPos = 0;
// 		QMatrix4x4 mat;
// 		// left side
// 		for (int row = 0; row < rowNum; ++row) {
// 			for (int col = 0; col < colNum; ++col) {
// 				mat.setToIdentity();
// 				mat.translate(QVector3D(leftOffset, yStartPos + interval * (row % rowNum), zStartPos + interval * (col % colNum)));
// 				m_assimpPtr->LoadModelWithModelMatrixAndShaderType("./models/largesphere.obj", mat, ShaderHelper::Default);
// 			}
// 		}
// 		// right side
// 		for (int row = 0; row < rowNum; ++row) {
// 			for (int col = 0; col < colNum; ++col) {
// 				mat.setToIdentity();
// 				mat.translate(QVector3D(rightOffset, yStartPos + interval * (row % rowNum), zStartPos + interval * (col % colNum)));
// 				m_assimpPtr->LoadModelWithModelMatrixAndShaderType("./models/largesphere.obj", mat, ShaderHelper::Default);
// 			}
// 		}
	}

	QMatrix4x4 matModel;
// 	matModel.translate(QVector3D(0, 0, 30));
	matModel.rotate(-90, QVector3D(1, 0, 0));
	matModel.scale(20);
// 	auto mod = m_assimpPtr->LoadModelWithModelMatrixAndShaderType("./models/piety.fbx", matModel, ShaderHelper::Diffuse);

	m_assimpPtr->LoadModel("./models/skybox.obj");

	Model *pMod = m_modelMgrPtr->FindModelByName("Plane001");
	if (Q_NULLPTR != pMod) {
// 		pMod->EnableProjTex();
		QMatrix4x4 mat;
// 		mat.translate(0, 30, 0);
        mat.rotate(180, QVector3D(0, 1, 0));
        //mat.rotate(180, QVector3D(1, 0, 0));
		mat.scale(40, 40, 40);
		pMod->SetWroldMat(mat);
		pMod->SetNormalMapTextureByMeshName("./models/brickwall_normal.jpg", "Plane001");
	}

    // use this model to test volumetric light effect
    pMod = m_modelMgrPtr->FindModelByName("sponza_00");
    if (Q_NULLPTR != pMod) {
        QMatrix4x4 mat;
        mat.scale(0.1f, 0.1f, 0.1f);
        pMod->SetWroldMat(mat);
    }

	pMod = m_modelMgrPtr->FindModelByName("water");
	if (Q_NULLPTR != pMod) {
		pMod->SetNormalMapTextureByMeshName("./textures/waterNormal.jpg", "water");
		QMatrix4x4 mat;
		mat.scale(40, 40, 40);
		pMod->SetWroldMat(mat);
	}

	Model *pMod2 = m_modelMgrPtr->FindModelByName("Plane002");
	if (Q_NULLPTR != pMod2) {
		QMatrix4x4 mat;
		mat.translate(-90, 0, 0);
		mat.rotate(-90, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod2->SetWroldMat(mat);
		pMod2->SetNormalMapTextureByMeshName("./models/brickwall_normal.jpg", "Plane002");
	}
	Model *pMod3 = m_modelMgrPtr->FindModelByName("Plane003");
	if (Q_NULLPTR != pMod3) {
		QMatrix4x4 mat;
		mat.translate(90, 0, 0);
		mat.rotate(270, QVector3D(0, 0, 1));
		mat.scale(10, 10, 10);
		pMod3->SetWroldMat(mat);
		pMod3->SetNormalMapTextureByMeshName("./models/brickwall_normal.jpg", "Plane003");
	}

//	Model *pSphere001 = m_modelMgrPtr->FindModelByName("Sphere001");
//	if (Q_NULLPTR != pSphere001) {
//		QMatrix4x4 mat;
//		mat.translate(0, 20, 0);
//// 		mat.scale(2, 2, 2);
//		pSphere001->SetWroldMat(mat);
//		pSphere001->SetAllMeshesNormalMapTexture("./models/brickwall_normal.jpg");
//		pSphere001->SetAllMeshesDiffuseTexture("./models/brickwall.jpg");
//	}

	Model *pBox001 = m_modelMgrPtr->FindModelByName("Box001");
	if (Q_NULLPTR != pBox001) {
// 		pBox001->EnableProjTex();
// 		pBox001->SetDrawType(Mesh::Point);
		QMatrix4x4 mat;
		mat.translate(0, 40, 0);
// 		mat.translate(0, 0, 0);
// 		mat.scale(2, 2, 2);
		pBox001->SetWroldMat(mat);
		pBox001->SetAllMeshesNormalMapTexture("./models/brickwall_normal.jpg");
	}
	Model *pBox002 = m_modelMgrPtr->FindModelByName("Box002");
	if (Q_NULLPTR != pBox002) {
		QMatrix4x4 mat;
		mat.translate(0, -37, 0);
		pBox002->SetWroldMat(mat);
		pBox002->SetAllMeshesNormalMapTexture("./models/brickwall_normal.jpg");
	}

	Model *pTeapot = m_modelMgrPtr->FindModelByName("defaultobject");
	if (Q_NULLPTR != pTeapot) {
		QMatrix4x4 mat;
		mat.translate(30, 0, 0);
		mat.scale(5);
		pTeapot->SetWroldMat(mat);
		// must generate aabb mesh manual
		pTeapot->GenerateAABBMesh();
	}

	Model *pSkybox = m_modelMgrPtr->FindModelByName("skybox");
	if (Q_NULLPTR != pSkybox) {
		pSkybox->EnableSkybox();
	}

	// toufa body shitimoface eye
	pMod = m_modelMgrPtr->FindModelByName("toufa");
	if (Q_NULLPTR != pMod) {
// 		pMod->SetNormalMapTextureByMeshName("./models/dva/Map__11_Normal_Bump.png", "toufa");
// 		pMod->SetNormalMapTextureByMeshName("./models/dva/Map__15_Normal_Bump.png", "body");
// 		pMod->SetNormalMapTextureByMeshName("./models/dva/Map__13_Normal_Bump.png", "shitimoface");
// 		pMod->SetNormalMapTextureByMeshName("./models/dva/Map__18_Normal_Bump.png", "eye");
		pMod->GenerateAABBMesh();
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

    // test gl scissor
    {
        //glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, 300, 300);
    }

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
    glDisable(GL_SCISSOR_TEST);

    //glOrtho(100, 100, 200, 200, -1, 1000);

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

void OpenWidget::gles1()
{
    // gles1中的最大区别就在于，是固定管线，不用shader，像mat的设置都是调用api完成的

    const GLfloat g_vertices[12] = {
        -0.95f, -0.95f, 0.92f, -0.95f, -0.95f, 0.92f, // first triangle
        0.95f, -0.92f, 0.95f, 0.95f, -0.92f, 0.95f, // second triangle
    };

    const GLfloat g_uvs[12] = {
        0, 1, 1, 1, 0, 0, //
        1, 1, 1, 0, 0, 0
    };

    float triangleCoords[] = {
        // X, Y, Z
        -0.4f, -0.25f, 0,
         0.5f, -0.25f, 0,
         0.0f,  0.559016994f, 0
    };


    float ratio;
    float zNear = -1.0f;
    float zFar = 1000.0f;
    float fieldOfView = 3.1415926f * (30.0f / 180.0f);
    float size;

    glEnable(GL_NORMALIZE);
    ratio = (float)80 / (float)60;
    glMatrixMode(GL_PROJECTION);
    size = zNear * (float)(tan((double)(fieldOfView / 2.0f)));
    //glFrustumf(-size, size, -size / ratio, size / ratio, zNear, zFar);
    glFrustum(-size, size, -size / ratio, size / ratio, zNear, zFar);
    // 设置某个type的矩阵之前，一般要loadIdentity，否则矩阵的数据会一直叠加，是不正确的
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, 0);

    static const GLfloat black[] = { 1, 0, 0, 0.5 };
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_COLOR, 1, black);
    glClearBufferfv(GL_COLOR, 2, black);
    glClearBufferfv(GL_COLOR, 3, black);
    glClearBufferfv(GL_COLOR, 4, black);
    glClearBufferfv(GL_COLOR, 5, black);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);// 开灯会导致设置的颜色无效，绘制出的物体是全黑的，灯光应该需要相应的设置
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);

    //glUseProgram(0);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

    //glTranslatef(start_x - width / 2, start_y + height / 2, 0.0);
    //glTranslatef(width / 2, -height / 2, 0.0);
    //glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
    //glTranslatef(-width / 2, height / 2, 0.0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, g_vertices);
    glColor4f(1, 1, 0, 1);
    GLubyte idx[6] = { 0, 1, 2, 3, 4, 5 };
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, idx);
    //glDrawArrays(GL_TRIANGLES, 0, 3);

    //glDisableClientState(GL_VERTEX_ARRAY);
    //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void OpenWidget::paintGL()
{
    //gles1();
    //return; // return for gles1 test

    // 测试map和unmap buffer的用法
    if (GetAsyncKeyState(VK_RMENU))
    {
        // 注意这里是没有绑定buffer的，因为是接着外面绑定好的buffer，直接就进行修改
        // 我会在其他位置（buffer已经变化后，再尝试修改内存值试试看什么情况）
        // 这里就是其他的地方了，我会在绘制途中，按键，进行修改
        // 经过我的测试，目前比较有意思的结论是，target是否是GL_ARRAY_BUFFER并不影响，也可以用GL_COPY_WRITE_BUFFER
        // 而且，绑定的是vbo还是vao（当然是基于同一个vao下面的vbo），都可以拿回数据修改
        glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);

        // 基础的flag就是GL_MAP_READ_BIT和GL_MAP_WRITE_BIT，其中write的话，也是可以看到值的（不过为了防止误修改，一般read的时候还是只设置read）
        // 关于GL_MAP_FLUSH_EXPLICIT_BIT这个flag，比较有意思，官方说的是如果指明了这个bit，就必须要明确的调用glFlushMappedBufferRange，但是我这里试了下，每调用，还是可以成功修改，range外的值，以后有发现再补充吧
        void *dataPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, 48, GL_MAP_WRITE_BIT);
        ChkGLErr;

		float fArr[6][2] = { 0 };
		memcpy_s(fArr, sizeof(GLfloat) * 12, dataPtr, sizeof(GLfloat) * 12);

        float f1 = *(float*)dataPtr;
        float f2 = *((float*)dataPtr + 1);
        // 尝试不使用write bit来修改数据，测试结果是无效的
        // 只有使用了GL_MAP_WRITE_BIT，才能让修改生效
        //*((float*)dataPtr + 1) = -1;

        float f3 = *((float*)dataPtr + 2);
		// 尝试读取范围以外的数据，可以读取，但是可能有未知风险
        float f13 = *((float*)dataPtr + 12);
        float f14 = *((float*)dataPtr + 13);
		// 尝试修改范围外的数据，可以修改，但是可能有未定义错误
        //*((float*)dataPtr + 13) = 0;

		// 没有指定GL_MAP_FLUSH_EXPLICIT_BIT的话，只不需要调用flush的
        //glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, 48);

        // 调用unmap的时候会自动flush整个mapbuffer
        glUnmapBuffer(GL_ARRAY_BUFFER);
        ChkGLErr;

		// --------------------------------测试sub data替代mapBufferRange的用法
		
		// 尝试用glBufferSubData替换unmap
		// 这里并不需要原始数据，我们应该填入的数据，应该是之前map过来的指针
        const GLfloat g_vertices[6][2] = {
			{-0.95f, -0.55f}, {0.92f, -0.95f}, {-0.95f, 0.92f}, // first triangle
			{0.95f, -0.92f}, {0.95f, 0.95f}, {-0.92f, 0.95f}, // second triangle
        };
		// 使用sub data的时候，并不能和mapbuffer混用，我记错了bst那边的代码，那边在mapBufferRange的时候就没有调用mapBufferRange，而是直接返回的内存段指针
		// 所以这里要使用sub data的时候，就直接使用subdata的函数进行修改既可，mapBufferRange以后，反而不能使用subdata修改，因为正在map中
		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 12, g_vertices);

		// 即便要先取得之前的数据，我们也应该再unmap之后再使用
		fArr[0][1] = -0.5;
		((float*)dataPtr)[1] = -0.5;// 使用map出来的指针也可以，但是必须再Unmap之后！！！
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 12, dataPtr);

        //glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    //TraceStack();

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
    // shadow should use light vp mat(restore to normal vp, after shadow pass)
    // 这里之前出了一个非常严重的问题，应该是我随手改的，那就是对于平行光来说，我们的lightVP里的投影，应该使用正交投影，除非是其他光源
    // 而我这里用成了projection的矩阵，没有和最后pass中设置的orthoganal对应起来
    matVP = matOrtho * m_cam->GetLightViewMatrix();
    // 使用透视的细节我有点忘了，使用正交的话，阴影表现很正常，但是光源的位置改变就不能起效了，这里应该还有点问题
    //matVP = matProj * m_cam->GetLightViewMatrix();

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
	// 关闭deferred绘制后，gbuffer也应该关闭，否则绘制不正常
 	CreateGBufferFrameBufferTextures();
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
		// enable shadow tex and bind it(not with deferred render mode)
// 		glActiveTexture(GL_TEXTURE2);
// 		glBindTexture(GL_TEXTURE_2D, m_shadowMapTexId);
// 		QMatrix4x4 matLightVP = matProj * m_cam->GetLightViewMatrix();
// 		m_shaderHelperPtr->SetLightVPMat(matLightVP);

		Model *mod = ModelMgr::Instance().GetModel(i);
		// 开启线框模式
// 		mod->SetDrawType(Mesh::Line);

		if (mod->GetModelName().compare("water") == 0) {
			continue;
		}
 		mod->SetShaderType(ShaderHelper::GBufferGeometry);
		QMatrix4x4 matModel = mod->GetWorldMat();
		mod->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
    }

// 	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
// 	SwitchShader(ShaderHelper::FrameBuffer1);
// 	glBindVertexArray(vao_quad);
// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
// 	glDrawArrays(GL_TRIANGLES, 0, 6);

// 	return;// test skeleton anim

	//----test add water wave during the deferred rendering g-buffer phase
	Model *pWater = ModelMgr::Instance().FindModelByName("water");
	if (Q_NULLPTR != pWater) {
		pWater->SetShaderType(ShaderHelper::Water);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
		glGenerateMipmap(GL_TEXTURE_2D);
		ChkGLErr;

		QMatrix4x4 matModel = pWater->GetWorldMat();
		pWater->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down
	ChkGLErr;

	//--------SSAO buffer handle pass
// 	CreateSSAOFrameBufferTextures();
// 	SwitchShader(ShaderHelper::SSAO);
// 	ClearAndReset();
// 
// 	glBindVertexArray(vao_quad);
// 	m_shaderHelperPtr->SetSSAOSamples(m_ssaoSampleVec);
// 	m_shaderHelperPtr->SetMVPMatrix(matVP, matVP, matView, matProj);
// 
// 	glActiveTexture(GL_TEXTURE4);
// 	glBindTexture(GL_TEXTURE_2D, m_gBufferPosTex);
// 	glActiveTexture(GL_TEXTURE5);
// 	glBindTexture(GL_TEXTURE_2D, m_gBufferNormalTex);
// 	glActiveTexture(GL_TEXTURE6);
// 	glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
// 	glActiveTexture(GL_TEXTURE8);
// 	glBindTexture(GL_TEXTURE_2D, m_ssaoNoiseTex);
// 
// 	glDrawArrays(GL_TRIANGLES, 0, 6);
// 	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down
// 
// 	//--------SSAO blur pass
// 	CreateSSAOBlurFrameBufferTextures();
// 	SwitchShader(ShaderHelper::SSAOBlur);
// 	ClearAndReset();
// 	glBindVertexArray(vao_quad);
// 
// 	glActiveTexture(GL_TEXTURE9);
// 	glBindTexture(GL_TEXTURE_2D, m_ssaoTex);
// 
// 	glDrawArrays(GL_TRIANGLES, 0, 6);
// 	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down
// 
// 	//-------Draw normal debug after blur
// 	CreateGBufferFrameBufferTextures();
// 	Model *pMode = m_modelMgrPtr->FindModelByName("body");
// 	if (nullptr != pMode) {
// 		pMode->SetShaderType(ShaderHelper::Geometry);
// 		pMode->Draw(matVP, pMode->GetWorldMat(), camPos, matProj, matView, matOrtho);
// 	}
// 	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo); // must restore after pass down
// 
// 	//-------Final draw pass
// 	glViewport(0, 0, size().width(), size().height());

// 	SwitchShader(ShaderHelper::FrameBuffer1);
// 	glBindVertexArray(vao_quad);
// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
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
		ChkGLErr;
	
		auto wndSize = size();
		glGenTextures(1, &m_shadowMapTexId);
		glBindTexture(GL_TEXTURE_2D, m_shadowMapTexId);
		ChkGLErr;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowTexWidth, m_shadowTexHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		ChkGLErr;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		static float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
		ChkGLErr;
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMapTexId, 0);
	// 	glDrawBuffer(GL_NONE);
	// 	glReadBuffer(GL_NONE);
	
		glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
		ChkGLErr;
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
		// Position
		glGenTextures(1, &m_gBufferPosTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferPosTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, wndSize.width(), wndSize.height(), 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// This ensures we don't accidentally oversample position/depth values in screen-space outside the texture's default coordinate region
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gBufferPosTex, 0);

		// Normal
		glGenTextures(1, &m_gBufferNormalTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferNormalTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, wndSize.width(), wndSize.height(), 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gBufferNormalTex, 0);

		// Albedo
		//----Use GL_RGBA16F to enable the HDR effect with albedo and skybox
		glGenTextures(1, &m_gBufferAlbedoTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedoTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, wndSize.width(), wndSize.height(), 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// This ensures we don't accidentally oversample position/depth values in screen-space outside the texture's default coordinate region
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// 这是一个可能会造成tex取值闪烁的重要函数，之前水面下的闪烁就和这个有关
		// 但是，它的调用地点应该在设置完有效内容后再进行生成，这里生成的话就是一个空的（无效的）Mipmap
// 		glGenerateMipmap(GL_TEXTURE_2D);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gBufferAlbedoTex, 0);

		// Albedo2
//----Use GL_RGBA16F to enable the HDR effect with albedo and skybox
		glGenTextures(1, &m_gBufferAlbedo2Tex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferAlbedo2Tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, wndSize.width(), wndSize.height(), 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// This ensures we don't accidentally oversample position/depth values in screen-space outside the texture's default coordinate region
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_gBufferAlbedo2Tex, 0);

		// Skybox
		glGenTextures(1, &m_gBufferSkyboxTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferSkyboxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wndSize.width(), wndSize.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_gBufferSkyboxTex, 0);

		// tell opengl which color attachments we'll use(of the framebuffer) for rendering
		// you only need to set this once, because I can verify it from the Nsight tool
		static unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(5, attachments);

		//----render buffer for depth
// 		GLuint rb;
// 		glGenRenderbuffers(1, &rb);
// 		glBindRenderbuffer(GL_RENDERBUFFER, rb);
// 		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, wndSize.width(), wndSize.height());
// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
// 		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// you must create depth buffer for your fb, even you don't use it out here, otherwise your gbuffer won't show correctly
		glGenTextures(1, &m_gBufferDepthTex);
		glBindTexture(GL_TEXTURE_2D, m_gBufferDepthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, wndSize.width(), wndSize.height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// This ensures we don't accidentally oversample position/depth values in screen-space outside the texture's default coordinate region
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_gBufferDepthTex, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);

		// create and attach depth buffer (renderbuffer), method 2 to create the depth buffer
// 		unsigned int rboDepth;
// 		glGenRenderbuffers(1, &rboDepth);
// 		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
// 		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, wndSize.width(), wndSize.height());
// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
// 		// finally check if framebuffer is complete
// 		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
// 			AddTipInfo("Frame buffer not complete");
// 		glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFbo);
}

void OpenWidget::DrawDeferredShading()
{
    // 	glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);

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
	auto camPos = m_cam->GetCamPos().toVector3D();
	m_shaderHelperPtr->SetCamWorldPos(camPos);
    // 由于是平行光，所以用的正交投影吗？？（经过验证，使用正交的阴影不正确????，模拟摄像机的话，应该还是要用透视投影???）
    QMatrix4x4 matLightVP = m_cam->GetOrthographicMatrix() * m_cam->GetLightViewMatrix();
    //matLightVP = m_cam->GetProjectionMatrix() * m_cam->GetLightViewMatrix();
	m_shaderHelperPtr->SetLightVPMat(matLightVP);
	QMatrix4x4 matVP = m_cam->GetVPMatrix();
	QMatrix4x4 matProj = m_cam->GetProjectionMatrix();
	QMatrix4x4 matOrtho = m_cam->GetOrthographicMatrix();
	QMatrix4x4 matView = m_cam->GetViewMatrix();
    QMatrix4x4 modelMat = QMatrix4x4();
	m_shaderHelperPtr->SetMVPMatrix(matVP, modelMat, matView, matProj);
	m_shaderHelperPtr->SetAmbientSpecularColor(((MainWindow*)m_mainObj)->GetAmbientColor(), ((MainWindow*)m_mainObj)->GetSpecularColor());

	// ----Adjust PBR dynamically
	static int iCount = 0;
	static float matallic = 0.01f;
	if (iCount++ % 100 == 0) {
		matallic += 0.01f;
		if (matallic > 1.0f) {
			iCount = 0;
			matallic = 0.0f;
		}
	}
	m_shaderHelperPtr->SetPBR(matallic, matallic, 0);


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

	// test 2d array textures(use the last texture id)
	glActiveTexture(GL_TEXTURE31);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_texArr01Id);
	glActiveTexture(GL_TEXTURE30);
	glBindTexture(GL_TEXTURE_3D, m_tex3d01Id);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // ----测试read和draw的fbo的分离使用细节
    // 之前没有这么用过，因为bs5模拟器所以详细测试了下，这里简单记录下要点：
    // 1：如果设置的是GL_FRAMEBUFFER，那就相当于同时把read和draw都设置到了一个fbo上（read和draw分别同时只能存在一个）
    // 2：开始因为没用过这种分离式的fb，所以有点懵，理解以后其实很简单，就是设置一个read（src）和一个draw（dest）后，我们就可以使用glBlitFramebuffer快速的把内容拷贝到另外一个fbo中了
    // 3：所以说CreateReadAndDrawBufferTextures中是不需要创建一个新的read fbo的，因为没理解我才创建了一个
    // 4（补充）：第3点，本来说是没有问题的，确实是从read拷贝到draw，我们可以选择default fbo作为src来read，但是我当时是想了解新建的read如何运作，其最经典的处理就在于，绑定fbo的texture时，和其他fbo的使用的texture绑定在一起
    // 这样的话，只要任何一方fbo进行了绘制，两个fbo就都有相同的像素了，这是我以前没有接触过的一种方式
    if (0) {
        CreateReadAndDrawBufferTextures();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_readFbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_drawFbo);
        GLenum arr[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, arr);
	
	    int w = size().width(), h = size().height();
        GLint err = 0;
        GLuint newTexId = 0;
        glGenTextures(1, &newTexId);
        glBindTexture(GL_TEXTURE_2D, newTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // 都不需要绑定fb的tex2d居然也可以成功CopyTexSubImage2D.....
        //glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, newTexId, 0);
        err = glGetError();

        err = glGetError();
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 800, 600);
        err = glGetError();
        // test get tex data out
        GLint texWidth = 0, texHeight = 0, texDepth = 0, texInternalFmt = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
            GL_TEXTURE_WIDTH,
            &texWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
            GL_TEXTURE_HEIGHT,
            &texHeight);
        glGetTexLevelParameteriv(
            GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &texInternalFmt);
        static int iCount = 0;

#define GL_BGRA 0x80E1
        if (0 != texWidth && 0 != texHeight) {
            GLint picSize = texWidth * texHeight * 4;
            unsigned char* tmpBuf = new unsigned char[picSize];
            glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                tmpBuf);

            bool allBlackFlag = true;
            for (int i = 0; i < picSize; i += 4) {
                if (tmpBuf[i] != 0 || tmpBuf[i + 1] != 0 || tmpBuf[i + 2] != 0) {
                    allBlackFlag = false;
                    break;
                }
            }

            if (!allBlackFlag) {
                GLint param = 0;
                
                std::string strTmp = "c:/users/administrator/desktop/tmpPic/qt_out_";
                strTmp += std::to_string(param);
                strTmp += "_";
                strTmp += std::to_string(iCount);
                strTmp += ".bmp";

                generateBmp(tmpBuf, 4, texWidth, texHeight, strTmp.c_str());
                ++iCount;
            }

            delete[] tmpBuf;
        }




        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        if (0)
        GetFboPixelSaveToBmp();
	
        glBindFramebuffer(GL_FRAMEBUFFER, m_originalFbo);
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 1);
    }

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

void OpenWidget::CreateReadAndDrawBufferTextures()
{
    if (0 == m_readFbo) {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_originalFbo);
        if (m_originalFbo != 1) {
            AddTipInfo(Q8("Read帧缓存创建错误，应该在主窗口创建之后再创建"));
        }

        auto s = size();
        GLuint w = s.width(), h = s.height();
        glCreateFramebuffers(1, &m_readFbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_readFbo);

        glGenTextures(1, &m_readTexId);
        glBindTexture(GL_TEXTURE_2D, m_readTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gBufferAlbedoTex, 0);

        if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            AddTipInfo(Q8("Read帧缓存创建错误，应该在主窗口创建之后再创建"));
            return;
        }
    }

    if (0 == m_drawFbo) {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_originalFbo);
        if (m_originalFbo != 1) {
            AddTipInfo(Q8("Draw帧缓存创建错误，应该在主窗口创建之后再创建"));
        }

        auto s = size();
        GLuint w = s.width(), h = s.height();
        glCreateFramebuffers(1, &m_drawFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_drawFbo);

        glGenTextures(1, &m_drawTexId);
        glBindTexture(GL_TEXTURE_2D, m_drawTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_drawTexId, 0);

        auto err = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if (err != GL_FRAMEBUFFER_COMPLETE) {
            AddTipInfo(Q8("Draw帧缓存创建错误，应该在主窗口创建之后再创建"));
            return;
        }
    }
}

void OpenWidget::GetFboPixelSaveToBmp()
{
    unsigned int w = size().width(), h = size().height();
    GLint picSize = w * h * 4;
    unsigned char* tmpBuf = new unsigned char[picSize];
    // test get tex data out
    //#define GL_BGRA 0x80E1
    glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, tmpBuf);


    if (0 != w && 0 != h) {

        bool allBlackFlag = true;
        for (int i = 0; i < picSize; i += 4) {
            if (tmpBuf[i] != 0 || tmpBuf[i + 1] != 0 || tmpBuf[i + 2] != 0) {
                allBlackFlag = false;
                break;
            }
        }

        if (!allBlackFlag) {
            GLint param = 0;
            static int iCount = 0;
            //ctx->dispatcher().glGetIntegerv(GL_TEXTURE_BINDING_2D, &param);
            std::string strTmp = "c:/users/administrator/desktop/tmpPic/qt_out_";
            strTmp += std::to_string(param);
            strTmp += "_";
            strTmp += std::to_string(iCount);
            strTmp += ".bmp";

            generateBmp(tmpBuf, 4, w, h, strTmp.c_str());
            ++iCount;
        }

        delete[] tmpBuf;
    }
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
