#include "Model.h"
#include "Mesh.h"
#include "Cube.h"
#include "LightMgr.h"
#include "AnimationMgr.h"
#include <windows.h>

Model::Model()
	:m_shaderType(ShaderHelper::Default)
	, m_enableNormalDebug(false)
	, m_animationMgrPtr(&AnimationMgr::Instance())
	, m_time(GetTickCount())
	, m_aabbMesh(nullptr)
{
	m_worldMat.setToIdentity();

	m_lightMgrPtr = &LightMgr::Instance();
	m_shaderHelperPtr = &ShaderHelper::Instance();
}

Model::~Model()
{
	for (auto &mesh : m_meshes)
	{
		delete mesh;
	}
}

void Model::AddMesh(Mesh *mesh)
{
	m_meshes.append(mesh);
}

unsigned int Model::GetMeshNum() const
{
	return m_meshes.size();
}

Mesh* Model::GetMesh(int index) const
{
	if (index > m_meshes.size() - 1) {
		return nullptr;
	}

	return m_meshes[index];
}

void Model::SetWroldMat(QMatrix4x4 worldMat)
{
	m_worldMat = worldMat;
}

QMatrix4x4 Model::GetWorldMat() const
{
	return m_worldMat;
}

void Model::SetModelName(QString str)
{
	m_name = str;
}

QString Model::GetModelName() const
{
	return m_name;
}

AABB Model::GetAABB()
{
	m_aabb.GenerateFromModel(this);
	return m_aabb;
}

void Model::GenerateAABBMesh()
{
	if (nullptr == m_aabbMesh) {
		auto aabb = GetAABB();
		Cube cube(aabb.GetMinPos(), aabb.GetMaxPos());
		m_aabbMesh = cube.CreateMesh();
		m_aabbMesh->SetDrawType(Mesh::Line);
	}
}

void Model::EnableSkybox()
{
	for (auto &mesh : m_meshes)
	{
		mesh->InitSkybox();
	}
}

void Model::EnableProjTex()
{
	for (auto &mesh : m_meshes)
	{
		mesh->InitProjTex();
	}
}

void Model::SetNormalDebugEnable(bool bEnable)
{
	m_enableNormalDebug = bEnable;
}

Material& Model::GetMaterial() {

	return m_material;
}

void Model::SetShaderType(ShaderHelper::eShaderType type)
{
	m_shaderType = type;
}

void Model::SetDrawType(Mesh::eDrawType type)
{
	for (auto &mesh : m_meshes)
	{
		mesh->SetDrawType(type);
	}
}

void Model::SetNormalMapTextureByMeshName(const QString &path, const QString &meshName)
{
	for (auto &mesh : m_meshes)
	{
		if (mesh->GetName() == meshName) {
			mesh->SetNormalMapTexture(path);
			return;
		}
	}
}

void Model::SetAllMeshesNormalMapTexture(const QString &path)
{
	for (auto &mesh : m_meshes)
	{
		mesh->SetNormalMapTexture(path);
	}
}

void Model::SetAllMeshesDiffuseTexture(const QString &path)
{
	for (auto &mesh : m_meshes)
	{
		mesh->SetDiffuseTexture(path);
	}
}

void Model::Draw(QMatrix4x4 matVP, QMatrix4x4 matModel, QVector3D camPos, QMatrix4x4 matProj, QMatrix4x4 matView,
	QMatrix4x4 matOrtho)
{
	m_time = GetTickCount() - m_time;

	// switch the shader type and set the light data before we draw
	m_shaderHelperPtr->SetShaderType(m_shaderType);
	auto lightNum = m_lightMgrPtr->GetCurLightNum();
	for (int i = 0; i < lightNum; ++i)
	{
		auto &lightInfo = m_lightMgrPtr->GetLightInfo(i);
		m_shaderHelperPtr->SetLightsInfo(lightInfo, i);
	}

	auto meshNum = GetMeshNum();
	for (unsigned int i = 0; i < meshNum; ++i)
	{
		auto mesh = GetMesh(i);
		if (m_time == 0) {
			m_time = 2;
		}
		m_animationMgrPtr->UpdateAnimation(mesh->GetAnimId(), m_time / 1000.0f);
		mesh->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);

		// Geometry normal display
		if (m_enableNormalDebug) {
			m_shaderHelperPtr->SetShaderType(ShaderHelper::Geometry);
			mesh->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
			m_shaderHelperPtr->SetShaderType(m_shaderType);
		}
	}

	// aabb box
	if (m_aabbMesh) {
		m_aabbMesh->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}

	m_time = GetTickCount();
}