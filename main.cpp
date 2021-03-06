#include <QApplication>
#include <QtOpenGL/QGLFormat>
#include "PreDef.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	auto i = format.swapInterval();
	format.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(format);

	MainWindow *pWnd = GetGlobalMainWndPtr();
	pWnd->show();
    return a.exec();
}
