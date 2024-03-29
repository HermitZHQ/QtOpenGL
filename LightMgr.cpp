#include "LightMgr.h"


LightMgr::LightMgr()
{
	Init();
}

LightMgr::~LightMgr()
{

}

LightMgr& LightMgr::Instance()
{
	static LightMgr ins;
	return ins;
}

void LightMgr::AddLight(LightInfo info)
{
	if (m_lightInfoVec.size() >= m_maxLightNum) {
		return;
	}

	m_lightInfoVec.push_back(info);
}

int LightMgr::GetCurLightNum() const
{
	return m_lightInfoVec.size();
}

LightMgr::LightInfo& LightMgr::GetLightInfo(int index)
{
	if (index > m_lightInfoVec.size() - 1) {
		static LightInfo info;
		return info;
	}

	return m_lightInfoVec[index];
}

QVector3D LightMgr::GetDirLightPos() const
{
	return m_lightInfoVec[0].pos;
}

void LightMgr::SetDirLightPos(const QVector3D &pos)
{
    m_lightInfoVec[0].pos = pos;
}

QVector3D LightMgr::GetDirLightDir() const
{
    return m_lightInfoVec[0].dir.normalized();
}

void LightMgr::SetDirLightDir(const QVector3D &dir)
{
    m_lightInfoVec[0].dir = dir.normalized();
}

void LightMgr::Init()
{
	// must set the dir light first, because we will use the first light to be the default dir light
    // 这里其实并不能控制平行光的dir，因为我们把光的方向获取写到camera里面了，需要联动一下
	LightInfo dirLightInfo(true, true, false, 
		QVector3D(1, 1, 0), QVector3D(0, 50, 0), QVector4D(0.2, 0.2, 0.2, 1), 0);
	AddLight(dirLightInfo);

	// 点光源是不需要light dir的，但是为了统一设置，还是需要随便填写一个，但是在shader的计算中是不起作用的
	// PBR需要的光照强度非常高（普通的点光源计算，可能1-3的值就够了，PBR需要100-300，应该跟计算公式有一些关系）
	//LightInfo dirLightInfo2(true, false, true,
	//	QVector3D(-1, 1, 1), QVector3D(10, 20, -20), QVector4D(300, 300, 0, 1), 30);
	//AddLight(dirLightInfo2);
// 	LightInfo dirLightInfo3(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(-10, 20, -20), QVector4D(0, 0, 300, 1), 30);
// 	AddLight(dirLightInfo3);
// 	LightInfo dirLightInfo4(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(-10, 20, 20), QVector4D(0, 300, 0, 1), 30);
// 	AddLight(dirLightInfo4);
// 	LightInfo dirLightInfo5(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(10, 20, 20), QVector4D(300, 0, 0, 1), 30);
// 	AddLight(dirLightInfo5);

// 	LightInfo dirLightInfo6(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(40, 10, -40), QVector4D(1, 1, 0, 1), 40);
// 	AddLight(dirLightInfo6);
// 	LightInfo dirLightInfo7(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(-40, 10, -40), QVector4D(0, 0, 1.5, 1), 40);
// 	AddLight(dirLightInfo7);
// 	LightInfo dirLightInfo8(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(-40, 10, 40), QVector4D(0, 1.5, 0, 1), 40);
// 	AddLight(dirLightInfo8);
// 	LightInfo dirLightInfo9(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(40, 10, 40), QVector4D(1.5, 0, 0, 1), 40);
// 	AddLight(dirLightInfo9);

// 	LightInfo dirLightInfo10(true, false, false,
// 		QVector3D(1, 1, 1), QVector3D(60, 60, 60), QVector4D(1, 1, 1, 1), 120);
// 	AddLight(dirLightInfo10);
}
