#pragma once
#include "Object.h"

class Material : public Object
{
public:
	Material();
	~Material();

// 	QString GetClassName() const;

	void SetMetallic(float metallic);
	float GetMetallic() const;
	void SetRoughness(float roughness);
	float GetRoughness() const;
	void SetAO(float ao);
	float GetAO() const;


	virtual QString GetStaticClassName() const override;

private:
	float						m_metallic;
	float						m_roughness;
	float						m_ao;
};

