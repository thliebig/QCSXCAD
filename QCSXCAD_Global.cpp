/*
*	Copyright (C) 2012 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include <cstring>
#include <iostream>
#include <QtCore>
#include "QCSXCAD_Global.h"

using namespace std;

// create global object
QCSXCAD_Global QCSX_Settings;

QCSXCAD_Global::QCSXCAD_Global()
{
	m_EnableEdit = true;
}

void QCSXCAD_Global::ShowArguments(ostream& ostr, string front)
{
	ostr << front << "--disableEdit\tDisable Edit" << endl;
}

bool QCSXCAD_Global::parseCommandLineArgument( QString argv )
{
	if (argv.isEmpty())
		return false;

	if (argv.compare("--disableEdit")==0)
	{
		cout << "QCSXCAD - disabling editing" << endl;
		SetEdit(false);
		return true;
	}
	cerr << "QCSXCAD_Global::parseCommandLineArgument: Warning, unknown argument: " << argv.toStdString() << endl;
	return false;
}

