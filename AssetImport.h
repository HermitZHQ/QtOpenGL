#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "QString"
#include "QMatrix4x4"
#include "ShaderHelper.h"

#pragma comment(lib, "assimp-vc141-mtd.lib")

class Mesh;
class AssetImport
{
public:
	~AssetImport();

	static AssetImport& Instance() {
		static AssetImport ins;
		return ins;
	}

	int LoadModel(const char *path);
	int LoadModelWithModelMatrixAndShaderType(const char *path, QMatrix4x4 &matModel, ShaderHelper::eShaderType type);
	int HandleChildNode(const aiScene *scene, aiNode *node);
	int HandleMeshMaterial(aiMaterial *mat, Mesh *mesh);

private:
	AssetImport();

	QString						m_prePath;
	QMatrix4x4					m_matModel;
	ShaderHelper::eShaderType	m_shaderType;
};

