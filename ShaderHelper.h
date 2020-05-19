#pragma once
#include "QGL"
#include "QOpenGLFunctions_4_5_Core"

class QObject;
class ShaderHelper : public QOpenGLFunctions_4_5_Core
{
public:
	struct ShaderInfo 
	{
		GLuint			shaderType;
		GLchar			shaderPath[260];

		ShaderInfo(GLuint type, const GLchar *path)
			:shaderType(type)
		{
			strcpy_s(shaderPath, 260, path);
		}
	};

	ShaderHelper();
	~ShaderHelper();

	GLuint LoadShaders(ShaderInfo *info, GLuint size);
	GLuint GetProgram() const;
	void Use();
	void Unuse();

	GLint GetAttriLocation(const GLchar *name);
	GLint GetUniformLocation(const GLchar *name);

protected:

private:
	GLuint					m_pipeline;
	GLuint					m_program;
};

