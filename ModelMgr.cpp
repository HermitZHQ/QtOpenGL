#include "ModelMgr.h"
#include "Model.h"

ModelMgr::ModelMgr()
{
}

ModelMgr::~ModelMgr()
{
	for (auto &mod : m_modelList)
	{
		delete mod;
	}

	m_modelList.clear();
}

Model* ModelMgr::CreateNewModel()
{
	Model *mod = new Model;
	m_modelList.append(mod);

	return mod;
}

unsigned int ModelMgr::GetModelNum() const
{
	return m_modelList.size();
}

Model* ModelMgr::GetModel(unsigned int index)
{
	return m_modelList.at(index);
}

Model* ModelMgr::FindModelByName(QString name)
{
	for (auto &mod : m_modelList)
	{
		if (mod->GetModelName().compare(name) == 0) {
			return mod;
		}
	}

	return nullptr;
}
