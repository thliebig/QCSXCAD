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

#include <QtXml>
#include <vector>
#include <ContinuousStructure.h>
#include "CSPrimBox.h"
#include "CSPrimPolygon.h"

#include "QCSXCAD.h"
#include "export_x3d.h"

export_X3D::export_X3D( QCSXCAD *CSX ) : m_CSX(CSX)
{
}

void export_X3D::save( QString filename )
{
	QDomImplementation impl;
	QDomDocument doc( impl.createDocumentType ( "X3D", "ISO//Web3D//DTD X3D 3.2//EN", "http://www.web3d.org/specifications/x3d-3.2.dtd" ) );
	doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" )); // FIXME this is not a processing instruction!
	QDomElement X3D = doc.createElement( "X3D" );
	X3D.setAttribute( "version", "3.2" );
	X3D.setAttribute( "profile", "Immersive" );
	doc.appendChild( X3D );
	QDomElement head = doc.createElement( "head" );
	X3D.appendChild( head );
	QDomElement meta = doc.createElement( "meta" );
	meta.setAttribute( "name", "description" );
	meta.setAttribute( "content", "X3D model exported by QCSXCAD" );
	head.appendChild( meta );
	QDomElement Scene = doc.createElement( "Scene" );
	X3D.appendChild( Scene );

	QDomElement NavigationInfo = doc.createElement( "NavigationInfo" );
	NavigationInfo.setAttribute( "headlight", "true" );
//	NavigationInfo.setAttribute( "avatarSize", "0.25 1.6 0.75" );
	NavigationInfo.setAttribute( "type", "EXAMINE" );
	Scene.appendChild( NavigationInfo );

	std::vector<CSProperties*> properties = m_CSX->GetPropertyByType( CSProperties::MATERIAL );
	export_properties( Scene, properties );

	properties = m_CSX->GetPropertyByType( CSProperties::METAL );
	QDomElement Material = doc.createElement( "Material" );
	Material.setAttribute( "diffuseColor", "0.0 1.0 0.0" );
	export_properties( Scene, properties, Material );

	// create camera
#if VTK_MAJOR_VERSION>=9
	vtkRendererCollection* collection = ((QVTKOpenGLStereoWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->renderWindow()->GetRenderers();
#elif VTK_MAJOR_VERSION==8
	vtkRendererCollection* collection = ((QVTKOpenGLWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->GetRenderWindow()->GetRenderers();
#else
	vtkRendererCollection* collection = ((QVTKWidget*)(m_CSX->StructureVTK->GetVTKWidget()))->GetRenderWindow()->GetRenderers();
#endif
	vtkRenderer *r = collection->GetFirstRenderer();
	if (!r)
		return;
	vtkCamera *c = r->GetActiveCamera();
	if (!c)
		return;

	QDomElement Viewpoint = doc.createElement( "Viewpoint" );
	double *pos = c->GetPosition();
	Viewpoint.setAttribute( "position", QString::number(pos[0]) + " " + QString::number(pos[1]) + " " + QString::number(pos[2]) );
//	double *focalpos = c->GetFocalPoint();
//	Viewpoint.setAttribute( "centerOfRotation", QString::number(focalpos[0]) + " " + QString::number(focalpos[1]) + " " + QString::number(focalpos[2]) );
	double angle = c->GetViewAngle();
	Viewpoint.setAttribute( "fieldOfView", QString::number(angle/180.0*M_PI) );
	double *WXYZ = c->GetOrientationWXYZ();
	Viewpoint.setAttribute( "orientation", QString::number(WXYZ[1]) + " " + QString::number(WXYZ[2]) + " " + QString::number(WXYZ[3]) + " " + QString::number((360.0-WXYZ[0])/180.0*M_PI) ); // WXYZ[0] is wrong VTK-BUG!?
	Scene.appendChild( Viewpoint );

//	cout << doc.toString(4).toStdString() << endl;

	QFile file( filename );
	if (!file.open( QFile::WriteOnly )) {
		QMessageBox::warning( m_CSX, QObject::tr("save failed"), QObject::tr("Cannot save to the file %1").arg(filename), QMessageBox::Ok );
		return;
	}
	QTextStream stream(&file);
	stream.setCodec( "UTF-8" );
	doc.save( stream, 4, QDomNode::EncodingFromTextStream );
}

void export_X3D::export_properties( QDomElement Scene, std::vector<CSProperties*> properties, QDomElement Material )
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
				export_box( Scene, start, stop, Material.cloneNode().toElement() );
			}
			else if (prim->GetType() == CSPrimitives::POLYGON)
			{
				CSPrimPolygon *polygon = prim->ToPolygon();
				size_t count = -1;
				double *array = 0;
				array = polygon->GetAllCoords( count, array );
				int normDir = polygon->GetNormDir();
				double elevation = polygon->GetElevation();
				export_polygon( Scene, count, array, elevation, normDir, Material.cloneNode().toElement() );
			}
		}
	}
}

void export_X3D::export_box( QDomElement &elem, double start[3], double stop[3], QDomElement Material )
{
	double center[3], size[3];
	for (int i=0; i<3; i++)
	{
		center[i] = (stop[i]+start[i])/2.0;// * GetGrid()->GetDeltaUnit();
		size[i]   = fabs(stop[i]-start[i]);// * GetGrid()->GetDeltaUnit();
	}

	QDomDocument doc = elem.ownerDocument();
	QDomElement Transform = doc.createElement( "Transform" );
	Transform.setAttribute( "translation", QString::number(center[0]) + " " + QString::number(center[1]) + " " + QString::number(center[2]) );
	QDomElement Shape = doc.createElement( "Shape" );
	Transform.appendChild( Shape );
	QDomElement Box = doc.createElement( "Box" );
	Box.setAttribute( "size", QString::number(size[0]) + " " + QString::number(size[1]) + " " + QString::number(size[2]) );
	Shape.appendChild( Box );
	QDomElement Appearance = doc.createElement( "Appearance" );
	Shape.appendChild( Appearance );
	if (Material.isNull())
	{
		Material = doc.createElement( "Material" );
		Material.setAttribute( "diffuseColor", "1.0 0.0 0.0" );
	}
	Appearance.appendChild( Material );

	elem.appendChild( Transform );
}

void export_X3D::export_polygon( QDomElement &elem, size_t count, double *array, double elevation, int normDir, QDomElement Material )
{
	if (count < 2)
		return;

	// VRML: cross section in xz-plane
	QString crossSection;
	crossSection = QString::number( array[0] ) + " " + QString::number( array[1] );
	for (size_t i=2; i<count; i=i+2)
	{
		crossSection += ", " + QString::number( array[i] ) + " " + QString::number( array[i+1] );
	}

	QDomDocument doc = elem.ownerDocument();
	QDomElement Transform = doc.createElement( "Transform" );
//	Transform.setAttribute( "translation", QString::number(center[0]) + " " + QString::number(center[1]) + " " + QString::number(center[2]) );
	QDomElement Shape = doc.createElement( "Shape" );
	Transform.appendChild( Shape );
	QDomElement Extrusion = doc.createElement( "Extrusion" );
	Extrusion.setAttribute( "crossSection", crossSection );
	if (normDir == 0)
		Extrusion.setAttribute( "orientation", "0 0 1 0" ); // FIXME not tested
	else if (normDir == 1)
		Extrusion.setAttribute( "orientation", "0 1 0 0" ); // FIXME not tested
	else
		Extrusion.setAttribute( "orientation", "1 0 0 " + QString::number(M_PI) );
	float delta = 1;
	Extrusion.setAttribute( "spine", "0 0 " + QString::number(elevation-delta) + ", 0 0 " + QString::number(elevation+delta) );
	Shape.appendChild( Extrusion );
	QDomElement Appearance = doc.createElement( "Appearance" );
	Shape.appendChild( Appearance );
	if (Material.isNull())
	{
		Material = doc.createElement( "Material" );
		Material.setAttribute( "diffuseColor", "1.0 0.0 0.0" );
	}
	Appearance.appendChild( Material );

	elem.appendChild( Transform );
}
