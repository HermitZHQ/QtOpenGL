#pragma once
#include "QVector"
#include "QVector3D"
#include "QVector4D"
#include "qmath.h"

class Model;
class LightMgr
{
public:
	struct LightInfo 
	{
		bool				isEnabled;
		bool				isDirectional;
		bool				isPoint; // if both not directional and point, then it's spot light
		QVector3D			dir;
		QVector3D			pos;
		QVector4D			color;
		float				radius; // for point light
		float				constant; // coefficient
		float				linear;
		float				quadratic;
		float				innerCutoff;
		float				outerCutoff;

		bool				needDynamicUpdate;
		bool				alreadySetFlag;
		Model				*model;

		LightInfo()
			: isEnabled(false), isDirectional(true), isPoint(false)
			, dir(1, 1, 1), pos(0, 0, 0), color(1, 1, 1, 1)
			, radius(10.0f), constant(1.0f), linear(0.045f), quadratic(0.0075f)
			, innerCutoff(qCos(qDegreesToRadians(12.5f))), outerCutoff(qCos(qDegreesToRadians(17.5f)))
			, needDynamicUpdate(false), alreadySetFlag(false), model(nullptr)
		{}

		LightInfo(bool enable, bool directional, bool point, QVector3D _dir, QVector3D _pos, QVector4D _color
			, float r, float _innerCutoff = 7.5f, float _outerCutoff = 12.5f)
			: isEnabled(enable), isDirectional(directional), isPoint(point)
			, dir(_dir), pos(_pos), color(_color)
			, radius(r), constant(1.0f), linear(0.01f), quadratic(0.007f)
			, innerCutoff(qCos(qDegreesToRadians(_innerCutoff))), outerCutoff(qCos(qDegreesToRadians(_outerCutoff)))
			, needDynamicUpdate(false), alreadySetFlag(false), model(nullptr)
		{}

		void EnableDynamicUpdate() {
			needDynamicUpdate = true;
		}
		void DisableDynamicUpdate() {
			needDynamicUpdate = false;
		}
		void SetModel(Model *mod) {
			model = mod;
		}
	};
	~LightMgr();
	static LightMgr& Instance();

	void AddLight(LightInfo info);
// 	int GetMaxLightNum() const;
	int GetCurLightNum() const;
	LightInfo& GetLightInfo(int index);

	const static int				m_maxLightNum = 8;
protected:
	void Init();

private:
	LightMgr();

	QVector<LightInfo>				m_lightInfoVec;
};

