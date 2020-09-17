#pragma once
#include "Object.h"

class Material : Object
{
public:
	Material();
	~Material();

	virtual QString GetClassName() const override;

	void SetMetallic(float metallic);
	float GetMetallic() const;
	void SetRoughness(float roughness);
	float GetRoughness() const;
	void SetAO(float ao);
	float GetAO() const;

private:
	float						m_metallic;
	float						m_roughness;
	float						m_ao;
};

