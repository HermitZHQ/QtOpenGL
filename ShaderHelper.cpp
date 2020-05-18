#include "ShaderHelper.h"
#include "QFile"
#include "PreDef.h"

ShaderHelper::ShaderHelper()
	:m_obj(Q_NULLPTR), m_program(0)
{
}

ShaderHelper::~ShaderHelper()
{
}

GLuint ShaderHelper::LoadShaders(ShaderInfo *info, GLuint size)
{
	initializeOpenGLFunctions();

	m_program = glCreateProgram();

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
		auto len = strContent.length();

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

		glAttachShader(m_program, shaderId);
	}

	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &res);
	if (!res) {
		GLchar cLog[520] = { 0 };
		glGetProgramInfoLog(m_program, 520, NULL, cLog);
		AddTipInfo(cLog);
		return 0;
	}

	return m_program;
}

GLuint ShaderHelper::GetProgram() const
{
	return m_program;
}

void ShaderHelper::Use()
{
	glUseProgram(m_program);
}

void ShaderHelper::Unuse()
{
	glUseProgram(0);
}

GLint ShaderHelper::GetAttriLocation(const GLchar *name)
{
	return glGetAttribLocation(m_program, name);
}

GLint ShaderHelper::GetUniformLocation(const GLchar *name)
{
	return glGetUniformLocation(m_program, name);
}

void ShaderHelper::AddTipInfo(QString info)
{
	if (Q_NULLPTR != m_obj)
	{
		QMetaObject::invokeMethod(m_obj, "AddInfo", Q_ARG(QString, info));
	}
}
