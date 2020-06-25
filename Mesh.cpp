#include "Mesh.h"
#include "TextureMgr.h"
#include "ShaderHelper.h"
#include "PreDef.h"
#include "QMatrix4x4"

Mesh::Mesh()
	:m_vao(0), m_vbo(0), m_vaeo(0), m_instanceBufferId(0), m_tbo1(0)
	, m_drawType(Triangle)
	, m_skyboxTexID(0), m_projTexID(0), m_normalmapTexID(0)
	, m_diffuseTex1ID(0), m_shader(ShaderHelper::Instance())
	, m_mainWnd(GetGlobalMainWndPtr())
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
	const GLfloat *vertex_boneIds = GetBoneIds();// id 5
	const GLfloat *vertex_boneWeights = GetBoneWeights();// id 6

	//--------------------------You must gen and bind the VAO first, the after operations all depends on it!!!!!!
	// set vertex array object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// set element array(index array) buffer
	glGenBuffers(1, &m_vaeo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vaeo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetIndicesMemSize(), vertex_indices, GL_STATIC_DRAW);

	glVertexArrayElementBuffer(m_vao, m_vaeo);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize()
		+ GetBoneIdsMemSize() + GetBoneWeightsMemSize(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, GetVerticesMemSize(), vertex_positions);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize(), GetUvs1MemSize(), vertex_uvs);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize(), GetTangentsMemSize(), vertex_tangents);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize(),
		GetBitangentsMemSize(), vertex_bitangents);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize(),
		GetNormalsMemSize(), vertex_normals);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize(),
		GetBoneIdsMemSize(), vertex_boneIds);
	auto err = glGetError();
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize() + GetBoneIdsMemSize(),
		GetBoneWeightsMemSize(), vertex_boneWeights);
	err = glGetError();

	GLint positionLoc = 0;
	GLint uvLoc = 1;
	GLint tangentLoc = 2;
	GLint bitangentLoc = 3;
	GLint normalLoc = 4;
	GLint modelMatLoc = -1;
	GLint boneIdsLoc = 5;
	GLint boneWeightsLoc = 6;

	auto boneIdSize = GetBoneIdsMemSize();
	auto boneNum = GetBoneIdsNum();
	auto boneWeightSize = GetBoneWeightsMemSize();
	auto boneWeightNum = GetBoneWeightsNum();

	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, (0));
	err = glGetError();
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize()));
	err = glGetError();
	glEnableVertexAttribArray(uvLoc);
	glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize() + GetUvs1MemSize()));
	glEnableVertexAttribArray(tangentLoc);

	glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize()));
	err = glGetError();
	glEnableVertexAttribArray(bitangentLoc);

	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize() + GetBitangentsMemSize()));
	err = glGetError();
	glEnableVertexAttribArray(normalLoc);

	glVertexAttribPointer(boneIdsLoc, 4, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize()
			+ GetTangentsMemSize() + GetBitangentsMemSize()
			+ GetNormalsMemSize()));
	err = glGetError();
	glEnableVertexAttribArray(boneIdsLoc);

	glVertexAttribPointer(boneWeightsLoc, 4, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize()
			+ GetTangentsMemSize() + GetBitangentsMemSize()
			+ GetNormalsMemSize() + GetBoneIdsMemSize()));
	err = glGetError();
	glEnableVertexAttribArray(boneWeightsLoc);

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

	if (0 != m_diffuseTex1ID) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_diffuseTex1ID);
	}

	if (0 != m_normalmapTexID && 0 == m_skyboxTexID) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_normalmapTexID);
	}

	matVP = matVP * matModel;
	m_shader.SetMVPMatrix(matVP, matModel, matView, matProj);
	m_shader.SetCamWorldPos(camPos);
	m_shader.SetOrthoMat(matOrtho);

	m_shader.SetAmbientSpecularColor(m_mainWnd->GetAmbientColor(), m_mainWnd->GetSpecularColor());
	m_shader.SetTime(GetTickCount());

	// Draw element(with indices)
	if (Triangle == m_drawType) {
		glDrawElements(GL_TRIANGLES, GetIndicesNum(), GL_UNSIGNED_INT, 0);
	}
	else if (Point == m_drawType) {
		glDrawArrays(GL_TRIANGLES, 0, GetVerticesNum());
	}

	if (0 != m_skyboxTexID) {
		glCullFace(GL_BACK);
		glDepthMask(1);
	}

	glBindVertexArray(0);
}