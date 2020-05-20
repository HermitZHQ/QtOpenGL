#include "ShaderHelper.h"
#include "QFile"
#include "PreDef.h"
#include "QMatrix4x4"

ShaderHelper::ShaderHelper()
{
	Init();
}

ShaderHelper::~ShaderHelper()
{
}

GLuint ShaderHelper::LoadShaders(ShaderInfo *info, GLuint size)
{
	initializeOpenGLFunctions();

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

	m_matMVPLoc[m_shaderType] = GetUniformLocation("mat_mvp");
	m_matWorldLoc[m_shaderType] = GetUniformLocation("mat_world");
	m_worldCamPosLoc[m_shaderType] = GetUniformLocation("worldCamPos");

	auto texId = GetUniformLocation("tex");
	auto normalMapId = GetUniformLocation("normalMap");
	glUniform1i(texId, 0);
	glUniform1i(normalMapId, 1);

	if (-1 == m_matMVPLoc[m_shaderType] || -1 == m_matWorldLoc[m_shaderType]
		|| -1 == m_worldCamPosLoc[m_shaderType]) {
		AddTipInfo(Q8("查询uniform失败！"));
	}

	Unuse();
}

void ShaderHelper::InitDefaultShader()
{
	m_shaderType = ShaderDefault;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/triangle.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/triangle.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitPureColorShader()
{
	m_shaderType = ShaderPureColor;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/program1.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/program1.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::InitDiffuseShader()
{
	m_shaderType = ShaderDiffuse;

	ShaderHelper::ShaderInfo info[] = {
		{GL_VERTEX_SHADER, "./shaders/diffuse.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/diffuse.frag"}
	};
	m_programs[m_shaderType] = LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));

	GetCommonUniformLocation();
}

void ShaderHelper::Init()
{
	InitDefaultShader();
	InitPureColorShader();
	InitDiffuseShader();

	m_shaderType = ShaderDefault;
	Use();
}

void ShaderHelper::SetShaderType(eShaderType type)
{
	m_shaderType = type;
	Use();
}

void ShaderHelper::SetMVPMatrix(QMatrix4x4 &matMVP)
{
	glUniformMatrix4fv(m_matMVPLoc[m_shaderType], 1, GL_FALSE, matMVP.data());
}

void ShaderHelper::SetWorldMatrix(QMatrix4x4 &matWorld)
{
	glUniformMatrix4fv(m_matWorldLoc[m_shaderType], 1, GL_FALSE, matWorld.data());
}

void ShaderHelper::SetCamWorldPos(QVector3D &camPos)
{
	glUniform3f(m_worldCamPosLoc[m_shaderType], camPos.x(), camPos.y(), camPos.z());
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
	return glGetAttribLocation(m_programs[m_shaderType], name);
}

GLint ShaderHelper::GetUniformLocation(const GLchar *name)
{
	return glGetUniformLocation(m_programs[m_shaderType], name);
}