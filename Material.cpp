#include "Material.h"


Material::Material()
	: m_metallic(0), m_roughness(0), m_ao(0)
{

}

Material::~Material() {

}
// 
// QString Material::GetClassName() const {
// 
// 	return "Material";
// }

void Material::SetMetallic(float metallic) {

	m_metallic = metallic;
}

float Material::GetMetallic() const {

	return m_metallic;
}

void Material::SetRoughness(float roughness) {

	m_roughness = roughness;
}

float Material::GetRoughness() const {

	return m_roughness;
}

void Material::SetAO(float ao) {

	m_ao = ao;
}

float Material::GetAO() const {

	return m_ao;
}

QString Material::GetStaticClassName() const
{
	return "Material";
}
