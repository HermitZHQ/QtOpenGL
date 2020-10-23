#pragma once
#include "Object.h"
#include "QVector3D"

class Model;
class AABB : public Object
{
public:
	AABB();
	~AABB();

	virtual QString GetStaticClassName() const override;

	void GenerateFromModel(Model *mod);
	QVector3D GetMinPos() const;
	QVector3D GetMaxPos() const;

private:
	QVector3D						m_minPos;
	QVector3D						m_maxPos;
};
