#pragma once
#include "QGL"
#include "QOpenGLFunctions_4_5_Core"
#include "LightMgr.h"

class QObject;
class ShaderHelper : public QOpenGLFunctions_4_5_Core
{
public:
	const static int		ssaoSampleNum = 32;
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
		SSAO,
		SSAOBlur,
		Geometry,
        MotionBlur,
        ScreenQuad,
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
	eShaderType GetShaderType() const;
	void SetMVPMatrix(QMatrix4x4 &matMVP, QMatrix4x4 &matWorld, QMatrix4x4 &matView, QMatrix4x4 &matProj);
	void SetCamWorldPos(QVector3D &camPos);
	void SetOrthoMat(QMatrix4x4 &matOrtho);
	void SetLightVPMat(QMatrix4x4 &matLightVP);

	//----Bones relevant
	void SetBonesInfo(QVector<QMatrix4x4> &bonesInfoVec);

	void SetTime(unsigned int time);

	//----Light relevant
	void SetAmbientSpecularColor(QVector4D ambient, QVector4D specular);
	void SetLightsInfo(const LightMgr::LightInfo &info, int index);

    void SetMeltThreshold(float threshold);
	void SetInsta360(QMatrix4x4 mat);

	//----PBR relevant
	void SetPBR(float metallic, float roughness, float ao);
	void SetPBR_Metallic(float metallic);
	void SetPBR_Roughness(float roughness);
	void SetPBR_AO(float ao);

	// ----Tex relevant
	void SetHasNormalMap(bool flag);

	//----SSAO relevant
	void SetSSAOSamples(QVector<QVector3D> &sampleVec);

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
	void InitSSAOShader();
	void InitSSAOBlurShader();
	void InitGeometryShader();
    void InitMotionBlurShader();
    void InitScreenQuadShader();

private:
	ShaderHelper();

	const static int		maxShaderNum = 50;
	const static int		maxBoneNum = 200;
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

	// bones location
	GLint					m_bonesLoc[maxShaderNum][maxBoneNum];

	GLint					m_ambientColorLoc[maxShaderNum];
	GLint					m_specularColorLoc[maxShaderNum];
    GLint                   m_meltLoc[maxShaderNum];
	GLint					m_insta360Loc[maxShaderNum];

	//----PBR params
	GLint					m_metallic[maxShaderNum];
	GLint					m_roughness[maxShaderNum];
	GLint					m_ao[maxShaderNum];

	// ----Tex
	GLint					m_hasNormalMap[maxShaderNum];

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

	// uniform for the SSAO samples
	GLint					m_ssaoSamples[maxShaderNum][ssaoSampleNum];
};

