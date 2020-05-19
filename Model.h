#pragma once
#include "QVector"
#include "QMatrix4x4"

class Mesh;
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

private:
	QString								m_name;
	QVector<Mesh*>						m_meshes;
	QMatrix4x4							m_worldMat;
};

