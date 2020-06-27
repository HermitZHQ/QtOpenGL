#pragma once
#include "QOpenGLWidget"
#include "QOpenGLFunctions_4_5_Core"
#include "QOpenGLFunctions_4_5_Compatibility"
#include "QGLWidget"
#include "QTimer"
#include "ShaderHelper.h"

class Camera;
class Mesh;
class AssetImport;
class ModelMgr;
class LightMgr;
class OpenWidget : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
public:
	OpenWidget();
	~OpenWidget();

	void SetMainWndObj(QObject *obj) {
		m_mainObj = obj;
	}
	void ChangeMouseMoveSpeed(int value);
	void UpdateKeys();

protected:
	void TestGeometryPoints();
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;

	void SwitchShader(ShaderHelper::eShaderType type);
	void ClearAndReset();
	void UpdateDynamicLightsInfo();
	void UpdateAllLightsInfo();
	void UpdateAnimTime();
	virtual void paintGL() override;

	float lerp(float a, float b, float f);
	void GenerateHemisphereSamplers();

	void BeginGetOcclusionSampleNum();
	void EndGetOcclusionSampleNum();

	//---- frame buffer
	void CreateOffScreenFrameBufferTexture();
	void DrawOffScreenTexture();
	void DrawWaterWaveWithOffScreenTexture();

	void CreateShadowMapFrameBufferTexture();
	void DrawShadowMapTexture_ForTest();
	void DrawOriginalSceneWithShadow();

	void CreateGBufferFrameBufferTextures();
	void DrawDeferredShading();

	void CreateSSAOFrameBufferTextures();
	void CreateSSAOBlurFrameBufferTextures();

	virtual void moveEvent(QMoveEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
	QObject						*m_mainObj;
	GLint						m_matMVPLoc;
	GLint						m_matWorldLoc;
	GLint						m_worldCamPosLoc;
	Camera						*m_cam;
	AssetImport					*m_assimpPtr;
	ModelMgr					*m_modelMgrPtr;
	LightMgr					*m_lightMgrPtr;
	ShaderHelper				*m_shaderHelperPtr;

	GLuint						m_query;
	GLuint						m_sampleNum;

	QTimer						m_updateKeyTimer;
	QVector<Qt::Key>			m_pressedKeyVec;

	// ----FrameBuffer
	GLint						m_originalFbo;

	GLuint						m_offScreenFbo;
	GLuint						m_offScreenTexId;

	GLuint						m_shadowMapFbo;
	GLuint						m_shadowMapTexId;
	GLushort					m_shadowTexWidth;
	GLushort					m_shadowTexHeight;

	GLuint						m_gBufferFbo;
	GLuint						m_gBufferPosTex;
	GLuint						m_gBufferNormalTex;
	GLuint						m_gBufferAlbedoTex;
	GLuint						m_gBufferSkyboxTex;
	GLuint						m_gBufferDepthTex;

	GLuint						m_ssaoFbo;
	GLuint						m_ssaoTex;
	GLuint						m_ssaoNoiseTex;
	QVector<GLfloat>			m_noiseVec;
	QVector<QVector3D>			m_ssaoSampleVec;

	GLuint						m_ssaoBlurFbo;
	GLuint						m_ssaoBlurTex;
};

