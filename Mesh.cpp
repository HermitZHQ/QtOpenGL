#include "Mesh.h"
#include "Texture.h"
#include "ShaderHelper.h"

Mesh::Mesh()
	:m_vao(0), m_vbo(0), m_vaeo(0), m_instanceBufferId(0)
{
	initializeOpenGLFunctions();

	m_normalMapTex = new Texture;
	m_normalMapTex->LoadTexture("./models/brickwall_normal.jpg");
}

Mesh::~Mesh()
{
	for (auto &tex : m_diffuseTexVec)
	{
		delete tex;
	}

	for (auto &tex : m_specularTexVec)
	{
		delete tex;
	}

	if (nullptr != m_normalMapTex) {
		delete m_normalMapTex;
		m_normalMapTex = nullptr;
	}
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
	const GLuint *vertex_indices = GetIndices();

	const GLfloat *vertex_positions = GetVertices();
	const GLfloat *vertex_uvs = GetUvs1();
	const GLfloat *vertex_tangents = GetTangents();
	const GLfloat *vertex_bitangents = GetBitangents();
	const GLfloat *vertex_normals = GetNormals();

	// set element array(index array) buffer
	glGenBuffers(1, &m_vaeo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vaeo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetIndicesMemSize(), vertex_indices, GL_STATIC_DRAW);

	// set vertex array object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glVertexArrayElementBuffer(m_vao, m_vaeo);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize() /*+ 16 * sizeof(GLfloat) * 10*/, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, GetVerticesMemSize(), vertex_positions);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize(), GetUvs1MemSize(), vertex_uvs);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize(), GetTangentsMemSize(), vertex_tangents);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize(),
		GetBitangentsMemSize(), vertex_bitangents);
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize(),
		GetNormalsMemSize(), vertex_normals);

	GLint positionLoc = ShaderHelper::Instance().GetAttriLocation("vPosition");
	GLint uvLoc = ShaderHelper::Instance().GetAttriLocation("vUV");
	GLint normalLoc = ShaderHelper::Instance().GetAttriLocation("vNormal");
	GLint tangentLoc = ShaderHelper::Instance().GetAttriLocation("vTangent");
	GLint bitangentLoc = ShaderHelper::Instance().GetAttriLocation("vBitangent");
	GLint modelMatLoc = ShaderHelper::Instance().GetAttriLocation("model_matrix");

	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, (0));
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize()));
	glEnableVertexAttribArray(uvLoc);
	glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize() + GetUvs1MemSize()));
	glEnableVertexAttribArray(tangentLoc);

	glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize()));
	glEnableVertexAttribArray(bitangentLoc);

	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize() + GetBitangentsMemSize()));
	glEnableVertexAttribArray(normalLoc);

	// multi instances model matrix
	if (-1 != modelMatLoc) {
		// generate the new buffer for instances
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glGenBuffers(1, &m_instanceBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * 10, nullptr, GL_STATIC_DRAW);

// 		m_instanceBufferId = m_vbo;

		// mat4 type take space of 4 vec4, so we should circle 4 times
		for (int i = 0; i < 4; ++i)
		{
			glVertexAttribPointer(modelMatLoc + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16,
				(void*)(/*GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize() +*/ sizeof(GLfloat) * 4 * i));
			glEnableVertexAttribArray(modelMatLoc + i);
			// implement the multi instances
			glVertexAttribDivisor(modelMatLoc + i, 1);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

GLuint Mesh::GetVao()
{
	return m_vao;
}

GLuint Mesh::GetTexture1() const
{
	if (0 == m_diffuseTexVec.size()) {
		return 0;
	}

	return m_diffuseTexVec[0]->GetId();
}

GLuint Mesh::GetTextureNormalMap() const
{
	return m_normalMapTex->GetId();
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

GLuint Mesh::GetMultiInstanceModelMatrixOffset() const
{
	return (GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize()
		+ GetBitangentsMemSize() + GetNormalsMemSize());
}

GLuint Mesh::GetInstancesBufferId() const
{
	return m_instanceBufferId;
}

void Mesh::AddDiffuseTexture(Texture *tex)
{
	m_diffuseTexVec.append(tex);
}

void Mesh::AddSpecularTexture(Texture *tex)
{
	m_specularTexVec.append(tex);
}
