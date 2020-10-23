#include "Cube.h"
#include "ModelMgr.h"
#include "Model.h"

Cube::Cube(const QVector3D &minPos, const QVector3D &maxPos)
	:m_long(0), m_width(0), m_height(0)
{
	m_long = abs(minPos.x() - maxPos.x());
	m_height = abs(minPos.y() - maxPos.y());
	m_width = abs(minPos.z() - maxPos.z());

	// combine the 8 points of cube
	m_points[0] = minPos;
	m_points[1] = minPos + QVector3D(m_long, 0, 0);
	m_points[2] = minPos + QVector3D(0, m_height, 0);
	m_points[3] = minPos + QVector3D(0, 0, m_width);

	m_points[4] = maxPos;
	m_points[5] = maxPos - QVector3D(m_long, 0, 0);
	m_points[6] = maxPos - QVector3D(0, m_height, 0);
	m_points[7] = maxPos - QVector3D(0, 0, m_width);
}

Cube::~Cube()
{

}

Mesh* Cube::CreateMesh()
{
	Mesh *mesh = new Mesh;

	// generate the cube vertices and indices
	for (auto &pos : m_points) {
		mesh->AddVertex(pos);
	}

	// front & back
	mesh->AddIndex(0);
	mesh->AddIndex(2);
	mesh->AddIndex(1);
	mesh->AddIndex(2);
	mesh->AddIndex(7);
	mesh->AddIndex(1);

	mesh->AddIndex(3);
	mesh->AddIndex(6);
	mesh->AddIndex(4);
	mesh->AddIndex(3);
	mesh->AddIndex(4);
	mesh->AddIndex(5);

	// top & bottom
	mesh->AddIndex(2);
	mesh->AddIndex(5);
	mesh->AddIndex(7);
	mesh->AddIndex(5);
	mesh->AddIndex(4);
	mesh->AddIndex(7);

	mesh->AddIndex(0);
	mesh->AddIndex(1);
	mesh->AddIndex(6);
	mesh->AddIndex(6);
	mesh->AddIndex(3);
	mesh->AddIndex(0);

	// left & right
	mesh->AddIndex(1);
	mesh->AddIndex(7);
	mesh->AddIndex(4);
	mesh->AddIndex(4);
	mesh->AddIndex(6);
	mesh->AddIndex(1);

	mesh->AddIndex(3);
	mesh->AddIndex(5);
	mesh->AddIndex(2);
	mesh->AddIndex(2);
	mesh->AddIndex(0);
	mesh->AddIndex(3);

	mesh->BindBuffer();

	return mesh;
}

QString Cube::GetStaticClassName() const
{
	return "Cube";
}
