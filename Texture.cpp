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
	//auto f = image.format();

    // create 6000 per row test data
    // height:512(same as brickwall), RGBA
    static unsigned int unpack_size = 6000;
    //unsigned char* testBuf = new unsigned char[h * unpack_size * 4];
    //memset(testBuf, 0, h * unpack_size * 4);
    //for (int i = 0; i < h; ++i) {
    //    for (int j = 0; j < w * 4; j += 4) {
    //        memset(testBuf + (i * unpack_size) + j, 0, 1);
    //        memset(testBuf + (i * unpack_size) + j + 1, 0, 1);
    //        memset(testBuf + (i * unpack_size) + j + 2, 255, 1);
    //        memset(testBuf + (i * unpack_size) + j + 3, 255, 1);
    //	}
    //}

	glGenTextures(1, &m_texId);
	ChkGLErr;
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, m_texId);
	ChkGLErr;

    // test with unpack flag
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    ChkGLErr;
    //glPixelStorei(GL_UNPACK_ROW_LENGTH, 6000);
    ChkGLErr;
    // skip pixels可以搭配row length使用
    // 不搭配也能用，就是指定从开始的像素跳过多少个像素开始拷贝而已
    //glPixelStorei(GL_UNPACK_SKIP_PIXELS, 5488);
    ChkGLErr;
    //glPixelStorei(GL_UNPACK_SKIP_ROWS, 10);
    ChkGLErr;

 	//glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex);
	ChkGLErrMsg(path);

    // restore test unpack params
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    ChkGLErr;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    ChkGLErr;
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    ChkGLErr;
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    ChkGLErr;

    //delete[] testBuf;

    // 自动生成mipmap，没有这句的话，默认只有lv0的原图
	glGenerateMipmap(GL_TEXTURE_2D);
	ChkGLErr;

	// Parameters
    // GL_CLAMP, GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	ChkGLErr;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ChkGLErr;

    // 测试map到texture的内存内容
    if (1)
    {   
        // 暂时没有找到有效的示例，后面有了再补充，现在取出来都是null，但是大体作用肯定是一样的，都是映射一段GPU的内存到本地
        void *texAddr = glMapBufferRange(GL_TEXTURE_BUFFER, 0, 100, GL_MAP_WRITE_BIT);
        auto err = glGetError();
        if (0 != err) {
        }
        else {
            char c = ((char*)texAddr)[0];
        }
    }

    // test get tex data out
    // 经过测试，这个lv的值如果是没有的，那么得到的width或者height是0，切会生成一个glErr
    // 一般来说通过width == 0就足够判断没有该层的mipmap了
    if (0) 
    {
	    GLint texWidth = 0, texHeight = 0, texInternalFmt = 0;
	    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
	    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
	    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &texInternalFmt);
	    if (0 != texWidth && 0 != texHeight)
	    {
	        uchar* tmpBuf = new uchar[texWidth * texHeight * 4];
	        glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, tmpBuf);
	
	        static int i = 0;
	        QString strTmp = "c:/users/zhq/desktop/qt_out_";
	        //generateBmp(tmpBuf, 4, texWidth, texHeight, (QString("c:/users/zhq/desktop/qt_out_%1.bmp").arg(i)).toStdString().c_str());
	        ++i;
	
	        delete[] tmpBuf;
	    }
	    else {
	        AddTipInfo("导出纹理失败，尺寸为0");
	    }
    }

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
        AddTipInfo(Q8("纹理无效[%1]").arg(pathList[0]));
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
				AddTipInfo(Q8("纹理无效[%1]").arg(pathList[i]));
				return -1;
			}
			if (w != width || h != height) {
				AddTipInfo(Q8("纹理无效，不应该使用大小不一致的图片组成array[%1]").arg(pathList[i]));
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
	
	// 开辟存储空间
	// 需要手动mipmap的话，我们应该有对应的图片，而不是一直使用同样的totalBits数据，这是有问题的
	// 我在上面已经加了处理生成缩小图片的函数了，texture3d那边还没加，应该是一样的
	unsigned int lvls = 3;
	// 下面的intelnalFormat必须严格写对，不然就无法正常显示，不能写成GL_RGBA
	// 这块的说明官方文档倒是对的，不过下面还是有2个天坑
	// 这里属于是开辟空间大小和指定格式，并不需要有具体的像素数据，但是下面调用SubImage3D的时候就必须要有数据了，不能为空
	// 否则会崩溃
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, lvls, GL_RGBA8, width, height, 6);
	ChkGLErr; 
	for (unsigned int i = 0; i < lvls; ++i)
	{
		combine_bits_func();
		// 3d中的depth，其实可以理解成几张图片
		// 天坑1：尼玛的，我真是服了，第一个参数不能是GL_TEXTURE_2D_ARRRY，必须是3D，官网的文档开始写的可以
		// 结果后面文档的报错参考里面又说只能是3D......
        //glTexImage3D(GL_TEXTURE_3D, i, GL_RGBA, width, height, texNum, 0, GL_RGBA, GL_UNSIGNED_BYTE, totalBits);
		// 天坑2：官方文档上匹配的写入像素的函数是glTexImage3D，结果我试了很久，根本不行，要用下面的
		// glTexSubImage3D才能正常显示，否则都是黑色的
		// 
		// 最后的像素指针不能为空，否则会崩溃
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, 0, width, height, texNum, GL_BGRA, GL_UNSIGNED_BYTE, totalBits);
        ChkGLErr;
		// prepare for next mipmap size
		width = max(1, (width / 2));
		height = max(1, (height / 2));
	}

	// GL_LINEAR_MIPMAP_LINEAR我测试过了，使用了也不会变黑，是正常的
	// 但是，如果用了mipmap的linear，如何生成的mipmap有问题，那么混合的结果就是错误的了
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
	// 2d array和2d不一样，直接使用GenerateMipmap并没有作用
	//glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	delete[] totalBits;
	delete[] totalBitsTestRed;
    ChkGLErr;

    // test get tex data out
    //GLint texWidth = 0, texHeight = 0, texDepth = 0, texInternalFmt = 0;
    //glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &texWidth);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &texHeight);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_DEPTH, &texDepth);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_INTERNAL_FORMAT, &texInternalFmt);

    //if (0 != texWidth && 0 != texHeight)
    //{
	   // uchar* tmpBuf = new uchar[texWidth * texHeight * 4 * texDepth];
	   // glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_BGRA, GL_UNSIGNED_BYTE, tmpBuf);
	   // static int i = 0;
	   // QString strTmp = "c:/users/zhq/desktop/qt_out_";
	   // generateBmp(tmpBuf, 4, texWidth, texHeight * texDepth, (QString("c:/users/zhq/desktop/qt_out_arr_%1.bmp").arg(i)).toStdString().c_str());
	   // ++i;

    //    delete[] tmpBuf;
    //}

    // reset binding at last
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
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
        AddTipInfo(Q8("纹理无效[%1]").arg(pathList[0]));
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
        for (unsigned int i = 0; i < texNum; ++i)
        {
            QImage image(pathList[i]);
            QImage iamge_scaled = image.scaled(width, height);
            w = iamge_scaled.width();
            h = iamge_scaled.height();

            chunkSize = w * h * 4;
            totalSize = chunkSize * texNum;

            auto tex = iamge_scaled.bits();
            if (nullptr == tex) {
                AddTipInfo(Q8("纹理无效[%1]").arg(pathList[i]));
                return -1;
            }
            if (w != width || h != height) {
                AddTipInfo(Q8("纹理无效，不应该使用大小不一致的图片组成array[%1]").arg(pathList[i]));
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

    // 开辟存储空间
    unsigned int lvls = 9;
    // 下面的intelnalFormat必须严格写对，不然就无法正常显示，不能写成GL_RGBA
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

		// Warning：texture 3d跟2darr最大不同的主要地方在于mipmap的深度值也需要更新
		// 而且每次更新都必须 / 2，就算你不写，opengl也会自动处理，而且会给你报错提示（glError检测的）
		// 所以说3d的mipmap感觉还挺神奇的，先到这里吧，感觉可以了，以后有需要再深挖
        texNum = max(1, (texNum / 2));
    }

    // GL_LINEAR_MIPMAP_LINEAR我测试过了，使用了也不会变黑，是正常的
    // 但是，如果用了mipmap的linear，如何生成的mipmap有问题，那么混合的结果就是错误的了
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    delete[] totalBits;
    delete[] totalBitsTestRed;
    ChkGLErr;


    glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture::LoadSkyboxTexture(QVector<QString> paths)
{
	// 新方法：使用常规方式（我更熟悉的方式）来创建texture，cubemap在Nsight中调试就是正常的
	glGenTextures(1, &m_texId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texId);
	ChkGLErr;

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
		ChkGLErr;
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// 主动生成mipmap对2d和cubemap都有用，只有3d和2darr不行
	// 这里只是暂时关闭了，关闭天空盒应该需要高清，不需要mipmap
	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	ChkGLErr;
}

GLuint Texture::GetId() const
{
	return m_texId;
}