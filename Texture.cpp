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
		AddTipInfo(Q8("������Ч[%1]").arg(path));
		return;
	}
	w = image.width();
	h = image.height();
	//auto f = image.format();

	glGenTextures(1, &m_texId);
	ChkGLErr;
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, m_texId);
	ChkGLErr;
// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
	ChkGLErrMsg(path);
	glGenerateMipmap(GL_TEXTURE_2D);
	ChkGLErr;

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	ChkGLErr;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ChkGLErr;

	glBindTexture(GL_TEXTURE_2D, 0);
	ChkGLErr;
}

void Texture::Load2DArrTextures(const QVector<QString>& pathList)
{
	int texNum = pathList.size();
	if (texNum <= 0) {
		return;
	}

	// use the first pic to calc total bits size
    QImage image(pathList[0]);
    auto tex = image.bits();
    if (nullptr == tex) {
        AddTipInfo(Q8("������Ч[%1]").arg(pathList[0]));
        return;
    }
    unsigned int width = image.width();
    unsigned int height = image.height();
	// combine the total bits
	// Warning: we consider the QImage use 32bit(4 bytes) for all images as default(this maybe wrong)
	auto chunkSize = width * height * 4;
	auto totalSize = chunkSize * texNum;
	uchar* totalBits = new uchar[totalSize];
    uchar* totalBitsTestRed = new uchar[totalSize];
    ZeroMemory(totalBitsTestRed, totalSize);
	// format:BGRA
	const unsigned char pixel[4] = { 0, 0, 255, 255 };
	for (unsigned int i = 0; i < width * height * 4 * texNum; i += 4)
	{
		memcpy_s(totalBitsTestRed + i, 4, pixel, 4);
	}
	//memset(totalBitsTestRed, 255, totalSize);

    // here copy all the image's bits to the total bits
	auto combine_bits_func = [&]() {
		int w = 0, h = 0;
		for (int i = 0; i < texNum; ++i)
		{
            QImage image(pathList[i]);
            QImage iamge_scaled = image.scaled(width, height);
            w = iamge_scaled.width();
            h = iamge_scaled.height();

            chunkSize = w * h * 4;
            totalSize = chunkSize * texNum;

			auto tex = iamge_scaled.bits();
			if (nullptr == tex) {
				AddTipInfo(Q8("������Ч[%1]").arg(pathList[i]));
				return -1;
			}
			if (w != width || h != height) {
				AddTipInfo(Q8("������Ч����Ӧ��ʹ�ô�С��һ�µ�ͼƬ���array[%1]").arg(pathList[i]));
				return -1;
			}

			memcpy_s(totalBits + chunkSize * i, chunkSize, tex, chunkSize);
		}

		return 0;
	};

	glGenTextures(1, &m_texId);
	ChkGLErr;
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_texId);
	ChkGLErr;
	
	// ���ٴ洢�ռ�
	// ��Ҫ�ֶ�mipmap�Ļ�������Ӧ���ж�Ӧ��ͼƬ��������һֱʹ��ͬ����totalBits���ݣ������������
	// ���������Ѿ����˴���������СͼƬ�ĺ����ˣ�texture3d�Ǳ߻�û�ӣ�Ӧ����һ����
	unsigned int lvls = 5;
	// �����intelnalFormat�����ϸ�д�ԣ���Ȼ���޷�������ʾ������д��GL_RGBA
	// ����˵���ٷ��ĵ����ǶԵģ��������滹����2�����
	// ���������ǿ��ٿռ��С��ָ����ʽ��������Ҫ�о�����������ݣ������������SubImage3D��ʱ��ͱ���Ҫ�������ˣ�����Ϊ��
	// ��������
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, lvls, GL_RGBA8, width, height, 6);
	ChkGLErr; 
	for (unsigned int i = 0; i < lvls; ++i)
	{
		combine_bits_func();
		// 3d�е�depth����ʵ�������ɼ���ͼƬ
		// ���1������ģ������Ƿ��ˣ���һ������������GL_TEXTURE_2D_ARRRY��������3D���������ĵ���ʼд�Ŀ���
		// ��������ĵ��ı���ο�������˵ֻ����3D......
        //glTexImage3D(GL_TEXTURE_3D, i, GL_RGBA, width, height, texNum, 0, GL_RGBA, GL_UNSIGNED_BYTE, totalBits);
		// ���2���ٷ��ĵ���ƥ���д�����صĺ�����glTexImage3D����������˺ܾã��������У�Ҫ�������
		// glTexSubImage3D����������ʾ�������Ǻ�ɫ��
		// 
		// ��������ָ�벻��Ϊ�գ���������
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, 0, width, height, texNum, GL_BGRA, GL_UNSIGNED_BYTE, totalBits);
        ChkGLErr;
		// prepare for next mipmap size
		width = max(1, (width / 2));
		height = max(1, (height / 2));
	}

	// GL_LINEAR_MIPMAP_LINEAR�Ҳ��Թ��ˣ�ʹ����Ҳ�����ڣ���������
	// ���ǣ��������mipmap��linear��������ɵ�mipmap�����⣬��ô��ϵĽ�����Ǵ������
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	// 2d array��2d��һ����ֱ��ʹ��GenerateMipmap��û������
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	delete[] totalBits;
	delete[] totalBitsTestRed;
    ChkGLErr;
}

void Texture::Load3DTexture(const QVector<QString>& pathList)
{
    unsigned int texNum = pathList.size();
    if (texNum <= 0) {
        return;
    }

    // use the first pic to calc total bits size
    QImage image(pathList[0]);
    auto tex = image.bits();
    if (nullptr == tex) {
        AddTipInfo(Q8("������Ч[%1]").arg(pathList[0]));
        return;
    }
    unsigned int width = image.width();
    unsigned int height = image.height();
    // combine the total bits
    // Warning: we consider the QImage use 32bit(4 bytes) for all images as default(this maybe wrong)
    auto chunkSize = width * height * 4;
    auto totalSize = chunkSize * texNum;
    uchar* totalBits = new uchar[totalSize];
    uchar* totalBitsTestRed = new uchar[totalSize];
    ZeroMemory(totalBitsTestRed, totalSize);
    // format:bgra
    const unsigned char pixel[4] = { 0, 0, 255, 255 };
    for (unsigned int i = 0; i < width * height * 4 * texNum; i += 4)
    {
        memcpy_s(totalBitsTestRed + i, 4, pixel, 4);
    }
    //memset(totalBitsTestRed, 255, totalSize);

    // here copy all the image's bits to the total bits
    auto combine_bits_func = [&]() {
        int w = 0, h = 0;
        for (int i = 0; i < texNum; ++i)
        {
            QImage image(pathList[i]);
            QImage iamge_scaled = image.scaled(width, height);
            w = iamge_scaled.width();
            h = iamge_scaled.height();

            chunkSize = w * h * 4;
            totalSize = chunkSize * texNum;

            auto tex = iamge_scaled.bits();
            if (nullptr == tex) {
                AddTipInfo(Q8("������Ч[%1]").arg(pathList[i]));
                return -1;
            }
            if (w != width || h != height) {
                AddTipInfo(Q8("������Ч����Ӧ��ʹ�ô�С��һ�µ�ͼƬ���array[%1]").arg(pathList[i]));
                return -1;
            }

            memcpy_s(totalBits + chunkSize * i, chunkSize, tex, chunkSize);
        }

        return 0;
    };

    glGenTextures(1, &m_texId);
    ChkGLErr;
    glBindTexture(GL_TEXTURE_3D, m_texId);
    ChkGLErr;

    // ���ٴ洢�ռ�
    unsigned int lvls = 9;
    // �����intelnalFormat�����ϸ�д�ԣ���Ȼ���޷�������ʾ������д��GL_RGBA
    glTexStorage3D(GL_TEXTURE_3D, lvls, GL_RGBA8, width, height, 6);
    ChkGLErr;
    for (unsigned int i = 0; i < lvls; ++i)
    {
		combine_bits_func();
        glTexSubImage3D(GL_TEXTURE_3D, i, 0, 0, 0, width, height, texNum, GL_BGRA, GL_UNSIGNED_BYTE, totalBits);
        ChkGLErr;
        // prepare for next mipmap size
        width = max(1, (width / 2));
        height = max(1, (height / 2));

		// Warning��texture 3d��2darr���ͬ����Ҫ�ط�����mipmap�����ֵҲ��Ҫ����
		// ����ÿ�θ��¶����� / 2�������㲻д��openglҲ���Զ��������һ���㱨����ʾ��glError���ģ�
		// ����˵3d��mipmap�о���ͦ����ģ��ȵ�����ɣ��о������ˣ��Ժ�����Ҫ������
        texNum = max(1, (texNum / 2));
    }

    // GL_LINEAR_MIPMAP_LINEAR�Ҳ��Թ��ˣ�ʹ����Ҳ�����ڣ���������
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_3D, 0);
    delete[] totalBits;
    delete[] totalBitsTestRed;
    ChkGLErr;
}

void Texture::LoadSkyboxTexture(QVector<QString> paths)
{
	// �·�����ʹ�ó��淽ʽ���Ҹ���Ϥ�ķ�ʽ��������texture��cubemap��Nsight�е��Ծ���������
	glGenTextures(1, &m_texId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texId);
	ChkGLErr;

	// �Ϸ��������Դ���ָ��mipmap�㼶�����ǲ�֪��Ϊʲô����������cubemap������
	// ������Ҫʹ��������·�����������
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
			AddTipInfo(Q8("������Ч[%1]").arg(path));
			return;
		}
		w = imgVec[face].width();
		h = imgVec[face].height();
// 		auto f = imgVec[face].format();
// 		auto d = imgVec[face].depth();


		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
			0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
		ChkGLErr;
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// ��������mipmap��2d��cubemap�����ã�ֻ��3d��2darr����
	// ����ֻ����ʱ�ر��ˣ��ر���պ�Ӧ����Ҫ���壬����Ҫmipmap
	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	ChkGLErr;
}

GLuint Texture::GetId() const
{
	return m_texId;
}