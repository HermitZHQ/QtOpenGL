#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp/matrix4x4.h"
#include "QString"
#include "QMatrix4x4"
#include "ShaderHelper.h"
#include <map>

#define VS2019
#ifdef VS2019
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc141-mtd.lib")
#endif

class Mesh;
class Model;
class Assimp::Importer;
class AssetImport
{
public:
	~AssetImport();

	static AssetImport& Instance() {
		static AssetImport ins;
		return ins;
	}

	Model* LoadModel(const char *path);
	Model* LoadModelWithModelMatrixAndShaderType(const char *path, QMatrix4x4 &matModel, ShaderHelper::eShaderType type);
	int HandleChildNode(const aiScene *scene, aiNode *node, Model *mod);
	int HandleMeshMaterial(aiMaterial *mat, Mesh *mesh);

protected:
	void InitScene(const char *path);

private:
	AssetImport();

	const aiScene				*m_scene;
	Assimp::Importer			*m_importer;
	QString						m_prePath;
	QMatrix4x4					m_matModel;
	ShaderHelper::eShaderType	m_shaderType;
	Model						*m_model;
	aiMatrix4x4t<float>			m_matRootInverse;
	typedef std::vector<std::vector<aiVertexWeight> > WeightsPerVertexVec;
	WeightsPerVertexVec			m_weightsPerVertexVec;
};

