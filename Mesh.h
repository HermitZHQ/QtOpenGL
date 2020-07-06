#pragma once
#include "QVector"
#include "QVector2D"
#include "QVector3D"
#include "QVector4D"
#include "QOpenGLFunctions_4_5_Core"
#include "ShaderHelper.h"

#define MAX_SHADER_NUM 8
#define NUM_BONES_PER_VEREX 8

class Texture;
class MainWindow;
class Mesh : public QOpenGLFunctions_4_5_Core
{
public:
	enum eDrawType
	{
		Triangle,
		Point,
		Line,
	};

	struct VertInfo
	{
		QVector3D			pos;
		QVector4D			color;
		QVector2D			uv1;
		QVector3D			normal;
		QVector4D			tangent;
		QVector4D			bitangent;
		GLuint				boneIds[NUM_BONES_PER_VEREX];
		GLfloat				boneWeights[NUM_BONES_PER_VEREX];
	};
public:
	Mesh();
	~Mesh();

	void AddVertInfo(const VertInfo &info);
	void BindBuffer();
	void BindVertexRelevantBuffer();
	GLuint GetVao();

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

	void BuildLinesIndicesFromTrianglesIndices();
	int GetLinesIndicesNum() const;
	const unsigned int* GetLinesIndices() const;
	int GetLinesIndicesMemSize() const;

	//----bone ids
	void AddBoneId(GLfloat id);
	const GLfloat* GetBoneIds() const;
	int GetBoneIdsMemSize() const;
	int GetBoneIdsNum() const;

	//----bone weights
	void AddBoneWeight(GLfloat weight);
	const GLfloat* GetBoneWeights() const;
	int GetBoneWeightsMemSize() const;
	int GetBoneWeightsNum() const;

	//----anim id
	void SetAnimId(GLuint id);
	GLuint GetAnimId() const;

	//----multi instance
	GLuint GetMultiInstanceModelMatrixOffset() const;
	GLuint GetInstancesBufferId() const;

	//----texture
	void SetDiffuseTexID(GLuint id);
	void AddSpecularTexture(GLuint id);
	void SetNormalMapTexture(const QString &path);
	GLuint GetTextureBuffer1() const;
	void InitSkybox();
	void InitProjTex();

	//----shader & draw
	void SetDrawType(eDrawType type);
	void Draw(QMatrix4x4 matVP, QMatrix4x4 matModel, QVector3D camPos, QMatrix4x4 matProj, QMatrix4x4 matView,
		QMatrix4x4 matOrtho);

private:
	eDrawType							m_drawType;
	QVector<float>						m_vertices;
	QVector<float>						m_colors;
	QVector<float>						m_normals;
	QVector<float>						m_tangents;
	QVector<float>						m_binormals;
	QVector<float>						m_uvs1;
	QVector<float>						m_boneIds;
	QVector<float>						m_boneWeights;
	QVector<VertInfo>					m_vertInfoVec;

	ShaderHelper						&m_shader;
	MainWindow							*m_mainWnd;

	GLuint								m_diffuseTex1ID;
	GLuint								m_specularTex1ID;
	GLuint								m_skyboxTexID;
	GLuint								m_projTexID;
	GLuint								m_normalmapTexID;

	unsigned short						m_faceNum;
	QVector<unsigned int>				m_indices;// for triangles
	QVector<unsigned int>				m_indices_lines;// for lines

	GLuint								m_vao;
	GLuint								m_vbo;
	GLuint								m_vaeo;// vertex array elements object
	GLuint								m_vaeo_lines;// for lines
	GLuint								m_instanceBufferId;
	GLuint								m_tbo1;// texture buffer object

	//----animation
	GLuint								m_animId;
};

