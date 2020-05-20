#include "AssetImport.h"
#include "Mesh.h"
#include "Model.h"
#include "ModelMgr.h"
#include "Texture.h"
#include <string>

using namespace Assimp;

AssetImport::AssetImport()
{
}

AssetImport::~AssetImport()
{
}

int AssetImport::LoadModel(const char *path)
{
	Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate 
		| aiProcess_FlipUVs 
		| aiProcess_CalcTangentSpace
		/*| aiProcess_GenNormals*/);
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		return -1;
	}

	m_prePath = path;
	auto index = m_prePath.lastIndexOf("/");
	if (-1 != index) {
		m_prePath = m_prePath.left(index + 1);
	}

	HandleChildNode(scene, scene->mRootNode);

	return 0;
}

int AssetImport::HandleChildNode(const aiScene *scene, aiNode *node)
{
	unsigned int numChild = node->mNumChildren;
	for (unsigned int a = 0; a < numChild; ++a)
	{
		auto child = node->mChildren[a];
		if (child->mNumChildren > 0) {
			HandleChildNode(scene, child);
		}

		if (0) {
			continue;
		}

		// create a new model for a child
		Model *mod = ModelMgr::Instance().CreateNewModel();
		mod->SetModelName(child->mName.C_Str());

		auto numMeshes = child->mNumMeshes;
		for (unsigned int i = 0; i < numMeshes; ++i)
		{
			// create new sub mesh, then you can add it to one model
			Mesh *m = new Mesh;

			auto mesh = scene->mMeshes[child->mMeshes[i]];

			//handle the mesh's materials
			auto mat = scene->mMaterials[mesh->mMaterialIndex];
			HandleMeshMaterial(mat, m);
			// add vertex info
			for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				auto vert = mesh->mVertices[j];
				auto uv = mesh->mTextureCoords[0][j];
				Mesh::VertInfo info;

				info.pos = QVector3D(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
				info.color = QVector4D(0.8f, 0.8f, 0.8f, 1);
				info.normal = QVector3D(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
				if (nullptr != mesh->mTangents)
				{
					info.tangent = QVector4D(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z, 0);
				}
				if (nullptr != mesh->mBitangents)
				{
					info.bitangent = QVector4D(mesh->mBitangents[j].x, mesh->mBitangents[j].y, mesh->mBitangents[j].z, 0);
				}
				info.uv1 = QVector2D(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);

				m->AddVertInfo(info);
			}

			// add index info
			for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
			{
				auto face = mesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; ++k)
				{
					m->AddIndex(face.mIndices[k]);
				}
			}

			m->BindBuffer();
			mod->AddMesh(m);
		}
	}

	return 0;
}

int AssetImport::HandleMeshMaterial(aiMaterial *mat, Mesh *mesh)
{
	auto diffuseTexCount = mat->GetTextureCount(aiTextureType_DIFFUSE);
	auto specularTexCount = mat->GetTextureCount(aiTextureType_SPECULAR);

	for (unsigned int i = 0; i < diffuseTexCount; ++i)
	{
		aiString path;
		mat->GetTexture(aiTextureType_DIFFUSE, i, &path);
		QString strPath = path.C_Str();

		strPath.replace("/", "\\");
		auto index = strPath.lastIndexOf("\\");
		if (-1 != index) {
			strPath = strPath.right(strPath.length() - index - 1);
			strPath.prepend(m_prePath);
		}

		Texture *tex = new Texture;
		tex->LoadTexture(strPath.toLocal8Bit());
		mesh->AddDiffuseTexture(tex);
	}

	for (unsigned int i = 0; i < specularTexCount; ++i)
	{
		aiString strPath;
		mat->GetTexture(aiTextureType_SPECULAR, i, &strPath);
	}

	return 0;
}
