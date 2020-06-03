#include "ShaderHelper.h"
#include "QFile"
#include "PreDef.h"
#include "QMatrix4x4"

ShaderHelper::ShaderHelper()
{
	m_lightMgrPtr = &LightMgr::Instance();
	Init();
}

ShaderHelper::~ShaderHelper()
{
}

GLuint ShaderHelper::LoadShaders(ShaderInfo *info, GLuint size)
{

	GLuint program = glCreateProgram();

	GLint res = 0;
	for (GLuint i = 0; i < size; ++i)
	{
		GLuint shaderId = glCreateShader(info[i].shaderType);

		QFile file(info[i].shaderPath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			AddTipInfo(Q8("打开文件失败"));
			return 0;
		}

		QString strContent = file.readAll();

		QByteArray ba = strContent.toLocal8Bit();
		const GLchar *content = ba.data();

		glShaderSource(shaderId, 1, &(content), NULL);
		glCompileShader(shaderId);

		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &res);
		if (!res) {
			AddTipInfo(Q8("编译文件失败：%1").arg(info[i].shaderPath));
			GLchar cLog[520] = { 0 };
			glGetShaderInfoLog(shaderId, 520, NULL, cLog);
			AddTipInfo(cLog);
			return 0;
		}

		glAttachShader(program, shaderId);
	}

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (!res) {
		GLchar cLog[520] = { 0 };
		glGetProgramInfoLog(program, 520, NULL, cLog);
		AddTipInfo(cLog);
		return 0;
	}

	return program;
}

void ShaderHelper::GetCommonUniformLocation()
{
	if (m_programs[m_shaderType] != 0) {
		AddTipInfo(Q8("读取shader文件成功----[%1]").arg(m_shaderType));
	}
	else {
		AddTipInfo(Q8("读取shader文件失败----[%1]").arg(m_shaderType));
	}

	Use();

	//----matrix
	m_matMVPLoc[m_shaderType] = GetUniformLocation("mat_mvp");
	m_matWorldLoc[m_shaderType] = GetUniformLocation("mat_world");
	m_worldCamPosLoc[m_shaderType] = GetUniformLocation("worldCamPos");
	m_matProjLoc[m_shaderType] = GetUniformLocation("projMat");
	m_matOrthoLoc[m_shaderType] = GetUniformLocation("orthoMat");
	m_matViewLoc[m_shaderType] = GetUniformLocation("viewMat");
	m_matLightVPLoc[m_shaderType] = GetUniformLocation("lightVPMat");

	//----material
	m_ambientColorLoc[m_shaderType] = GetUniformLocation("ambientColor");
	m_specularColorLoc[m_shaderType] = GetUniformLocation("specularColor");

	//----time
	m_timeLoc[m_shaderType] = GetUniformLocation("time");

	//----light
	QString strTmp;
	for (int i = 0; i < maxLightNum; ++i)
	{
		strTmp = QString("lights[%1].isEnabled").arg(i);
		m_lightEnableLoc[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].isDirectional").arg(i);
		m_isDirectionalLightLoc[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].isPoint").arg(i);
		m_isPointLightLoc[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].dir").arg(i);
		m_lightDirLoc[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].pos").arg(i);
		m_lightPosLoc[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].color").arg(i);
		m_lightColorLoc[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].radius").arg(i);
		m_pointLightRadiusLoc[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].constant").arg(i);
		m_coefficientConstant[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].linear").arg(i);
		m_coefficientLinear[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].quadratic").arg(i);
		m_coefficientQuadratic[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].innerCutoff").arg(i);
		m_spotLightInnerCutoff[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
		strTmp = QString("lights[%1].outerCutoff").arg(i);
		m_spotLightOuterCutoff[m_shaderType][i] = GetUniformLocation(strTmp.toLocal8Bit());
	}
	//----set the static light data
// 	auto lightNum = m_lightMgrPtr->GetCurLightNum();
// 	for (int i = 0; i < lightNum; ++i)
// 	{
// 		SetLightsInfo(m_lightMgrPtr->GetLightInfo(i), i);
// 	}

	//----texture
	auto texId = GetUniformLocation("tex");
	if (-1 != texId) {
		glUniform1i(texId, 0);
	}
	auto normalMapId = GetUniformLocation("normalMap");
	if (-1 != normalMapId) {
		glUniform1i(normalMapId, 1);
	}
	auto projTexId = GetUniformLocation("projTex");
	if (-1 != projTexId) {
		glUniform1i(projTexId, 10);
	}
	auto shadowMapId = GetUniformLocation("shadowMap");
	if (-1 != shadowMapId) {
		glUniform1i(shadowMapId, 2);
	}
	auto skyboxId = GetUniformLocation("skybox");
	if (-1 != skyboxId)	{
		glUniform1i(skyboxId, 30);// why I should set it 2?? I don't understand here.....
	}
	auto offScreenTexId = GetUniformLocation("offScreenTex");
	if (-1 != offScreenTexId) {
		glUniform1i(offScreenTexId, 3);
	}

	Unuse();
}

void ShaderHelper::InitDefaultShader()
{
	m_shaderType = Default;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/triangle.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/triangle.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitPureColorShader()
{
	m_shaderType = PureColor;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/program1.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/program1.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitDiffuseShader()
{
	m_shaderType = Diffuse;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/diffuse.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/diffuse.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitPlaneClipShader()
{
	m_shaderType = PlaneClip;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/planeClip.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/planeClip.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitPointSpriteShader()
{
	m_shaderType = PointSprite;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/sprite.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/sprite.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitSkyboxShader()
{
	m_shaderType = Skybox;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/skybox.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/skybox.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitDecalShader()
{
	m_shaderType = Decal;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/decal.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/decal.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitFrameBuffer1Shader()
{
	m_shaderType = FrameBuffer1;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/framebuffer1.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/framebuffer1.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitBillBoardShader()
{
	m_shaderType = BillBoard;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/billboard.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/billboard.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitWaterShader()
{
	m_shaderType = Water;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/water.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/water.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::Init()
{
	initializeOpenGLFunctions();

	//----matrix relevant
	memset(m_matMVPLoc, -1, sizeof(m_matMVPLoc));
	memset(m_matWorldLoc, -1, sizeof(m_matWorldLoc));
	memset(m_worldCamPosLoc, -1, sizeof(m_worldCamPosLoc));
	memset(m_matProjLoc, -1, sizeof(m_matProjLoc));
	memset(m_matViewLoc, -1, sizeof(m_matViewLoc)); 
	memset(m_matOrthoLoc, -1, sizeof(m_matOrthoLoc));
	memset(m_ambientColorLoc, -1, sizeof(m_ambientColorLoc));
	memset(m_specularColorLoc, -1, sizeof(m_specularColorLoc));
	memset(m_matLightVPLoc, -1, sizeof(m_matLightVPLoc));
	memset(m_timeLoc, -1, sizeof(m_timeLoc));

	//----light relevant
	memset(m_lightEnableLoc, -1, sizeof(m_lightEnableLoc));
	memset(m_isDirectionalLightLoc, -1, sizeof(m_isDirectionalLightLoc));
	memset(m_isPointLightLoc, -1, sizeof(m_isPointLightLoc));
	memset(m_lightDirLoc, -1, sizeof(m_lightDirLoc));
	memset(m_lightPosLoc, -1, sizeof(m_lightPosLoc));
	memset(m_lightColorLoc, -1, sizeof(m_lightColorLoc));
	memset(m_pointLightRadiusLoc, -1, sizeof(m_pointLightRadiusLoc));
	memset(m_coefficientConstant, -1, sizeof(m_coefficientConstant));
	memset(m_coefficientLinear, -1, sizeof(m_coefficientLinear));
	memset(m_coefficientQuadratic, -1, sizeof(m_coefficientQuadratic));
	memset(m_spotLightInnerCutoff, -1, sizeof(m_spotLightInnerCutoff));
	memset(m_spotLightOuterCutoff, -1, sizeof(m_spotLightOuterCutoff));

	InitDefaultShader();
	InitPureColorShader();
	InitDiffuseShader();
	InitPlaneClipShader();
	InitPointSpriteShader();
	InitSkyboxShader();
	InitDecalShader();
	InitFrameBuffer1Shader();
	InitBillBoardShader();
	InitWaterShader();

	m_shaderType = Default;
	Use();
}

void ShaderHelper::SetShaderType(eShaderType type)
{
	m_shaderType = type;
	Use();
}

void ShaderHelper::SetMVPMatrix(QMatrix4x4 &matMVP, QMatrix4x4 &matWorld, QMatrix4x4 &matView, QMatrix4x4 &matProj)
{
	if (m_matMVPLoc[m_shaderType] != -1)
	{
		glUniformMatrix4fv(m_matMVPLoc[m_shaderType], 1, GL_FALSE, matMVP.data());
	}

	if (m_matWorldLoc[m_shaderType] != -1)
	{
		glUniformMatrix4fv(m_matWorldLoc[m_shaderType], 1, GL_FALSE, matWorld.data());
	}
	if (m_matViewLoc[m_shaderType] != -1)
	{
		glUniformMatrix4fv(m_matViewLoc[m_shaderType], 1, GL_FALSE, matView.data());
	}
	if (m_matProjLoc[m_shaderType] != -1)
	{
		glUniformMatrix4fv(m_matProjLoc[m_shaderType], 1, GL_FALSE, matProj.data());
	}
}

void ShaderHelper::SetCamWorldPos(QVector3D &camPos)
{
	if (m_worldCamPosLoc[m_shaderType] != -1)
	{
		glUniform3f(m_worldCamPosLoc[m_shaderType], camPos.x(), camPos.y(), camPos.z());
	}
}

void ShaderHelper::SetOrthoMat(QMatrix4x4 &matOrtho)
{
	if (m_matOrthoLoc[m_shaderType] != -1)
	{
		glUniformMatrix4fv(m_matOrthoLoc[m_shaderType], 1, GL_FALSE, matOrtho.data());
	}
}

void ShaderHelper::SetLightVPMat(QMatrix4x4 &matLightVP)
{
	if (m_matLightVPLoc[m_shaderType] != -1)
	{
		glUniformMatrix4fv(m_matLightVPLoc[m_shaderType], 1, GL_FALSE, matLightVP.data());
	}
}

void ShaderHelper::SetTime(unsigned int time)
{
	static unsigned int t = 0;
	if (m_timeLoc[m_shaderType] != -1)
	{
		glUniform1ui(m_timeLoc[m_shaderType], t);
		++t;
	}
}

void ShaderHelper::SetAmbientSpecularColor(QVector4D ambient, QVector4D specular)
{
	if (m_ambientColorLoc[m_shaderType] != -1) {
		glUniform4f(m_ambientColorLoc[m_shaderType], ambient.x(), ambient.y(), ambient.z(), ambient.w());
	}
	if (m_specularColorLoc[m_shaderType] != -1) {
		glUniform4f(m_specularColorLoc[m_shaderType], specular.x(), specular.y(), specular.z(), specular.w());
	}
}

void ShaderHelper::SetLightsInfo(const LightMgr::LightInfo &info, int index)
{
	if (m_lightEnableLoc[m_shaderType][index] != -1) {
		glUniform1i(m_lightEnableLoc[m_shaderType][index], info.isEnabled);
	}
	if (m_isDirectionalLightLoc[m_shaderType][index] != -1) {
		glUniform1i(m_isDirectionalLightLoc[m_shaderType][index], info.isDirectional);
	}
	if (m_isPointLightLoc[m_shaderType][index] != -1) {
		glUniform1i(m_isPointLightLoc[m_shaderType][index], info.isPoint);
	}

	if (m_lightDirLoc[m_shaderType][index] != -1) {
		glUniform3f(m_lightDirLoc[m_shaderType][index], info.dir.x(), info.dir.y(), info.dir.z());
	}
	if (m_lightPosLoc[m_shaderType][index] != -1) {
		glUniform3f(m_lightPosLoc[m_shaderType][index], info.pos.x(), info.pos.y(), info.pos.z());
	}
	if (m_lightColorLoc[m_shaderType][index] != -1) {
		glUniform4f(m_lightColorLoc[m_shaderType][index],
			info.color.x(), info.color.y(), info.color.z(), info.color.w());
	}

	if (m_pointLightRadiusLoc[m_shaderType][index] != -1) {
		glUniform1f(m_pointLightRadiusLoc[m_shaderType][index], info.radius);
	}

	if (m_coefficientConstant[m_shaderType][index] != -1) {
		glUniform1f(m_coefficientConstant[m_shaderType][index], info.constant);
	}
	if (m_coefficientLinear[m_shaderType][index] != -1) {
		glUniform1f(m_coefficientLinear[m_shaderType][index], info.linear);
	}
	if (m_coefficientQuadratic[m_shaderType][index] != -1) {
		glUniform1f(m_coefficientQuadratic[m_shaderType][index], info.quadratic);
	}

	if (m_spotLightInnerCutoff[m_shaderType][index] != -1) {
		glUniform1f(m_spotLightInnerCutoff[m_shaderType][index], info.innerCutoff);
	}
	if (m_spotLightOuterCutoff[m_shaderType][index] != -1) {
		glUniform1f(m_spotLightOuterCutoff[m_shaderType][index], info.outerCutoff);
	}
}

GLuint ShaderHelper::GetProgram() const
{
	return m_programs[m_shaderType];
}

void ShaderHelper::Use()
{
	glUseProgram(m_programs[m_shaderType]);
}

void ShaderHelper::Unuse()
{
	glUseProgram(0);
}

GLint ShaderHelper::GetAttriLocation(const GLchar *name)
{
	GLint res = glGetAttribLocation(m_programs[m_shaderType], name);
	if (-1 == res) {
// 		AddTipInfo(Q8("找不到Attrib:%1").arg(name));
		qDebug() << Q8("找不到Attrib:%1").arg(name);
	}
	return res;
}

GLint ShaderHelper::GetUniformLocation(const GLchar *name)
{
	GLint res = glGetUniformLocation(m_programs[m_shaderType], name);
	if (-1 == res) {
// 		AddTipInfo(Q8("找不到uniform:%1").arg(name));
		qDebug() << Q8("找不到Attrib:%1").arg(name);
	}
	return res;
}