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

void LightMgr::Init()
{
	// must set the dir light first, because we will use the first light to be the default dir light
	LightInfo dirLightInfo(true, true, false, 
		QVector3D(1, 1, 1), QVector3D(150, 150, 150), QVector4D(1, 1, 1, 1), 0);
	AddLight(dirLightInfo);

// 	LightInfo dirLightInfo2(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(20, 20, -20), QVector4D(1, 1, 0, 1), 30);
// 	AddLight(dirLightInfo2);
// 	LightInfo dirLightInfo3(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(-20, 20, -20), QVector4D(0, 0, 1.5, 1), 30);
// 	AddLight(dirLightInfo3);
// 	LightInfo dirLightInfo4(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(-20, 20, 20), QVector4D(0, 1.5, 0, 1), 30);
// 	AddLight(dirLightInfo4);
// 	LightInfo dirLightInfo5(true, false, true,
// 		QVector3D(-1, 1, 1), QVector3D(20, 20, 20), QVector4D(1.5, 0, 0, 1), 30);
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
