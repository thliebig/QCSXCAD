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

#include <QMessageBox>
#include <QtXml>

#include "QVTKStructure.h"
#if VTK_MAJOR_VERSION>=9
  #include "QVTKOpenGLStereoWidget.h"
#elif VTK_MAJOR_VERSION==8
  #include "QVTKOpenGLWidget.h"
#else
  #include "QVTKWidget.h"
#endif
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <vector>
#include <ContinuousStructure.h>

#include "CSPrimBox.h"
#include "CSPrimCylinder.h"
#include "CSPrimPolygon.h"
#include "CSPrimCurve.h"
#include "CSPrimWire.h"

#include "QCSXCAD.h"
#include "export_pov.h"

export_pov::export_pov( QCSXCAD *CSX ) : m_CSX(CSX)
{
	m_epsilon = 1; // FIXME this should be small compared to any other linear dimension of any object in the scene

}

void export_pov::save( QString filename )
{
	QFile file( filename );
	if (!file.open( QFile::WriteOnly )) {
		QMessageBox::warning( m_CSX, QObject::tr("save failed"), QObject::tr("Cannot save to the file %1").arg(filename), QMessageBox::Ok );
		return;
	}
	QTextStream stream(&file);
	stream.setCodec( "UTF-8" );

	stream << "// povray-file exported by QCSXCAD" << endl;
	stream << "// render with:" << endl;
	stream << "// povray -W640 -H480 +A " << QFileInfo(filename).fileName() << endl;
	stream << endl;
	stream << "#declare TRANSPARENT = off; // if on, also use the \"+ua\" command line flag" << endl;
	stream << endl;
	stream << "#include \"colors.inc\"" << endl;
	stream << "#include \"metals.inc\"" << endl;
	stream << "#include \"textures.inc\"" << endl;
	stream << "#include \"transforms.inc\"" << endl;
	stream << "background { color rgb<1.000000,1.000000,1.000000> #if(TRANSPARENT) transmit 1.0 #end }" << endl;

	// export material
	std::vector<CSProperties*> properties = m_CSX->GetPropertyByType( CSProperties::MATERIAL );
	export_properties( stream, properties, "pigment { color rgbt <0.000, 0.533, 0.800,0.0> } finish { diffuse 0.6 }" );

	// export metal
	properties = m_CSX->GetPropertyByType( CSProperties::METAL );
	export_properties( stream, properties, "texture { Copper_Metal }" );

	// create camera
	stream << get_camera() << endl;

	// create light
	stream << get_light() << endl;

	stream.flush();
	file.close();
}

void export_pov::export_properties( QTextStream &stream, std::vector<CSProperties*> properties, QString default_obj_modifier )
{
	foreach( CSProperties* prop, properties )
	{
		int size = prop->GetQtyPrimitives();
		for( int i=0; i<size; i++ )
		{
			CSPrimitives *prim = prop->GetPrimitive(i);
			if (prim->GetType() == CSPrimitives::BOX)
			{
				CSPrimBox *box = prim->ToBox();
				double start[] = {box->GetCoord(0),box->GetCoord(2),box->GetCoord(4)};
				double stop[]  = {box->GetCoord(1),box->GetCoord(3),box->GetCoord(5)};
				export_box( stream, start, stop, default_obj_modifier );
			}
			else if (prim->GetType() == CSPrimitives::POLYGON)
			{
				CSPrimPolygon *polygon = prim->ToPolygon();
				size_t count = -1;
				double *array = 0;
				array = polygon->GetAllCoords( count, array );
				int normDir = polygon->GetNormDir();
				double elevation = polygon->GetElevation();
				export_polygon( stream, count, array, elevation, normDir, default_obj_modifier );
			}
			else if (prim->GetType() == CSPrimitives::CYLINDER)
			{
				CSPrimCylinder *primitive = prim->ToCylinder();
				double start[] = {primitive->GetCoord(0),primitive->GetCoord(2),primitive->GetCoord(4)};
				double stop[]  = {primitive->GetCoord(1),primitive->GetCoord(3),primitive->GetCoord(5)};
				double radius  = primitive->GetRadius();
				export_cylinder( stream, start, stop, radius, default_obj_modifier );
			}
			else if (prim->GetType() == CSPrimitives::WIRE)
			{
				CSPrimWire *primitive = prim->ToWire();
				double radius = primitive->GetWireRadius();
				size_t count  = primitive->GetNumberOfPoints();
				double *array = new double[count*3];
				for (unsigned int i=0; i<count; i++)
					primitive->GetPoint(i,array+i*3, CARTESIAN);
				export_wire( stream, count, array, radius, default_obj_modifier );
				delete[] array;
			}
			else if (prim->GetType() == CSPrimitives::CURVE)
			{
				CSPrimCurve *primitive = prim->ToCurve();
				double radius = m_epsilon;
				size_t count  = primitive->GetNumberOfPoints();
				double *array = new double[count*3];
				for (unsigned int i=0; i<count; i++)
					primitive->GetPoint(i,array+i*3, CARTESIAN);
				export_wire( stream, count, array, radius, default_obj_modifier );
				delete[] array;
			}
		}
	}
}

void export_pov::export_box( QTextStream &stream, double start[3], double stop[3], QString object_modifier )
{
	for (int i=0; i<3; i++)
		if (start[i] == stop[i]) {
			// 2D box
			// povray supports 2D polygons, but has no priority concept, therefore we use the box primitive
			start[i] -= m_epsilon;
			stop[i]  += m_epsilon;
		}
	QString box = "box { %1, %2 %3 }";
	box = box.arg(pov_vect(start),pov_vect(stop),object_modifier);
	stream << box << endl;
}

void export_pov::export_polygon( QTextStream &stream, size_t count, double *array, double elevation, int normDir, QString object_modifier )
{
	UNUSED(normDir);
	Q_ASSERT((count%2)==0);

	count = count / 2; // now count is the number of 2D-vectors
	if (count < 2)
		return;

	elevation = -elevation;
	//NormDir = [CSX_polygon.NormDir.ATTRIBUTE.X CSX_polygon.NormDir.ATTRIBUTE.Y CSX_polygon.NormDir.ATTRIBUTE.Z];
	QString str = "prism { linear_spline linear_sweep %1, %2, %3";
	str = str.arg(elevation - m_epsilon).arg(elevation + m_epsilon).arg(count+1);
	for (unsigned int a=0; a<count; a++)
	{
		// iterate over all vertices
		str += ", " + pov_vect2( array + a*2 );
	}
	str += ", " + pov_vect2( array ); // close the prism
	str += " " + object_modifier + " rotate<-90,0,0> }";

	stream << str << endl;
}

void export_pov::export_cylinder( QTextStream &stream, double start[3], double stop[3], double radius, QString object_modifier )
{
	QString str = "cylinder { %1, %2, %3 %4 }";
	str = str.arg(pov_vect(start)).arg(pov_vect(stop)).arg(radius).arg(object_modifier);
	stream << str << endl;
}

void export_pov::export_wire( QTextStream &stream, size_t count, double *array, double radius, QString object_modifier )
{
	QString str = "sphere_sweep { linear_spline, " + QString::number(count);
	for (unsigned int a=0; a<count; a++)
	{
		// iterate over all vertices
		str += ", " + pov_vect(array+a*3) + ", " + QString::number(radius);
	}
	str += " " + object_modifier + " }";
	stream << str << endl;
}

QString export_pov::get_camera()
{
#if VTK_MAJOR_VERSION>=9
	vtkRendererCollection* collection = ((QVTKOpenGLStereoWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->renderWindow()->GetRenderers();
#elif VTK_MAJOR_VERSION==8
	vtkRendererCollection* collection = ((QVTKOpenGLWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->GetRenderWindow()->GetRenderers();
#else
	vtkRendererCollection* collection = ((QVTKWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->GetRenderWindow()->GetRenderers();
#endif
	vtkRenderer *r = collection->GetFirstRenderer();
	if (!r)
		return QString();
	vtkCamera *c = r->GetActiveCamera();
	if (!c)
		return QString();

	double *pos = c->GetPosition();
//	cout << "Camera position: " << pos[0] << ", " << pos[1] << ", " << pos[2] << endl;
	double *focalpos = c->GetFocalPoint();
//	cout << "Camera focal point: " << focalpos[0] << ", " << focalpos[1] << ", " << focalpos[2] << endl;
//	double distance = c->GetDistance();
//	cout << "Camera distance (position to focal point): " << distance << endl;
//	double roll = c->GetRoll();
//	cout << "Camera roll angle (about direction of projection): " << roll << "°" << endl;
	double angle = c->GetViewAngle();
//	cout << "Camera view angle: " << angle << "°" << endl;
	double *up = c->GetViewUp();
//	cout << "Camera up vector: " << up[0] << ", " << up[1] << ", " << up[2] << endl;

	QString camera_str;
	camera_str = "camera { perspective location %1 look_at %2 sky %3 right -1.33*x angle %4 }";
	camera_str = camera_str.arg(pov_vect(pos),pov_vect(focalpos),pov_vect(up)).arg(angle);

	return camera_str;
}

QString export_pov::get_light()
{
#if VTK_MAJOR_VERSION>=9
	vtkRendererCollection* collection = ((QVTKOpenGLStereoWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->renderWindow()->GetRenderers();
#elif VTK_MAJOR_VERSION==8
	vtkRendererCollection* collection = ((QVTKOpenGLWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->GetRenderWindow()->GetRenderers();
#else
	vtkRendererCollection* collection = ((QVTKWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->GetRenderWindow()->GetRenderers();
#endif
	vtkRenderer *r = collection->GetFirstRenderer();
	if (!r)
		return QString();
	vtkCamera *c = r->GetActiveCamera();
	if (!c)
		return QString();

	double *pos = c->GetPosition();
//	double *focalpos = c->GetFocalPoint();
//	double distance = c->GetDistance();
//	double roll = c->GetRoll();
//	double angle = c->GetViewAngle();
//	double *up = c->GetViewUp();

	QString light_str;
	light_str = "light_source { %1, rgb <1,1,1> }";
	light_str = light_str.arg(pov_vect(pos));

	return light_str;
}

QString export_pov::pov_vect( double *v )
{
	QString v_str = "<%1,%2,%3>";
	v_str = v_str.arg(v[0]).arg(v[1]).arg(v[2]);
	return v_str;
}
QString export_pov::pov_vect2( double *v )
{
	QString v_str = "<%1,%2>";
	v_str = v_str.arg(v[0]).arg(v[1]);
	return v_str;
}
