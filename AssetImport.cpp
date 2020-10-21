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

unsigned int ppsteps = 
 aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible，居然对圆形有bug，圆形用这个生成tan和bitan都是错误的，需要注意
	// 还需要注意的是，这里不开启的话，又会影响水面的NormalMap的计算结果，不开就是错误的.....，这尼玛的
//aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
//aiProcess_ValidateDataStructure | // perform a full validation of the loader's output
//aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
//aiProcess_RemoveRedundantMaterials | // remove redundant materials
aiProcess_FindDegenerates | // remove degenerated polygons from the import
aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
//aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
//aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
//aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
//aiProcess_OptimizeMeshes | // join small meshes, if possible;
//aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
// options....
aiProcess_GenSmoothNormals | // generate smooth normal vectors if not existing
//aiProcess_SplitLargeMeshes | // split large, unrenderable meshes into submeshes
aiProcess_Triangulate | // triangulate polygons with more than 3 edges
//aiProcess_ConvertToLeftHanded | // convert everything to D3D left handed space
//aiProcess_SortByPType | // make 'clean' meshes which consist of a single typ of primitives

// aiProcess_GlobalScale |
// aiProcess_FlipWindingOrder |
// aiProcess_GenSmoothNormals |
// aiProcess_ImproveCacheLocality |
// aiProcess_GenUVCoords |
// aiProcess_TransformUVCoords |
// aiProcess_FindInstances |
// aiProcess_OptimizeMeshes |
// aiProcess_PopulateArmatureData |

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

Model* AssetImport::LoadModel(const char *path)
{
	InitScene(path);

	m_model = nullptr;
	m_matModel.setToIdentity();
	m_shaderType = ShaderHelper::Default;

	// create a new model for a child
	Model *mod = nullptr;
	mod = ModelMgr::Instance().CreateNewModel();
	mod->SetModelName(m_scene->mMeshes[0]->mName.data);

	HandleChildNode(m_scene, m_scene->mRootNode, mod);

	return mod;
}

Model* AssetImport::LoadModelWithModelMatrixAndShaderType(const char *path, QMatrix4x4 &matModel, ShaderHelper::eShaderType type)
{
	InitScene(path);

	m_model = nullptr;
	m_matModel = matModel;
	m_shaderType = type;

	// create a new model for a child
	Model *mod = nullptr;
	mod = ModelMgr::Instance().CreateNewModel();
	mod->SetModelName(m_scene->mMeshes[0]->mName.data);

	HandleChildNode(m_scene, m_scene->mRootNode, mod);

	return m_model;
}

int AssetImport::HandleChildNode(const aiScene *scene, aiNode *node, Model *mod)
{
	unsigned int numChild = node->mNumChildren;
	for (unsigned int a = 0; a < numChild; ++a)
	{
		auto child = node->mChildren[a];
		if (child->mNumChildren > 0) {
			HandleChildNode(scene, child, mod);
		}

		auto numMeshes = child->mNumMeshes;
// 		unsigned int baseVertIndex = 0;
		for (unsigned int i = 0; i < numMeshes; ++i)
		{
			if (nullptr == child->mMeshes) {
				continue;
			}

			auto mesh = scene->mMeshes[child->mMeshes[i]];
			// create new sub mesh, then you can add it to one model
			Mesh *m = new Mesh;
			m->SetName(mesh->mName.data);

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
				if (nullptr != mesh->mNormals) {
					info.normal = QVector3D(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
				}
				if (nullptr != mesh->mTangents)	{
					info.tangent = QVector4D(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z, 0);
				}
				if (nullptr != mesh->mBitangents) {
					info.bitangent = QVector4D(mesh->mBitangents[j].x, mesh->mBitangents[j].y, mesh->mBitangents[j].z, 0);
				}
				if (nullptr != mesh->mTextureCoords[0])	{
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

			// 此数值不能随意修改，之前就是随意修改成6了，造成bug，因为必须和真实得shader中数量对应
			static const unsigned int max_bone_info_per_vertex = 8;
			// ------collect bone info
			m_weightsPerVertexVec.clear();
			m_weightsPerVertexVec.resize(mesh->mNumVertices);
			for (unsigned int j = 0; j < mesh->mNumBones; ++j)
			{
				const aiBone *bone = mesh->mBones[j];

				// for test
				//if (bone->mName == aiString("mouse")) {
				//	int iy = 0;
				//	++iy;
				//}

				for (unsigned int k = 0; k < bone->mNumWeights; ++k)
				{
					unsigned int vertexId = bone->mWeights[k].mVertexId;
					m_weightsPerVertexVec[vertexId].push_back(aiVertexWeight(j, bone->mWeights[k].mWeight));
				}
			}

			// check the max bone id&weight limit
			for (auto &info : m_weightsPerVertexVec)
			{
				ai_assert(info.size() <= max_bone_info_per_vertex);
			}

			// ------add bone info to vertex
			for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				unsigned char boneIndices[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
				float boneWeights[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

				if (mesh->HasBones()) {
					ai_assert(m_weightsPerVertexVec[j].size() <= max_bone_info_per_vertex);
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

			// init animation
			auto animId = AnimationMgr::Instance().CreateAnimFromAiScene(scene, mesh);
			m->SetAnimId(animId);
			m->BindBuffer();
			mod->AddMesh(m);
		}

		if (nullptr != mod)
		{
			mod->SetWroldMat(m_matModel);
			mod->SetShaderType(m_shaderType);
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
	// 修改点：非常重要，查了4，5个小时发现的大bug，不过也多亏了godot32中有这个处理我才能发现，虽然它其他的动画处理很头疼
	// 发现这个是因为改godot中的骨骼动画bug，参照的是assimp-viewer的流程，改完后发现不但能播放assimp-viewer能播放的，还能
	// 播放它播放不正常的，为了摸清真相，就进行了上面的长时间对比和查找不同点，最后发现就是这么一个属性.....
	m_importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

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
