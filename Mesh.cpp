#include "Mesh.h"
#include "Texture.h"
#include "ShaderHelper.h"
#include "PreDef.h"
#include "QMatrix4x4"

Mesh::Mesh()
	:m_vao(0), m_vbo(0), m_vaeo(0), m_instanceBufferId(0), m_tbo1(0)
	, m_shaderType(Default)
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
	// init the shaders first
	InitShaders();
}

void Mesh::BindVertexRelevantBuffer()
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

	GLint positionLoc = 0;
	GLint uvLoc = 1;
	GLint tangentLoc = 2;
	GLint bitangentLoc = 3;
	GLint normalLoc = 4;
	GLint modelMatLoc = -1;

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

	// sampler buffer bind
	glGenBuffers(1, &m_tbo1);
	glBindBuffer(GL_TEXTURE_BUFFER, m_tbo1);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(GLfloat) * 16 * 10, nullptr, GL_STATIC_DRAW);
	GLuint tex1;
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &tex1);
	glTextureBuffer(tex1, GL_RGBA32F, m_tbo1);
	glBindTextureUnit(0, m_tbo1);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
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

GLuint Mesh::GetTextureBuffer1() const
{
	return m_tbo1;
}

void Mesh::Draw(QMatrix4x4 &matVP, QMatrix4x4 &matModel, QVector3D &camPos)
{
#ifdef ENABLE_TEX
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GetTexture1());
#ifdef ENABLE_NORMALMAP
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, GetTextureNormalMap());
#endif
#endif
	glBindVertexArray(GetVao());

	matVP = matVP * matModel;
	SetMVPMatrix(matVP);
	SetWorldMatrix(matModel);
	SetCamWorldPos(camPos);

	// Draw element(with indices)
// 	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glDrawElements(GL_TRIANGLES, GetIndicesNum(), GL_UNSIGNED_INT, 0);
}

void Mesh::InitShaders()
{
	InitDefaultShader();
	InitShader1();
	BindVertexRelevantBuffer();

	SwitchShader(Shader1);
}

void Mesh::InitDefaultShader()
{
	ShaderHelper::ShaderInfo info[] = {
	{GL_VERTEX_SHADER, "./shaders/triangle.vert"},
	{GL_FRAGMENT_SHADER, "./shaders/triangle.frag"}
	};
	GLuint program = m_shaders[Default].LoadShaders(info, sizeof(info) / sizeof(ShaderHelper::ShaderInfo));
	if (program != 0) {
		AddTipInfo(Q8("读取shader文件成功----"));
	}
	else {
		AddTipInfo(Q8("读取shader文件失败----"));
	}
	m_shaders[Default].Use();

	m_matMVPLoc[Default] = m_shaders[Default].GetUniformLocation("mat_mvp");
	m_matWorldLoc[Default] = m_shaders[Default].GetUniformLocation("mat_world");
	m_worldCamPosLoc[Default] = m_shaders[Default].GetUniformLocation("worldCamPos");

	auto texId = m_shaders[Default].GetUniformLocation("tex");
	auto normalMapId = m_shaders[Default].GetUniformLocation("normalMap");
	glUniform1i(texId, 0);
	glUniform1i(normalMapId, 1);

	if (-1 == m_matMVPLoc[Default] || -1 == m_matWorldLoc[Default]
		|| -1 == m_worldCamPosLoc[Default]) {
		AddTipInfo(Q8("查询uniform失败！"));
	}

	m_shaders[Default].Unuse();
}

void Mesh::InitShader1()
{
	ShaderHelper::ShaderInfo info2[] = {
		{GL_VERTEX_SHADER, "./shaders/program1.vert"},
		{GL_FRAGMENT_SHADER, "./shaders/program1.frag"}
	};
	GLuint program = m_shaders[Shader1].LoadShaders(info2, sizeof(info2) / sizeof(ShaderHelper::ShaderInfo));
	if (program != 0) {
		AddTipInfo(Q8("读取shader1文件成功----"));
	}
	else {
		AddTipInfo(Q8("读取shader1文件失败----"));
	}
	m_shaders[Shader1].Use();

	m_matMVPLoc[Shader1] = m_shaders[Shader1].GetUniformLocation("mat_mvp");
	m_matWorldLoc[Shader1] = m_shaders[Shader1].GetUniformLocation("mat_world");
	m_worldCamPosLoc[Shader1] = m_shaders[Shader1].GetUniformLocation("worldCamPos");

	auto texId = m_shaders[Shader1].GetUniformLocation("tex");
	auto normalMapId = m_shaders[Shader1].GetUniformLocation("normalMap");
	glUniform1i(texId, 0);
	glUniform1i(normalMapId, 1);

	if (-1 == m_matMVPLoc[Shader1] || -1 == m_matWorldLoc[Shader1]
		|| -1 == m_worldCamPosLoc[Shader1]) {
		AddTipInfo(Q8("查询uniform1失败！"));
	}

	m_shaders[Shader1].Unuse();
}

void Mesh::SwitchShader(eShaderType type)
{
	m_shaderType = type;
	m_shaders[m_shaderType].Use();
}

void Mesh::SetMVPMatrix(QMatrix4x4 &matMVP)
{
	glUniformMatrix4fv(m_matMVPLoc[m_shaderType], 1, GL_FALSE, matMVP.data());
}

void Mesh::SetWorldMatrix(QMatrix4x4 &matWorld)
{
	glUniformMatrix4fv(m_matWorldLoc[m_shaderType], 1, GL_FALSE, matWorld.data());
}

void Mesh::SetCamWorldPos(QVector3D &camPos)
{
	glUniform3f(m_worldCamPosLoc[m_shaderType], camPos.x(), camPos.y(), camPos.z());
}

GLuint Mesh::GetDefaultProgram() const
{
	return m_shaders[Default].GetProgram();
}

GLuint Mesh::GetProgram1() const
{
	return m_shaders[Shader1].GetProgram();
}