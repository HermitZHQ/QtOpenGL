#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "QString"

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
	int HandleChildNode(const aiScene *scene, aiNode *node);
	int HandleMeshMaterial(aiMaterial *mat, Mesh *mesh);

private:
	AssetImport();

	QString						m_prePath;
};

