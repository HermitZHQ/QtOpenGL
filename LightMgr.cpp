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

void LightMgr::Init()
{
	LightInfo dirLightInfo(true, true, false, 
		QVector3D(1, 1, 1), QVector3D(150, 150, 150), QVector4D(1, 1, 1, 1), 0);
	AddLight(dirLightInfo);

	LightInfo dirLightInfo2(true, false, true,
		QVector3D(-1, 1, 1), QVector3D(20, 10, -20), QVector4D(1, 1, 0, 1), 20);
	AddLight(dirLightInfo2);

	LightInfo dirLightInfo3(true, false, true,
		QVector3D(-1, 1, 1), QVector3D(-20, 10, -20), QVector4D(0, 0, 1.5, 1), 20);
	AddLight(dirLightInfo3);

	LightInfo dirLightInfo4(true, false, true,
		QVector3D(-1, 1, 1), QVector3D(-20, 10, 20), QVector4D(0, 1.5, 0, 1), 20);
	AddLight(dirLightInfo4);

	LightInfo dirLightInfo5(true, false, true,
		QVector3D(-1, 1, 1), QVector3D(20, 10, 20), QVector4D(1, 1, 1, 1), 20);
	AddLight(dirLightInfo5);

// 	LightInfo dirLightInfo10(true, false, false,
// 		QVector3D(1, 1, 1), QVector3D(60, 60, 60), QVector4D(1, 1, 1, 1), 120);
// 	AddLight(dirLightInfo10);
}
