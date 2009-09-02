#ifndef QCSXCAD_GLOBAL_H_
#define QCSXCAD_GLOBAL_H_

#define _QCSXCAD_LIB_NAME_ "QCSXCAD-Lib: Qt-Gui for Continuous Structure XML - CAD"
#define _QCSXCAD_AUTHOR_ "Thorsten Liebig (2008-2009)"
#define _QCSXCAD_AUTHOR_MAIL_ "Thorsten.Liebig@gmx.de"
#define _QCSXCAD_VERSION_ "0.1.3 alpha"
#define _QCSXCAD_LICENSE_ ""

#if defined(WIN32)
	#ifdef BUILD_QCSXCAD_LIB
	#define QCSXCAD_EXPORT Q_DECL_EXPORT
	#else
	#define QCSXCAD_EXPORT Q_DECL_IMPORT
	#endif
#else
#define QCSXCAD_EXPORT
#endif

#endif /*QCSXCAD_GLOBAL_H_*/
