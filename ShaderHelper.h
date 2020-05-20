#pragma once
#include "QGL"
#include "QOpenGLFunctions_4_5_Core"

class QObject;
class ShaderHelper : public QOpenGLFunctions_4_5_Core
{
public:
	enum eShaderType
	{
		ShaderDefault,
		ShaderPureColor,
		ShaderDiffuse,
	};

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

	~ShaderHelper();

	static ShaderHelper& Instance() {
		static ShaderHelper ins;
		return ins;
	}

	void SetShaderType(eShaderType type);
	void SetMVPMatrix(QMatrix4x4 &matMVP);
	void SetWorldMatrix(QMatrix4x4 &matWorld);
	void SetCamWorldPos(QVector3D &camPos);

	GLuint GetProgram() const;
	void Use();
	void Unuse();

	GLint GetAttriLocation(const GLchar *name);
	GLint GetUniformLocation(const GLchar *name);

protected:
	void Init();
	GLuint LoadShaders(ShaderInfo *info, GLuint size);
	void GetCommonUniformLocation();
	void InitDefaultShader();
	void InitPureColorShader();
	void InitDiffuseShader();

private:
	ShaderHelper();

	const static int		maxShaderNum = 8;
	GLuint					m_programs[maxShaderNum];
	eShaderType				m_shaderType;

	// common share uniform location
	GLint					m_matMVPLoc[maxShaderNum];
	GLint					m_matWorldLoc[maxShaderNum];
	GLint					m_worldCamPosLoc[maxShaderNum];
};

