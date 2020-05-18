#pragma once
#include "QVector"
#include "QVector2D"
#include "QVector3D"
#include "QVector4D"
#include "QOpenGLFunctions_4_5_Core"

class Texture;
class Mesh : public QOpenGLFunctions_4_5_Core
{
public:
	struct VertInfo
	{
		QVector3D			pos;
		QVector4D			color;
		QVector2D			uv1;
		QVector3D			normal;
		QVector4D			tangent;
		QVector4D			bitangent;
	};
public:
	Mesh();
	~Mesh();

	void AddVertInfo(const VertInfo &info);
	void BindBuffer();
	GLuint GetVao();
	GLuint GetTexture1() const;
	GLuint GetTextureNormalMap() const;

	//----vertex
	void AddVertex(QVector3D vert);
	const float* GetVertices() const;
	int GetVerticesMemSize() const;
	int GetVerticesNum() const;

	//----color
	void AddColor(QVector4D color);
	const float* GetColors() const;
	int GetColorsMemSize() const;

	//----normal
	void AddNormal(QVector3D normal);
	const float* GetNormals() const;
	int GetNormalsMemSize() const;
	int GetNormalsNum() const;

	//----tangent
	void AddTangent(QVector4D tangent);
	const float* GetTangents() const;
	int GetTangentsMemSize() const;
	int GetTangentsNum() const;

	//----binormal
	void AddBitangent(QVector4D binormal);
	const float* GetBitangents() const;
	int GetBitangentsMemSize() const;
	int GetBitangentsNum() const;

	//----UV
	void AddUv1(QVector2D uv);
	const float* GetUvs1() const;
	int GetUvs1MemSize() const;
	int GetUvs1Num() const;

	//---index
	void AddIndex(unsigned int index);
	void AddIndexVec(QVector<unsigned int> &indexVec);
	const unsigned int* GetIndices() const;
	int GetIndicesMemSize() const;
	int GetIndicesNum() const;

	//----multi instance
	GLuint GetMultiInstanceModelMatrixOffset() const;
	GLuint GetInstancesBufferId() const;

	//----texture
	void AddDiffuseTexture(Texture *tex);
	void AddSpecularTexture(Texture *tex);

private:
	QVector<float>						m_vertices;
	QVector<float>						m_colors;
	QVector<float>						m_normals;
	QVector<float>						m_tangents;
	QVector<float>						m_binormals;
	QVector<float>						m_uvs1;
	QVector<VertInfo>					m_vertInfoVec;

	QVector<Texture*>					m_diffuseTexVec;
	QVector<Texture*>					m_specularTexVec;
	Texture								*m_normalMapTex;

	unsigned short						m_faceNum;
	QVector<unsigned int>				m_indices;

	GLuint								m_vao;
	GLuint								m_vbo;
	GLuint								m_vaeo;// vertex array elements object
	GLuint								m_instanceBufferId;
};

