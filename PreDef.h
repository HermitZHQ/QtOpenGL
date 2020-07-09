#pragma once
#include "QString"
#include "mainwindow.h"

MainWindow* GetGlobalMainWndPtr();

#define ENABLE_TEX
#define ENABLE_NORMALMAP
#define VERBOSE_DEBUG

#define Q8(str) (QString::fromLocal8Bit(str))
#define AddTipInfo(info) { \
	if (nullptr != GetGlobalMainWndPtr()){ \
		QMetaObject::invokeMethod((QObject*)GetGlobalMainWndPtr(), "AddInfo", Q_ARG(QString, info)); \
	} \
}

#ifdef VERBOSE_DEBUG
	#define CheckError \
	{ \
		auto err = glGetError(); \
		if (0 != err) { \
			QString str; \
			str = QString("[GL Error]-No[%1]-File[%2]-Line[%3]-Func[%4]").arg(err).arg(__FILE__).arg(__LINE__).arg(__FUNCTION__); \
			AddTipInfo(str); \
		} \
	}

	#define CheckErrorMsg(msg) \
	{ \
		auto err = glGetError(); \
		if (0 != err) { \
			QString str; \
			str = QString("[GL Error]-No[%1]-File[%2]-Line[%3]-Func[%4]-Msg[%5]").arg(err).arg(__FILE__).arg(__LINE__).arg(__FUNCTION__).arg(msg); \
			AddTipInfo(str); \
		} \
	}

	#define CheckErrorFatal \
	{ \
		auto err = glGetError(); \
		if (0 != err) { \
			QString str; \
			str = QString("[GL Error]-No[%1]-File[%2]-Line[%3]-Func[%4]").arg(err).arg(__FILE__).arg(__LINE__).arg(__FUNCTION__); \
			AddTipInfo(str); \
			throw(0); \
		} \
	}
#else
	#define CheckError
	#define CheckErrorMsg
#endif