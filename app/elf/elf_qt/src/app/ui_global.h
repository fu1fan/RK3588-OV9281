#ifndef GUI_GLOBAL_H
#define GUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GUI_LIBRARY)
#  define UI_LIBRARY Q_DECL_EXPORT
#else
#  define UI_LIBRARY Q_DECL_IMPORT
#endif

#endif // PLUGIN_GLOBAL_H
