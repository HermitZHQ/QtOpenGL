#include "Texture.h"
#include "QImage"
#include "PreDef.h"

Texture::Texture()
	:m_texId(0)
{
	initializeOpenGLFunctions();
}

Texture::~Texture()
{
}

void Texture::LoadTexture(QString path)
{
	m_path = path;

	int w = 0, h = 0;
	QImage image(path);
	auto tex = image.bits();
	if (nullptr == tex) {
		AddTipInfo(Q8("纹理无效[%1]").arg(path));
		return;
	}
	w = image.width();
	h = image.height();
	auto f = image.format();

	glGenTextures(1, &m_texId);
	CheckError;
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, m_texId);
	CheckError;
// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
	CheckErrorMsg(path);
	glGenerateMipmap(GL_TEXTURE_2D);
	CheckError;

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	CheckError;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CheckError;

	glBindTexture(GL_TEXTURE_2D, 0);
	CheckError;
}

void Texture::LoadSkyboxTexture(QVector<QString> paths)
{
	// 新方法：使用常规方式（我更熟悉的方式）来创建texture，cubemap在Nsight中调试就是正常的
	glGenTextures(1, &m_texId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texId);
	CheckError;

	// 老方法：可以创建指定mipmap层级，但是不知道为什么创建出来的cubemap有问题
	// 还是需要使用上面的新方法才能正常
// 	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_texId);
// 	glTextureStorage2D(m_texId, 10, GL_RGBA32F, 2048, 2048);
// 	CheckError;

// 	glGenTextures(1, &m_texId);
// 	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texId);

// 	glGenTextures(1, &m_texId);
// 	glBindTexture(GL_TEXTURE_2D_ARRAY, m_texId);

	QVector<QImage> imgVec;
	for (int face = 0; face < 6; ++face)
	{
		auto path = paths[face];

		int w = 0, h = 0;
		imgVec.push_back(QImage(path));
		auto tex = imgVec[face].bits();
		if (nullptr == tex) {
			AddTipInfo(Q8("纹理无效[%1]").arg(path));
			return;
		}
		w = imgVec[face].width();
		h = imgVec[face].height();
// 		auto f = imgVec[face].format();
// 		auto d = imgVec[face].depth();


		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
			0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
		CheckError;
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	CheckError;
}

GLuint Texture::GetId() const
{
	return m_texId;
}