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

#ifndef EXPORT_POV_H
#define EXPORT_POV_H

#include <QtXml>
#include <QString>

class QCSXCAD;
class CSProperties;

class export_pov
{
public:
	export_pov( QCSXCAD * );
	void save( QString filename );

protected:
	void export_properties( QTextStream &stream, std::vector<CSProperties*> properties, QString default_obj_modifier = QString() );
	void export_box( QTextStream &stream, double start[3], double stop[3], QString obj_modifier = QString() );
	void export_polygon( QTextStream &stream, size_t count, double *array, double elevation, int normDir, QString obj_modifier = QString() );
	void export_cylinder( QTextStream &stream, double start[3], double stop[3], double radius, QString object_modifier = QString() );
	void export_wire( QTextStream &stream, size_t count, double *array, double radius, QString object_modifier );
	QString get_camera();
	QString get_light();
	QString pov_vect( double *v );
	QString pov_vect2( double *v );

protected:
	QCSXCAD *m_CSX;
	double m_epsilon; //!< this should be very small compared to every object inside m_CSX; used to make 2D boxes -> 3D
};

#endif // EXPORT_POV_H
