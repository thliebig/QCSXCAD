/*
*	Copyright (C) 2010 Sebastian Held (sebastian.held@gmx.de)
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

#ifndef EXPORT_X3D_H
#define EXPORT_X3D_H

#include <QtXml>
#include <QString>

class QCSXCAD;
class CSProperties;

class export_X3D
{
public:
	export_X3D( QCSXCAD * );
	void save( QString filename );

protected:
	void export_properties( QDomElement Scene, std::vector<CSProperties*> properties, QDomElement Material = QDomElement() );
	void export_box( QDomElement &elem, double start[3], double stop[3], QDomElement Material = QDomElement() );
	void export_polygon( QDomElement &elem, size_t count, double *array, double elevation, int normDir, QDomElement Material = QDomElement() );

protected:
	QCSXCAD *m_CSX;
};

#endif // EXPORT_X3D_H
