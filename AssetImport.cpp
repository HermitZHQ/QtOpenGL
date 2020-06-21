#include "AssetImport.h"
#include "Mesh.h"
#include "Model.h"
#include "ModelMgr.h"
#include "TextureMgr.h"
#include <string>
#include <assimp/mesh.h>
#include <assimp/ai_assert.h>
#include <AnimationMgr.h>

using namespace Assimp;

unsigned int ppsteps = aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
aiProcess_ValidateDataStructure | // perform a full validation of the loader's output
aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
aiProcess_RemoveRedundantMaterials | // remove redundant materials
aiProcess_FindDegenerates | // remove degenerated polygons from the import
aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
aiProcess_OptimizeMeshes | // join small meshes, if possible;
aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
// options....
aiProcess_GenSmoothNormals | // generate smooth normal vectors if not existing
aiProcess_SplitLargeMeshes | // split large, unrenderable meshes into submeshes
aiProcess_Triangulate | // triangulate polygons with more than 3 edges
aiProcess_ConvertToLeftHanded | // convert everything to D3D left handed space
aiProcess_SortByPType | // make 'clean' meshes which consist of a single typ of primitives

0;

unsigned int ppsteps_extra = 
aiProcess_FlipUVs
// | aiProcess_GenNormals
;

AssetImport::AssetImport()
	:m_model(nullptr), m_scene(nullptr)
{
	m_importer = new Assimp::Importer;
}

AssetImport::~AssetImport()
{
}

int AssetImport::LoadModel(const char *path)
{
	InitScene(path);

	m_model = nullptr;
	m_matModel.setToIdentity();
	m_shaderType = ShaderHelper::Default;
	HandleChildNode(m_scene, m_scene->mRootNode);

	return 0;
}

Model* AssetImport::LoadModelWithModelMatrixAndShaderType(const char *path, QMatrix4x4 &matModel, ShaderHelper::eShaderType type)
{
	InitScene(path);

	m_model = nullptr;
	m_matModel = matModel;
	m_shaderType = type;
	HandleChildNode(m_scene, m_scene->mRootNode);

	return m_model;
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

		// create a new model for a child
		Model *mod = nullptr;
		if (0 != child->mNumMeshes) {
			mod = ModelMgr::Instance().CreateNewModel();
			mod->SetModelName(child->mName.C_Str());
		}

		auto numMeshes = child->mNumMeshes;
		unsigned int baseBoneIndex = 0;
		for (unsigned int i = 0; i < 1; ++i)
		{
			if (nullptr == child->mMeshes) {
				continue;
			}
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
				if (nullptr != mesh->mTextureCoords[0])
				{
					info.uv1 = QVector2D(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
				}

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

			// ------collect bone info
			m_weightsPerVertexVec.clear();
			m_weightsPerVertexVec.resize(mesh->mNumVertices);
			for (unsigned int j = 0; j < mesh->mNumBones; ++j)
			{
				const aiBone *bone = mesh->mBones[j];
				for (unsigned int k = 0; k < bone->mNumWeights; ++k)
				{
					unsigned int vertexId = bone->mWeights[k].mVertexId;
					m_weightsPerVertexVec[vertexId].push_back(aiVertexWeight(j, bone->mWeights[k].mWeight));
				}
			}
			static const unsigned int max_bone_info_per_vertex = 4;
			// ------add bone info to vertex
			for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				unsigned char boneIndices[4] = { 0, 0, 0, 0 };
				float boneWeights[4] = { 0, 0, 0, 0 };

				if (mesh->HasBones()) {
					ai_assert(m_weightsPerVertexVec[j].size() <= 4);
					for (unsigned int a = 0; a < m_weightsPerVertexVec[j].size(); a++) {
						boneIndices[a] = static_cast<unsigned char>(m_weightsPerVertexVec[j][a].mVertexId);
						boneWeights[a] = (m_weightsPerVertexVec[j][a].mWeight);
					}
				}

				for (unsigned int b = 0; b < max_bone_info_per_vertex; ++b)
				{
					m->AddBoneId(boneIndices[b]);
					m->AddBoneWeight(boneWeights[b]);
				}
			}

			m->BindBuffer();
			mod->AddMesh(m);

			baseBoneIndex += mesh->mNumBones;
		}

		if (nullptr != mod)
		{
			mod->SetWroldMat(m_matModel);
			mod->SetShaderType(m_shaderType);

			// init animation
			auto animId = AnimationMgr::Instance().CreateAnimFromAiScene(scene);
			mod->SetAnimId(animId);
		}
		m_model = mod;
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

		auto texID = TextureMgr::Instance().LoadTexture(strPath.toLocal8Bit());
		mesh->SetDiffuseTexID(texID);
	}

	for (unsigned int i = 0; i < specularTexCount; ++i)
	{
		aiString strPath;
		mat->GetTexture(aiTextureType_SPECULAR, i, &strPath);
	}

	return 0;
}

void AssetImport::InitScene(const char *path)
{
	m_scene = m_importer->ReadFile(path,
		ppsteps
		| ppsteps_extra
	);
	if (!m_scene || m_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
	{
		return;
	}
	m_matRootInverse = m_scene->mRootNode->mTransformation;
	m_matRootInverse.Inverse();

	m_prePath = path;
	auto index = m_prePath.lastIndexOf("/");
	if (-1 != index) {
		m_prePath = m_prePath.left(index + 1);
	}
}
