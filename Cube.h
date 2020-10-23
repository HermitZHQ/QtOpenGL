#pragma once
#include "Object.h"
#include "QVector3D"

class Mesh;

// this class can generate a cube with two points
class Cube : public Object
{
public:
	Cube(const QVector3D &minPos, const QVector3D &maxPos);
	~Cube();

	Mesh* CreateMesh();

	virtual QString GetStaticClassName() const override;

private:
	QVector3D						m_points[8];
	float							m_long;
	float							m_width;
	float							m_height;
};

