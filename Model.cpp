#include "Model.h"
#include "Mesh.h"

Model::Model()
{
	m_worldMat.setToIdentity();
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

void Model::SetDrawType(Mesh::eDrawType type)
{
	for (auto &mesh : m_meshes)
	{
		mesh->SetDrawType(type);
	}
}

void Model::Draw(QMatrix4x4 matVP, QMatrix4x4 matModel, QVector3D camPos, QMatrix4x4 matProj, QMatrix4x4 matView,
	QMatrix4x4 matOrtho)
{
	auto meshNum = GetMeshNum();
	for (int i = 0; i < meshNum; ++i)
	{
		auto mesh = GetMesh(i);
		mesh->Draw(matVP, matModel, camPos, matProj, matView, matOrtho);
	}
}
