#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp/matrix4x4.h"
#include "QString"
#include "QMatrix4x4"
#include "ShaderHelper.h"
#include <map>

#pragma comment(lib, "assimp-vc141-mtd.lib")

class Mesh;
class Model;
class AssetImport
{
public:
	~AssetImport();

	static AssetImport& Instance() {
		static AssetImport ins;
		return ins;
	}

	int LoadModel(const char *path);
	Model* LoadModelWithModelMatrixAndShaderType(const char *path, QMatrix4x4 &matModel, ShaderHelper::eShaderType type);
	int HandleChildNode(const aiScene *scene, aiNode *node);
	int HandleMeshMaterial(aiMaterial *mat, Mesh *mesh);

private:
	AssetImport();

	QString						m_prePath;
	QMatrix4x4					m_matModel;
	ShaderHelper::eShaderType	m_shaderType;
	Model						*m_model;
	aiMatrix4x4t<float>			m_matRootInverse;
	typedef std::map<std::string, GLuint> BoneMap;
	BoneMap						m_boneMap;
};

