#include "Texture.h"
#include "SOIL.h"
#include "QImage"

#pragma comment(lib, "SOIL.lib")

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

	int w = 0, h = 0, channels = 0;
	QImage image(path);
	auto tex = image.bits();
	if (nullptr == tex) {
		return;
	}
	w = image.width();
	h = image.height();
	auto f = image.format();


	glGenTextures(1, &m_texId);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, m_texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::GetId() const
{
	return m_texId;
}
