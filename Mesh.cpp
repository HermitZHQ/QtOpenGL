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
	// just for crash test
	//{
	//	// reduce the indices vector to half
	//	auto size = m_indices.size();
	//	for (int i = 0; i < size / 2; ++i) {
	//		m_indices.pop_back();
	//	}
	//}
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
	ChkGLErr;
	glBindVertexArray(m_vao);
	ChkGLErr;

	// set element array(index array) buffer
	glGenBuffers(1, &m_vaeo);
	ChkGLErr;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vaeo);
	ChkGLErr;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetIndicesMemSize(), vertex_indices, GL_STATIC_DRAW);
	ChkGLErr;
	glVertexArrayElementBuffer(m_vao, m_vaeo);
	ChkGLErr;

	// also create the lines indices at the same time, so we can switch them dynamicly
	glGenBuffers(1, &m_vaeo_lines);
	ChkGLErr;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vaeo_lines);
	ChkGLErr;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetLinesIndicesMemSize(), GetLinesIndices(), GL_STATIC_DRAW);
	ChkGLErr;

	glGenBuffers(1, &m_vbo);
	ChkGLErr;
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	ChkGLErr;
	glBufferData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize() + GetNormalsMemSize()
		/*+ GetBoneIdsMemSize() + GetBoneWeightsMemSize()*/, NULL, GL_STATIC_DRAW);
	ChkGLErr;
	glBufferSubData(GL_ARRAY_BUFFER, 0, GetVerticesMemSize(), vertex_positions);
	ChkGLErr;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize(), GetUvs1MemSize(), vertex_uvs);
	ChkGLErr;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize(), GetTangentsMemSize(), vertex_tangents);
	ChkGLErr;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize(),
		GetBitangentsMemSize(), vertex_bitangents);
	ChkGLErr;
	glBufferSubData(GL_ARRAY_BUFFER, GetVerticesMemSize() + GetUvs1MemSize()
		+ GetTangentsMemSize() + GetBitangentsMemSize(),
		GetNormalsMemSize(), vertex_normals);
	ChkGLErr;
	
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
	ChkGLErr;
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize()));
	ChkGLErr;
	glEnableVertexAttribArray(uvLoc);
	ChkGLErr;
	glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(GetVerticesMemSize() + GetUvs1MemSize()));
	ChkGLErr;
	glEnableVertexAttribArray(tangentLoc);
	ChkGLErr;

	glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize()));
	ChkGLErr;
	glEnableVertexAttribArray(bitangentLoc);

	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)(GetVerticesMemSize() + GetUvs1MemSize() + GetTangentsMemSize() + GetBitangentsMemSize()));
	ChkGLErr;
	glEnableVertexAttribArray(normalLoc);

	// 不能使用glEnableVertexAttribArray来设置GL_INT类型，shader中读取的数据不正常，但是官方又说可以使用两种来设置glint类型
	// 但是有说-I的类型是专门设置GL_INT或者uint的，反正感觉说的很模糊，只能当是经验积累学习了
	//----Bone Ids(8)
	GLuint boneIdBuffer = 0;
	glGenBuffers(1, &boneIdBuffer);
	ChkGLErr;
	glBindBuffer(GL_ARRAY_BUFFER, boneIdBuffer);
	ChkGLErr;
	glBufferData(GL_ARRAY_BUFFER, GetBoneIdsMemSize(), vertex_boneIds, GL_STATIC_DRAW);
	ChkGLErr;
	glVertexAttribPointer(boneIdsLoc, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(0));
	ChkGLErr;
	glEnableVertexAttribArray(boneIdsLoc);

	glVertexAttribPointer(boneIdsLoc + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(sizeof(GL_FLOAT) * 4));
	ChkGLErr;
	glEnableVertexAttribArray(boneIdsLoc + 1);

	//----Bone Weights(8)
	GLuint boneWeightBuffer = 0;
	glGenBuffers(1, &boneWeightBuffer);
	ChkGLErr;
	glBindBuffer(GL_ARRAY_BUFFER, boneWeightBuffer);
	ChkGLErr;
	glBufferData(GL_ARRAY_BUFFER, GetBoneWeightsMemSize(), vertex_boneWeights, GL_STATIC_DRAW);
	ChkGLErr;
	glVertexAttribPointer(boneWeightsLoc, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8,
		(void*)(0));
	ChkGLErr;
	glEnableVertexAttribArray(boneWeightsLoc);
	ChkGLErr;

	glVertexAttribPointer(boneWeightsLoc + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8,
		(void*)(sizeof(GL_FLOAT) * 4));
	ChkGLErr;
	glEnableVertexAttribArray(boneWeightsLoc + 1);
	ChkGLErr;

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

void Mesh::SetName(const QString &name)
{
	m_name = name;
}

QString Mesh::GetName() const
{
	return m_name;
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

void Mesh::SetDiffuseTexture(const QString &path)
{
	m_diffuseTex1ID = TextureMgr::Instance().LoadTexture(path);
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
	ChkGLErr;

	if (0 != m_skyboxTexID) {
		m_shader.SetShaderType(ShaderHelper::SkyboxGBuffer);
// 		m_shader.SetShaderType(ShaderHelper::Skybox);
		ChkGLErr;
		// should learn more, I don't know why here....
		// find some clue from Nsigth tool, other texture index, such as 0(gl_texture0), already bind to gl_texture_2d
		// so, you can't rebind it to another format tex
		glActiveTexture(GL_TEXTURE18);
		ChkGLErr;
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexID);
		ChkGLErr;
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
	ChkGLErr;

	if (0 != m_diffuseTex1ID) {
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_diffuseTex1ID);

        GLint ver = 0, ver2 = 0;
        glGetIntegerv(GL_MINOR_VERSION, &ver);
        glGetIntegerv(GL_MAJOR_VERSION, &ver2);

        // 下面的测试代码，是用来验证当时bs5中的diablo freeze的问题，GL_COMPARE_REF_TO_TEXTURE对应的texture如果不是depth的话，则会有报错输出，且在Intel上会freeze（可能是某些复合情况导致的）
        if (0)
        {
            static GLuint samp = 0;
            static GLuint texArrId = 0;
            if (0 == samp) {
                glGenSamplers(1, &samp);
                glSamplerParameteri(samp, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glSamplerParameteri(samp, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glSamplerParameteri(samp, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glSamplerParameteri(samp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(samp, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                glSamplerParameteri(samp, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

                QVector<QString> pathVec = {
                    "",
                    ""
                };
                texArrId = TextureMgr::Instance().Load2DArrTextures(pathVec);
            }
            glBindTexture(GL_TEXTURE_2D_ARRAY, texArrId);

            glBindSampler(0, samp);

            GLint sampler = 0;
            glGetIntegerv(GL_SAMPLER_BINDING, &sampler);
            GLint cmp_mode = 0;
            glGetSamplerParameteriv(sampler, GL_TEXTURE_COMPARE_MODE, &cmp_mode);

            GLint fmt = 0;
            glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
            if (0 != fmt) {
                int i = 0;
                ++i;
            }
        }
	}
	ChkGLErr;

	if (0 != m_normalmapTexID && 0 == m_skyboxTexID) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_normalmapTexID);
		m_shader.SetHasNormalMap(true);
	}
	else {
		m_shader.SetHasNormalMap(false);
	}
	ChkGLErr;

	matVP = matVP * matModel;
	m_shader.SetMVPMatrix(matVP, matModel, matView, matProj);
	m_shader.SetCamWorldPos(camPos);
	m_shader.SetOrthoMat(matOrtho);
	ChkGLErr;

	m_shader.SetAmbientSpecularColor(m_mainWnd->GetAmbientColor(), m_mainWnd->GetSpecularColor());
	m_shader.SetTime(GetTickCount());
	ChkGLErr;

    // test glInvalidateFramebuffer
    // 下面的测试会造成花屏，和之前遇到的一个bst里的bug一样，应该是对函数的错误调用
    //GLenum arr[1] = { GL_COLOR_ATTACHMENT0 };
    //glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, arr);

	// Draw element(with indices)
	if (Triangle == m_drawType) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vaeo);// 方法1，单独绑定elements
		//glVertexArrayElementBuffer(GetVao(), m_vaeo);// 方法2，连同vao一起绑定，但其实不需要，函数第一行我们就绑定了vao了
		ChkGLErr;
		// test crash with indices ptr
		const char* strTest = "34324344";// this can cause crash, if we set it to the 4th param of glDrawElements
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        GLint iTmp = 0;
        //glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &iTmp);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &iTmp);
        //void *pAddr1 = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        //void *pAddr2 = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, 1, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        //auto err111 = glGetError();

        unsigned int test[] = { 0, 1, 2, 2, 3, 4 };
        // this will cause crash, must set valid vao, before wo draw??
        //glBindVertexArray(0);
        glDrawElements(GL_TRIANGLES, GetIndicesNum(), GL_UNSIGNED_INT, (const void*)0);
        // this code can cause the 4101 error.....
        //auto ptr = m_indices.data();
        //glDrawElements(GL_TRIANGLES, GetIndicesNum(), GL_UNSIGNED_INT, m_indices.data());
		ChkGLErr;
	}
	else if (Point == m_drawType) {
		glDrawArrays(GL_TRIANGLES, 0, GetVerticesNum());
		ChkGLErr;
	}
	else if (Line == m_drawType) {
		glVertexArrayElementBuffer(GetVao(), m_vaeo_lines);
		ChkGLErr;
		glDrawElements(GL_LINES, GetLinesIndicesNum(), GL_UNSIGNED_INT, 0);
		ChkGLErr;
	}

	if (0 != m_skyboxTexID) {
		glCullFace(GL_BACK);
		glDepthMask(1);
	}
	ChkGLErr;

	// Test draw skeleton here
	if (GetAnimId() != 0) {
		AnimationMgr::Instance().DrawSkeleton(GetAnimId());
	}
	ChkGLErr;

    //glBindTexture(GL_TEXTURE_2D, 0);
    //glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindSampler(0, 0);
	glBindVertexArray(0);
}