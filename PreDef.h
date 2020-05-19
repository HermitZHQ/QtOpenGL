#pragma once
#include "QString"
#include "mainwindow.h"

MainWindow* GetGlobalMainWndPtr();

#define ENABLE_TEX
#define ENABLE_NORMALMAP

#define Q8(str) (QString::fromLocal8Bit(str))
#define AddTipInfo(info) { \
	if (nullptr != GetGlobalMainWndPtr()){ \
		QMetaObject::invokeMethod((QObject*)GetGlobalMainWndPtr(), "AddInfo", Q_ARG(QString, info)); \
	} \
}