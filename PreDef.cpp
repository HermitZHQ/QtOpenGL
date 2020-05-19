#include "PreDef.h"

MainWindow* GetGlobalMainWndPtr()
{
	static MainWindow wnd;
	return &wnd;
}
