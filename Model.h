#pragma once
#include "QVector"
#include "QMatrix4x4"
#include "Mesh.h"

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

	void EnableSkybox();
	void EnableProjTex();
	void SetDrawType(Mesh::eDrawType type);
	void Draw(QMatrix4x4 matVP, QMatrix4x4 matModel, QVector3D camPos, QMatrix4x4 matProj, QMatrix4x4 matView,
		QMatrix4x4 matOrtho);

private:
	QString								m_name;
	QVector<Mesh*>						m_meshes;
	QMatrix4x4							m_worldMat;
};

