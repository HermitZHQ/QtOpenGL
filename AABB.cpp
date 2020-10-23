#include "AABB.h"
#include "Model.h"

AABB::AABB()
{

}

AABB::~AABB()
{

}

QString AABB::GetStaticClassName() const
{
	return "AABB";
}

void AABB::GenerateFromModel(Model *mod)
{
	if (nullptr == mod) {
		return;
	}

	float minX = 0, minY = 0, minZ = 0;
	float maxX = 0, maxY = 0, maxZ = 0;
	bool bInitFlag = false;
	auto meshNum =  mod->GetMeshNum();
	for (auto i = 0; i < meshNum; ++i) {
		Mesh *mesh = mod->GetMesh(i);
		const float *vertexPtr = mesh->GetVertices();
		auto vertexNum = mesh->GetVerticesNum();
		if (!bInitFlag) {
			minX = maxX = vertexPtr[0];
			minY = maxY = vertexPtr[1];
			minZ = maxZ = vertexPtr[2];
			bInitFlag = true;
		}

		for (auto vertIdx = 0; vertIdx < vertexNum; vertIdx += 3) {
			minX = vertexPtr[vertIdx] < minX ? vertexPtr[vertIdx] : minX;
			maxX = vertexPtr[vertIdx] > maxX ? vertexPtr[vertIdx] : maxX;

			minY = vertexPtr[vertIdx + 1] < minY ? vertexPtr[vertIdx + 1] : minY;
			maxY = vertexPtr[vertIdx + 1] > maxY ? vertexPtr[vertIdx + 1] : maxY;

			minZ = vertexPtr[vertIdx + 2] < minZ ? vertexPtr[vertIdx + 2] : minZ;
			maxZ = vertexPtr[vertIdx + 2] > maxZ ? vertexPtr[vertIdx + 2] : maxZ;
		}
	}

	m_minPos = QVector3D(minX, minY, minZ);
	m_maxPos = QVector3D(maxX, maxY, maxZ);
}

QVector3D AABB::GetMinPos() const
{
	return m_minPos;
}

QVector3D AABB::GetMaxPos() const
{
	return m_maxPos;
}
