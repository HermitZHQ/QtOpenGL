#include "TextureMgr.h"
#include "Texture.h"

TextureMgr::TextureMgr()
{
	Init();
}

TextureMgr::~TextureMgr()
{
	for (auto &tex : m_texMap)
	{
		delete tex;
	}
}

void TextureMgr::Init()
{
	LoadTexture("./models/brickwall_normal.jpg");
	LoadTexture("./textures/proj.jpg");
	LoadCubemapTexture();
}

unsigned int TextureMgr::LoadTexture(const QString &path)
{
	auto it = m_texMap.find(path);
	if (it != m_texMap.end()) {
		return it.value()->GetId();
	}
	else {
		Texture *pTex = new Texture;
		pTex->LoadTexture(path);
		m_texMap.insert(path, pTex);

		return pTex->GetId();
	}
}

unsigned int TextureMgr::LoadCubemapTexture()
{
	QVector<QString> pathVec = {
		"./textures/skybox/right.jpg",
		"./textures/skybox/left.jpg",
		"./textures/skybox/top.jpg",
		"./textures/skybox/bottom.jpg",
		"./textures/skybox/front.jpg",
		"./textures/skybox/back.jpg",
	};

	// 	QVector<QString> pathVec = {
	// 	"./textures/cloudy/bluecloud_bk.jpg",
	// 	"./textures/cloudy/bluecloud_ft.jpg",
	// 	"./textures/cloudy/bluecloud_up.jpg",
	// 	"./textures/cloudy/bluecloud_dn.jpg",
	// 	"./textures/cloudy/bluecloud_lf.jpg",
	// 	"./textures/cloudy/bluecloud_rt.jpg",
	// 	};

	Texture *pTex = new Texture;
	pTex->LoadSkyboxTexture(pathVec);
	m_texMap.insert("skybox", pTex);

	return pTex->GetId();
}
