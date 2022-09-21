#include "TextureMgr.h"
#include "Texture.h"


int generateBmp(unsigned char* pData, unsigned char pixelWidth, int width, int height, const char* filename)
{
    int size = width * height * pixelWidth;
    BITMAPFILEHEADER bfh;
    bfh.bfType = 0X4d42;
    bfh.bfSize = size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfReserved1 = 0;// reserved  
    bfh.bfReserved2 = 0;// reserved  
    bfh.bfOffBits = bfh.bfSize - size;

    // 位图第二部分，数据信息  
    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = pixelWidth * 8;
    bih.biCompression = 0;
    bih.biSizeImage = size;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    FILE* fp = fopen(filename, "wb");
    if (!fp)
    {
        return S_FALSE;
    }

    size_t s = fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);
    s = fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);
    s = fwrite(pData, 1, size, fp);
    fclose(fp);

    return S_OK;
}

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
	TCHAR tmp[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, tmp);
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

unsigned int TextureMgr::Load2DArrTextures(const QVector<QString>& pathList)
{
	//if (0 == pathList.size()) {
	//	return 1;
	//}

    QVector<QString> pathVec = {
    "./textures/skybox/right.jpg",
    "./textures/skybox/left.jpg",
    "./textures/skybox/top.jpg",
    "./textures/skybox/bottom.jpg",
    "./textures/skybox/front.jpg",
    "./textures/skybox/back.jpg",
    };


    Texture* pTex = new Texture;
    pTex->Load2DArrTextures(pathVec);
	// TODO: change the cache name, for now just a test
    m_texMap.insert("test_2d_arr_tmp_name", pTex);

    return pTex->GetId();
}

unsigned int TextureMgr::Load3DTexture(const QVector<QString>& pathVec)
{
    //if (0 == pathList.size()) {
//	return 1;
//}

    QVector<QString> paths = {
    "./textures/skybox/right.jpg",
    "./textures/skybox/left.jpg",
    "./textures/skybox/top.jpg",
    "./textures/skybox/bottom.jpg",
    "./textures/skybox/front.jpg",
    "./textures/skybox/back.jpg",
    };


    Texture* pTex = new Texture;
    pTex->Load3DTexture(paths);
    // TODO: change the cache name, for now just a test
    m_texMap.insert("test_3d_arr_tmp_name", pTex);

    return pTex->GetId();
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
