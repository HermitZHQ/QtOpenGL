#pragma once
#include "QList"

class Model;
class ModelMgr
{
public:
	~ModelMgr();
	static ModelMgr& Instance() {
		static ModelMgr ins;
		return ins;
	}
	Model* CreateNewModel();

	unsigned int GetModelNum() const;
	Model* GetModel(unsigned int index);
	Model* FindModelByName(QString name);

private:
	ModelMgr();

	QList<Model*>						m_modelList;
};

