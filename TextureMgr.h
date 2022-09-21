#pragma once
#include "QVector"
#include "QMap"
#include "QString"

int generateBmp(unsigned char* pData, unsigned char pixelWidth, int width, int height, const char* filename);

class Texture;
class TextureMgr
{
public:
	~TextureMgr();
	static TextureMgr& Instance() {
		static TextureMgr ins;
		return ins;
	}

	unsigned int LoadTexture(const QString &path);
	unsigned int Load2DArrTextures(const QVector<QString>& pathVec);
	unsigned int Load3DTexture(const QVector<QString>& pathVec);

protected:
	void Init();
	unsigned int LoadCubemapTexture();

private:
	TextureMgr();
	QMap<QString, Texture*>						m_texMap;
};

