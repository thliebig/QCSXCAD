/*
*	Copyright (C) 2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QCSXCAD_GLOBAL_H_
#define QCSXCAD_GLOBAL_H_

#define _QCSXCAD_LIB_NAME_ "QCSXCAD-Lib: Qt-Gui for Continuous Structure XML - CAD"
#define _QCSXCAD_AUTHOR_ "Thorsten Liebig (2008-2010)"
#define _QCSXCAD_AUTHOR_MAIL_ "Thorsten.Liebig@gmx.de"
#define _QCSXCAD_VERSION_ GIT_VERSION
#define _QCSXCAD_LICENSE_ "LGPL v3"

#if defined(WIN32)
	#ifdef BUILD_QCSXCAD_LIB
	#define QCSXCAD_EXPORT Q_DECL_EXPORT
	#else
	#define QCSXCAD_EXPORT Q_DECL_IMPORT
	#endif
#else
#define QCSXCAD_EXPORT
#endif

class QCSXCAD_EXPORT QCSXCAD_Global
{
public:
	QCSXCAD_Global();
	bool parseCommandLineArgument( QString argv );

	void ShowArguments(std::ostream& ostr, std::string front=std::string());

	void SetEdit(bool val) {m_EnableEdit=val;}
	bool GetEdit() {return m_EnableEdit;}

	void SetRenderDiscMaterial(bool val) {m_EnableEdit=val;}
	bool GetRenderDiscMaterial() {return m_EnableEdit;}

protected:
	bool m_EnableEdit;
	bool m_RenderDiscMat;
};

extern QCSXCAD_EXPORT QCSXCAD_Global QCSX_Settings;

#endif /*QCSXCAD_GLOBAL_H_*/
