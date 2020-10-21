#pragma once
#include "QVector"
#include "QMatrix4x4"
#include "Mesh.h"
#include "ShaderHelper.h"
#include "Material.h"
#include "AABB.h"

class LightMgr;
class AnimationMgr;
class Model
{
public:

	Model();
	~Model();

	void AddMesh(Mesh *mesh);
	unsigned int GetMeshNum() const;
	Mesh* GetMesh(int index) const;
	
	void SetWroldMat(QMatrix4x4 worldMat);
	QMatrix4x4 GetWorldMat() const;

	void SetModelName(QString str);
	QString GetModelName() const;

	AABB GetAABB();
	void GenerateAABBMesh();

	void EnableSkybox();
	void EnableProjTex();
	void SetNormalDebugEnable(bool bEnable);

	Material& GetMaterial();
	void SetShaderType(ShaderHelper::eShaderType type);
	void SetDrawType(Mesh::eDrawType type);
	void SetNormalMapTextureByMeshName(const QString &path, const QString &meshName);
	void SetAllMeshesNormalMapTexture(const QString &path);
	void SetAllMeshesDiffuseTexture(const QString &path);

	void Draw(QMatrix4x4 matVP, QMatrix4x4 matModel, QVector3D camPos, QMatrix4x4 matProj, QMatrix4x4 matView,
		QMatrix4x4 matOrtho);

private:
	QString								m_name;
	QVector<Mesh*>						m_meshes;
	Mesh								*m_aabbMesh;
	QMatrix4x4							m_worldMat;
	ShaderHelper::eShaderType			m_shaderType;

	AnimationMgr						*m_animationMgrPtr;

	Material							m_material;
	ShaderHelper						*m_shaderHelperPtr;
	LightMgr							*m_lightMgrPtr;
	AABB								m_aabb;

	bool								m_enableNormalDebug;

	GLuint								m_time;
};

