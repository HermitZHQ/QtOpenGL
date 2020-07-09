#include "Mesh.h"
#include "TextureMgr.h"
#include "ShaderHelper.h"
#include "PreDef.h"
#include "QMatrix4x4"
#include "AnimationMgr.h"

Mesh::Mesh()
	:m_vao(0), m_vbo(0), m_vaeo(0), m_vaeo_lines(0), m_instanceBufferId(0), m_tbo1(0)
	, m_drawType(Triangle)
	, m_skyboxTexID(0), m_projTexID(0), m_normalmapTexID(0)
	, m_diffuseTex1ID(0), m_shader(ShaderHelper::Instance())
	, m_mainWnd(GetGlobalMainWndPtr())
	, m_animId(0)
{
	initializeOpenGLFunctions();

	m_normalmapTexID = TextureMgr::Instance().LoadTexture("./models/brickwall_normal.jpg");
}

Mesh::~Mesh()
{

}

void Mesh::AddVertex(QVector3D vert)
{
	m_vertices.push_back(vert.x());
	m_vertices.push_back(vert.y());
	m_vertices.push_back(vert.z());
}

const float* Mesh::GetVertices() const
{
	return m_vertices.data();
}

int Mesh::GetVerticesMemSize() const
{
	return m_vertices.size() * sizeof(float);
}

int Mesh::GetVerticesNum() const
{
	return m_vertices.size() / 3;
}

void Mesh::AddColor(QVector4D color)
{
	m_colors.push_back(color.x());
	m_colors.push_back(color.y());
	m_colors.push_back(color.z());
	m_colors.push_back(color.w());
}

const float* Mesh::GetColors() const
{
	return m_colors.data();
}

int Mesh::GetColorsMemSize() const
{
	return m_colors.size() * sizeof(float);
}

void Mesh::AddNormal(QVector3D normal)
{
	m_normals.append(normal.x());
	m_normals.append(normal.y());
	m_normals.append(normal.z());
}

const float* Mesh::GetNormals() const
{
	return m_normals.data();
}

int Mesh::GetNormalsMemSize() const
{
	return m_normals.size() * sizeof(float);
}

int Mesh::GetNormalsNum() const
{
	return m_normals.size() / 3;
}

void Mesh::AddTangent(QVector4D tangent)
{
	m_tangents.append(tangent.x());
	m_tangents.append(tangent.y());
	m_tangents.append(tangent.z());
	m_tangents.append(tangent.w());
}

const float* Mesh::GetTangents() const
{
	return m_tangents.data();
}

int Mesh::GetTangentsMemSize() const
{
	return m_tangents.size() * sizeof(float);
}

int Mesh::GetTangentsNum() const
{
	return m_tangents.size() / 4;
}

void Mesh::AddBitangent(QVector4D binormal)
{
	m_binormals.append(binormal.x());
	m_binormals.append(binormal.y());
	m_binormals.append(binormal.z());
	m_binormals.append(binormal.w());
}

const float* Mesh::GetBitangents() const
{
	return m_binormals.data();
}

int Mesh::GetBitangentsMemSize() const
{
	return m_binormals.size() * sizeof(float);
}

int Mesh::GetBitangentsNum() const
{
	return m_binormals.size() / 4;
}

void Mesh::AddUv1(QVector2D uv)
{
	m_uvs1.append(uv.x());
	m_uvs1.append(uv.y());
}

const float* Mesh::GetUvs1() const
{
	return m_uvs1.data();
}

int Mesh::GetUvs1MemSize() const
{
	return m_uvs1.size() * sizeof(float);
}

int Mesh::GetUvs1Num() const
{
	return m_uvs1.size() / 2;
}

void Mesh::AddVertInfo(const VertInfo &info)
{
	m_vertInfoVec.push_back(info);

	AddVertex(info.pos);
	AddColor(info.color);
	AddNormal(info.normal);
	AddTangent(info.tangent);
	AddBitangent(info.bitangent);
	AddUv1(info.uv1);
}

void Mesh::BindBuffer()
{
	// init the shaders first
	ShaderHelper::Instance();

	BindVertexRelevantBuffer();
}

void Mesh::BindVertexRelevantBuffer()
{
	const GLuint *vertex_indices = GetIndices();

	const GLfloat *vertex_positions = GetVertices();
	const GLfloat *vertex_uvs = GetUvs1();
	const GLfloat *vertex_tangents = GetTangents();
	const GLfloat *vertex_bitangents = GetBitangents();
	const GLfloat *vertex_normals = GetNormals();
	const GLfloat *vertex_boneIds = GetBoneIds();
	const GLfloat *vertex_boneWeights = GetBoneWeights();

	BuildLinesIndicesFromTrianglesIndices();

	//--------------------------You must gen and bind the VAO first, the after operations all depends on it!!!!!!
	// set vertex array object
	glGenVertexArrays(1, &m_vao);
	CheckError;
	glBindVertexArray(m_vao);
	CheckError;

	// set element array(index array) buffer
	glGenBuffers(1, &m_vaeo);
	CheckError;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vaeo);
	CheckError;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetIndicesMemSize(), vertex_indices, GL_STATIC_DRAW);
	CheckError;
	glVertexArrayElementBuffer(m_vao, m_vaeo);
	CheckError;

	// also create the lines indices at the same time, so we can switch them dynamicly
	glGenBuffers(1, &m_vaeo_lines);
	CheckError;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vaeo_lines);
	CheckError;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetLinesIndicesMemSize(), GetLinesIndices(), GL_STATIC_DRAW);
	CheckError;

	glGenBuffers(1, &m_vbo);
	CheckError;
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	CheckError;
	glBufferData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize()
		/*+ GetBoneIdsMemSize() + GetBoneWeightsMemSize()*/, NULL, GL_STATIC_DRAW);
	CheckError;
	glBufferSubData(GL_ARRAY_BUFFER, 0, GetVerticesMemSize(), vertex_positions);
	CheckError;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize(), GetUvs1MemSize(), vertex_uvs);
	CheckError;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize(), GetTangentsMemSize(), vertex_tangents);
	CheckError;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize(),
		GetBitangentsMemSize(), vertex_bitangents);
	CheckError;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize(),
		GetNormalsMemSize(), vertex_normals);
	CheckError;
	
	// 骨骼相关Buffer，在下面会单独生成
// 	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
// 		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize(),
// 		GetBoneIdsMemSize(), vertex_boneIds);
// 	CheckError;
// 	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
// 		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize() + GetBoneIdsMemSize(),
// 		GetBoneWeightsMemSize(), vertex_boneWeights);
// 	CheckError;

	GLint positionLoc = 0;
	GLint uvLoc = 1;
	GLint tangentLoc = 2;
	GLint bitangentLoc = 3;
	GLint normalLoc = 4;
	GLint modelMatLoc = -1;
	GLint boneIdsLoc = 5;
	GLint boneWeightsLoc = 7;// offset by 2(mat4x2)

	auto boneIdSize = GetBoneIdsMemSize();
	auto boneNum = GetBoneIdsNum();
	auto boneWeightSize = GetBoneWeightsMemSize();
	auto boneWeightNum = GetBoneWeightsNum();

	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, (0));
	CheckError;
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize()));
	CheckError;
	glEnableVertexAttribArray(uvLoc);
	CheckError;
	glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize() + GetUvs1MemSize()));
	CheckError;
	glEnableVertexAttribArray(tangentLoc);
	CheckError;

	glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize()));
	CheckError;
	glEnableVertexAttribArray(bitangentLoc);

	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize() + GetBitangentsMemSize()));
	CheckError;
	glEnableVertexAttribArray(normalLoc);

	// 不能使用glEnableVertexAttribArray来设置GL_INT类型，shader中读取的数据不正常，但是官方又说可以使用两种来设置glint类型
	// 但是有说-I的类型是专门设置GL_INT或者uint的，反正感觉说的很模糊，只能当是经验积累学习了
	//----Bone Ids(8)
	GLuint boneIdBuffer = 0;
	glGenBuffers(1, &boneIdBuffer);
	CheckError;
	glBindBuffer(GL_ARRAY_BUFFER, boneIdBuffer);
	CheckError;
	glBufferData(GL_ARRAY_BUFFER, GetBoneIdsMemSize(), vertex_boneIds, GL_STATIC_DRAW);
	CheckError;
	glVertexAttribPointer(boneIdsLoc, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(0));
	CheckError;
	glEnableVertexAttribArray(boneIdsLoc);

	glVertexAttribPointer(boneIdsLoc + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(sizeof(GL_FLOAT) * 4));
	CheckError;
	glEnableVertexAttribArray(boneIdsLoc + 1);

	//----Bone Weights(8)
	GLuint boneWeightBuffer = 0;
	glGenBuffers(1, &boneWeightBuffer);
	CheckError;
	glBindBuffer(GL_ARRAY_BUFFER, boneWeightBuffer);
	CheckError;
	glBufferData(GL_ARRAY_BUFFER, GetBoneWeightsMemSize(), vertex_boneWeights, GL_STATIC_DRAW);
	CheckError;
	glVertexAttribPointer(boneWeightsLoc, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8,
		(void*)(0));
	CheckError;
	glEnableVertexAttribArray(boneWeightsLoc);
	CheckError;

	glVertexAttribPointer(boneWeightsLoc + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8,
		(void*)(sizeof(GL_FLOAT) * 4));
	CheckError;
	glEnableVertexAttribArray(boneWeightsLoc + 1);
	CheckError;

	// multi instances model matrix
// 	if (-1 != modelMatLoc) {
// 		// generate the new buffer for instances
// 		glBindBuffer(GL_ARRAY_BUFFER, 0);
// 		glGenBuffers(1, &m_instanceBufferId);
// 		glBindBuffer(GL_ARRAY_BUFFER, m_instanceBufferId);
// 		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * 10, nullptr, GL_STATIC_DRAW);
// 
// 		// 		m_instanceBufferId = m_vbo;
// 
// 		// mat4 type take space of 4 vec4, so we should circle 4 times
// 		for (int i = 0; i < 4; ++i)
// 		{
// 			glVertexAttribPointer(modelMatLoc + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16,
// 				(void*)(/*GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize() +*/ sizeof(GLfloat) * 4 * i));
// 			glEnableVertexAttribArray(modelMatLoc + i);
// 			// implement the multi instances
// 			glVertexAttribDivisor(modelMatLoc + i, 1);
// 		}
// 
// 		glBindBuffer(GL_ARRAY_BUFFER, 0);
// 	}

	// sampler buffer bind
// 	glGenBuffers(1, &m_tbo1);
// 	glBindBuffer(GL_TEXTURE_BUFFER, m_tbo1);
// 	glBufferData(GL_TEXTURE_BUFFER, sizeof(GLfloat) * 16 * 10, nullptr, GL_STATIC_DRAW);
// 	GLuint tex1;
// 	glCreateTextures(GL_TEXTURE_BUFFER, 1, &tex1);
// 	glTextureBuffer(tex1, GL_RGBA32F, m_tbo1);
// 	glBindTextureUnit(0, m_tbo1);
// 	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glBindVertexArray(0);
}

GLuint Mesh::GetVao()
{
	return m_vao;
}

void Mesh::AddIndex(unsigned int index)
{
	m_indices.push_back(index);
}

void Mesh::AddIndexVec(QVector<unsigned int> &indexVec)
{
	m_indices = indexVec;
}

const unsigned int* Mesh::GetIndices() const
{
	return m_indices.data();
}

int Mesh::GetIndicesMemSize() const
{
	return m_indices.size() * sizeof(unsigned int);
}

int Mesh::GetIndicesNum() const
{
	return m_indices.size();
}

void Mesh::BuildLinesIndicesFromTrianglesIndices()
{
	for (int i = 0; i < m_indices.size() - 2; i += 3)
	{
		m_indices_lines.push_back(m_indices[i]);
		m_indices_lines.push_back(m_indices[i + 1]);
		m_indices_lines.push_back(m_indices[i + 1]);
		m_indices_lines.push_back(m_indices[i + 2]);
		m_indices_lines.push_back(m_indices[i + 2]);
		m_indices_lines.push_back(m_indices[i]);
	}
}

int Mesh::GetLinesIndicesNum() const
{
	return m_indices_lines.size();
}

const unsigned int* Mesh::GetLinesIndices() const
{
	return m_indices_lines.data();
}

int Mesh::GetLinesIndicesMemSize() const
{
	return m_indices_lines.size() * sizeof(GLuint);
}

void Mesh::AddBoneId(GLfloat id)
{
	m_boneIds.append(id);
}

const GLfloat* Mesh::GetBoneIds() const
{
	return m_boneIds.data();
}

int Mesh::GetBoneIdsMemSize() const
{
	return m_boneIds.size() * sizeof(GLfloat);
}

int Mesh::GetBoneIdsNum() const
{
	return m_boneIds.size() / NUM_BONES_PER_VEREX;
}

void Mesh::AddBoneWeight(GLfloat weight)
{
	m_boneWeights.append(weight);
}

const GLfloat* Mesh::GetBoneWeights() const
{
	return m_boneWeights.data();
}

int Mesh::GetBoneWeightsMemSize() const
{
	return m_boneWeights.size() * sizeof(GLfloat);
}

int Mesh::GetBoneWeightsNum() const
{
	return m_boneWeights.size() / NUM_BONES_PER_VEREX;
}

void Mesh::SetAnimId(GLuint id)
{
	m_animId = id;
}

GLuint Mesh::GetAnimId() const
{
	return m_animId;
}

GLuint Mesh::GetMultiInstanceModelMatrixOffset() const
{
	return (GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize()
		+ GetBitangentsMemSize() + GetNormalsMemSize());
}

GLuint Mesh::GetInstancesBufferId() const
{
	return m_instanceBufferId;
}

void Mesh::SetDiffuseTexID(GLuint id)
{
	m_diffuseTex1ID = id;
}

void Mesh::AddSpecularTexture(GLuint id)
{
	m_specularTex1ID = id;
}

void Mesh::SetNormalMapTexture(const QString &path)
{
	m_normalmapTexID = TextureMgr::Instance().LoadTexture(path);
}

GLuint Mesh::GetTextureBuffer1() const
{
	return m_tbo1;
}

void Mesh::InitSkybox()
{
	m_skyboxTexID = TextureMgr::Instance().LoadTexture("skybox");
}

void Mesh::InitProjTex()
{
	m_projTexID = TextureMgr::Instance().LoadTexture("./textures/proj.jpg");
}

void Mesh::SetDrawType(eDrawType type)
{
	m_drawType = type;
}

void Mesh::Draw(QMatrix4x4 matVP, QMatrix4x4 matModel, QVector3D camPos, QMatrix4x4 matProj, QMatrix4x4 matView,
	QMatrix4x4 matOrtho)
{
	glBindVertexArray(GetVao());
	CheckError;

	if (0 != m_skyboxTexID) {
		m_shader.SetShaderType(ShaderHelper::SkyboxGBuffer);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexID);
		matModel.translate(camPos);
		matModel.scale(10000);
		glCullFace(GL_FRONT);
		glDepthMask(0);
	}
	else if (0 != m_projTexID) {
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, m_projTexID);
		ShaderHelper::Instance().SetShaderType(ShaderHelper::Decal);
	}
	CheckError;

	if (0 != m_diffuseTex1ID) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_diffuseTex1ID);
	}
	CheckError;

	if (0 != m_normalmapTexID && 0 == m_skyboxTexID) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_normalmapTexID);
	}
	CheckError;

	matVP = matVP * matModel;
	m_shader.SetMVPMatrix(matVP, matModel, matView, matProj);
	m_shader.SetCamWorldPos(camPos);
	m_shader.SetOrthoMat(matOrtho);

	m_shader.SetAmbientSpecularColor(m_mainWnd->GetAmbientColor(), m_mainWnd->GetSpecularColor());
	m_shader.SetTime(GetTickCount());

	// Draw element(with indices)
	if (Triangle == m_drawType) {
		glVertexArrayElementBuffer(GetVao(), m_vaeo);
		glDrawElements(GL_TRIANGLES, GetIndicesNum(), GL_UNSIGNED_INT, 0);
	}
	else if (Point == m_drawType) {
		glDrawArrays(GL_TRIANGLES, 0, GetVerticesNum());
	}
	else if (Line == m_drawType) {
		glVertexArrayElementBuffer(GetVao(), m_vaeo_lines);
		glDrawElements(GL_LINES, GetLinesIndicesNum(), GL_UNSIGNED_INT, 0);
	}
	CheckError;

	if (0 != m_skyboxTexID) {
		glCullFace(GL_BACK);
		glDepthMask(1);
	}

	// Test draw skeleton here
	if (GetAnimId() != 0) {
		AnimationMgr::Instance().DrawSkeleton(GetAnimId());
	}
	CheckError;

	glBindVertexArray(0);
}