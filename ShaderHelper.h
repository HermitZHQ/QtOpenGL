#pragma once
#include "QGL"
#include "QOpenGLFunctions_4_5_Core"
#include "LightMgr.h"

class QObject;
class ShaderHelper : public QOpenGLFunctions_4_5_Core
{
public:
	enum eShaderType
	{
		Default,
		PureColor,
		Diffuse,
		PlaneClip,
		PointSprite,
		Skybox,
		SkyboxGBuffer,
		Decal,
		FrameBuffer1,
		BillBoard,
		Water,
		GBufferGeometry,
		DefferredRendering,
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
	void SetMVPMatrix(QMatrix4x4 &matMVP, QMatrix4x4 &matWorld, QMatrix4x4 &matView, QMatrix4x4 &matProj);
	void SetCamWorldPos(QVector3D &camPos);
	void SetOrthoMat(QMatrix4x4 &matOrtho);
	void SetLightVPMat(QMatrix4x4 &matLightVP);

	void SetTime(unsigned int time);

	//----Light relevant
	void SetAmbientSpecularColor(QVector4D ambient, QVector4D specular);
	void SetLightsInfo(const LightMgr::LightInfo &info, int index);

	//----Shader relevant
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
	void InitPlaneClipShader();
	void InitPointSpriteShader();
	void InitSkyboxShader();
	void InitDecalShader();
	void InitFrameBuffer1Shader();
	void InitBillBoardShader();
	void InitWaterShader();
	void InitGBufferGeometryShader();
	void InitDeferredRenderingShader();
	void InitSkyboxGBufferShader();

private:
	ShaderHelper();

	const static int		maxShaderNum = 20;
	GLuint					m_programs[maxShaderNum];
	eShaderType				m_shaderType;

	LightMgr				*m_lightMgrPtr;

	// common share uniform location
	GLint					m_matMVPLoc[maxShaderNum];
	GLint					m_matWorldLoc[maxShaderNum];
	GLint					m_worldCamPosLoc[maxShaderNum];
	GLint					m_matProjLoc[maxShaderNum];
	GLint					m_matOrthoLoc[maxShaderNum];
	GLint					m_matViewLoc[maxShaderNum];
	GLint					m_matLightVPLoc[maxShaderNum];

	GLint					m_ambientColorLoc[maxShaderNum];
	GLint					m_specularColorLoc[maxShaderNum];

	GLint					m_timeLoc[maxShaderNum];

	// uniform struct for light info
	const static int		maxLightNum = LightMgr::m_maxLightNum;
	GLint					m_lightEnableLoc[maxShaderNum][maxLightNum];
	GLint					m_isDirectionalLightLoc[maxShaderNum][maxLightNum];
	GLint					m_isPointLightLoc[maxShaderNum][maxLightNum];
	GLint					m_lightDirLoc[maxShaderNum][maxLightNum];
	GLint					m_lightPosLoc[maxShaderNum][maxLightNum];
	GLint					m_lightColorLoc[maxShaderNum][maxLightNum];
	GLint					m_pointLightRadiusLoc[maxShaderNum][maxLightNum];
	GLint					m_coefficientConstant[maxShaderNum][maxLightNum];
	GLint					m_coefficientLinear[maxShaderNum][maxLightNum];
	GLint					m_coefficientQuadratic[maxShaderNum][maxLightNum];
	GLint					m_spotLightInnerCutoff[maxShaderNum][maxLightNum];
	GLint					m_spotLightOuterCutoff[maxShaderNum][maxLightNum];
};

