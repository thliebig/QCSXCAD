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

#include <QFileDialog>

#include "QVTKStructure.h"

#if VTK_MAJOR_VERSION>=9
  #include "QVTKOpenGLStereoWidget.h"
  #include "vtkGenericOpenGLRenderWindow.h"
#elif VTK_MAJOR_VERSION==8
  #include "QVTKOpenGLWidget.h"
  #include "vtkGenericOpenGLRenderWindow.h"
#else
  #include "QVTKWidget.h"

#endif
#include "vtkCommand.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkAxesActor.h"
#include "vtkActor.h"
#include "ContinuousStructure.h"
#include "ParameterCoord.h"
#include "VTKPrimitives.h"
#include "vtkCubeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkCellArray.h"
#include "vtkActor.h"
#include "vtkLODActor.h"
#include "vtkFollower.h"
#include "vtkAxes.h"
#include "vtkVectorText.h"
#include "vtkFollower.h"
#include "vtkTextProperty.h"
#include "vtkTextActor.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkPropAssembly.h"
#include "vtkTextProperty.h"
#include "vtkCaptionActor2D.h"
#include "vtkRectilinearGrid.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkDoubleArray.h"
#include "vtkActorCollection.h"
#include "vtkInteractorStyle.h"
#include "vtkCallbackCommand.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include "vtkInteractorStyleRubberBand2DPlane.h"
#include <vtkInteractorStyleRubberBand2D.h>

#include "CSPrimPoint.h"
#include "CSPrimBox.h"
#include "CSPrimMultiBox.h"
#include "CSPrimSphere.h"
#include "CSPrimSphericalShell.h"
#include "CSPrimCylinder.h"
#include "CSPrimCylindricalShell.h"
#include "CSPrimPolygon.h"
#include "CSPrimLinPoly.h"
#include "CSPrimRotPoly.h"
#include "CSPrimPolyhedron.h"
#include "CSPrimCurve.h"
#include "CSPrimWire.h"
#include "CSPrimUserDefined.h"

#include "CSPropDiscMaterial.h"

#include "CSTransform.h"

QVTKStructure::QVTKStructure()
{
	clCS=NULL;
	ActorGridPlane[0]=NULL;
	ActorGridPlane[1]=NULL;
	ActorGridPlane[2]=NULL;
	m_Rect_Grid = NULL;
	m_Struct_Grid = NULL;
	m_CamData = NULL;

	iResolution=32;
	AllowUpdate=true;

#if VTK_MAJOR_VERSION>=9
	VTKWidget = new QVTKOpenGLStereoWidget();
	VTKWidget->setRenderWindow(vtkGenericOpenGLRenderWindow::New());
#elif VTK_MAJOR_VERSION==8
	VTKWidget = new QVTKOpenGLWidget();
	VTKWidget->SetRenderWindow(vtkGenericOpenGLRenderWindow::New());
#else
	VTKWidget= new QVTKWidget();
#endif

	ren = vtkRenderer::New();
	VTKWidget->GetRenderWindow()->AddRenderer(ren);

	AddAxes();
	SetBackgroundColor(255,255,255);

	SetCallback(VTKWidget->GetRenderWindow()->GetInteractor());
}

QVTKStructure::~QVTKStructure()
{
	clear();
}

void QVTKStructure::AddAxes()
{
	Axes = vtkAxesActor::New();
	Axes->SetTotalLength(4,4,4);
	vtkOrientationMarkerWidget* marker = vtkOrientationMarkerWidget::New();
	vtkPropAssembly* assembly = vtkPropAssembly::New();
	assembly->AddPart(Axes);

	marker->SetOrientationMarker(assembly);
	marker->SetViewport(0.0,0.0,0.25,0.25);

	marker->SetInteractor(VTKWidget->GetRenderWindow()->GetInteractor());
	marker->SetEnabled(1);
	marker->InteractiveOff();

	//assembly->Delete();
	//marker->Delete();
}

void QVTKStructure::SetBackgroundColor(int r, int g, int b)
{
	double rgb[3]={(double)r/255.0,(double)g/255.0,(double)b/255.0};
	double irgb[3]={1-rgb[0],1-rgb[1],1-rgb[2]};
	ren->SetBackground(rgb);

	Axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(irgb);
	Axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(irgb);
	Axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(irgb);

	for (int i=0;i<3;++i)
	{
		if (ActorGridPlane[i]!=NULL) ActorGridPlane[i]->GetProperty()->SetColor(irgb);
	}
    VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::SetGeometry(ContinuousStructure *CS)
{
	clear();
	clCS=CS;
}

void QVTKStructure::clear()
{
	for (int i=0;i<LayerPrimitives.size();++i)
	{
		delete LayerPrimitives.at(i).VTKProp;
	}
	LayerPrimitives.clear();

	for (int i=0;i<m_DiscMatModels.size();++i)
	{
		delete m_DiscMatModels.at(i).vtk_model;
	}
	m_DiscMatModels.clear();

	for (int i=0; i<3; i++)
	{
		if (ActorGridPlane[i]!=NULL)
		{
			ren->RemoveActor(ActorGridPlane[i]);
			ActorGridPlane[i]->Delete();
			ActorGridPlane[i]=NULL;
		}
	}
	if (m_Rect_Grid)
		m_Rect_Grid->Delete();
	m_Rect_Grid=NULL;
	if (m_Struct_Grid)
		m_Struct_Grid->Delete();
	m_Struct_Grid=NULL;
}


void QVTKStructure::RenderGrid()
{
	if (clCS==NULL) return;
	CSRectGrid* CSGrid = clCS->GetGrid();
	if (CSGrid->isValid()==false)
		return;

	if (CSGrid->GetMeshType()==CARTESIAN)
	{
		if (m_Rect_Grid)
			m_Rect_Grid->Delete();
		m_Rect_Grid = vtkRectilinearGrid::New();
		vtkDoubleArray *Coords[3];
		int iQty[3];

		for (int n=0;n<3;++n)
		{
			iQty[n]=CSGrid->GetQtyLines(n);
			Coords[n]=vtkDoubleArray::New();
			for (int m=0;m<iQty[n];++m) Coords[n]->InsertNextValue(CSGrid->GetLine(n,m));
		}
		if (iQty[0]*iQty[1]*iQty[2]==0)
		{
			for (int n=0;n<3;++n) Coords[n]->Delete();
			return;
		}
		m_Rect_Grid->SetDimensions(iQty[0],iQty[1],iQty[2]);
		m_Rect_Grid->SetXCoordinates(Coords[0]);
		m_Rect_Grid->SetYCoordinates(Coords[1]);
		m_Rect_Grid->SetZCoordinates(Coords[2]);
		for (int n=0;n<3;++n)
			Coords[n]->Delete();
	}
	else if (CSGrid->GetMeshType()==CYLINDRICAL)
	{
		if (m_Struct_Grid)
			m_Struct_Grid->Delete();
		m_Struct_Grid = vtkStructuredGrid::New();

		unsigned int uiQty[3];
		double* lines[3]={NULL,NULL,NULL};
		for (unsigned int n=0;n<3;++n)
			lines[n] = CSGrid->GetLines(n,lines[n],uiQty[n]);

		m_Struct_Grid->SetDimensions(uiQty[0],uiQty[1],uiQty[2]);
		vtkPoints *points = vtkPoints::New();
		points->SetNumberOfPoints(uiQty[0]*uiQty[1]*uiQty[2]);
		double r[3];
		int id=0;
		for (unsigned int k=0; k<uiQty[2]; ++k)
			for (unsigned int j=0; j<uiQty[1]; ++j)
				for (unsigned int i=0; i<uiQty[0]; ++i)
				{
					r[0] = lines[0][i] * cos(lines[1][j]);
					r[1] = lines[0][i] * sin(lines[1][j]);
					r[2] = lines[2][k];
					points->SetPoint(id++,r);
				}
		m_Struct_Grid->SetPoints(points);
		points->Delete();
		for (unsigned int n=0;n<3;++n)
		{
			delete[] lines[n];
			lines[n] = NULL;
		}
	}
	else
		cerr << "QVTKStructure::RenderGrid(): Error, unknown grid type!" << endl;

	RenderGridDir(0,0);
	RenderGridDir(1,0);
	RenderGridDir(2,0);
}

void QVTKStructure::RenderGridX(int plane_pos)
{
	RenderGridDir(0,plane_pos);
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::RenderGridY(int plane_pos)
{
	RenderGridDir(1,plane_pos);
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();

}

void QVTKStructure::RenderGridZ(int plane_pos)
{
	RenderGridDir(2,plane_pos);
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::RenderGridDir(int dir, unsigned int plane_pos)
{
	if (ActorGridPlane[dir]!=NULL)
	{
		ren->RemoveActor(ActorGridPlane[dir]);
		ActorGridPlane[dir]->Delete();
	}

	ActorGridPlane[dir] = vtkLODActor::New();
	vtkPolyDataMapper *gridMapper = vtkPolyDataMapper::New();
	vtkPolyDataAlgorithm *plane = NULL;

	CSRectGrid* CSGrid = clCS->GetGrid();
	int uiQty[3];

	for (int n=0;n<3;++n)
		uiQty[n]=CSGrid->GetQtyLines(n);
	if ((int)plane_pos>=uiQty[dir])
	{
		cerr << "QVTKStructure::RenderGridDir: requested plane postion is out of range, resetting to max value!" << endl;
		plane_pos = uiQty[dir]-1;
	}

	if (CSGrid->GetMeshType()==CARTESIAN)
	{
		if (m_Rect_Grid==NULL)
		{
			ActorGridPlane[dir]->Delete();
			gridMapper->Delete();
			ActorGridPlane[dir]=NULL;
			cerr << "QVTKStructure::RenderGridDir: Error, rect grid mesh was not created, skipping drawing..." << endl;
			return;
		}
		vtkRectilinearGridGeometryFilter *grid_plane = vtkRectilinearGridGeometryFilter::New();
		plane = grid_plane;
#if VTK_MAJOR_VERSION>=6
		grid_plane->SetInputData(m_Rect_Grid);
#else
		grid_plane->SetInput(m_Rect_Grid);
#endif
		switch (dir)
		{
		case 2:
		{
			grid_plane->SetExtent(0,uiQty[0]-1, 0,uiQty[1]-1, plane_pos,plane_pos);
			break;
		}
		case 1:
		{
			grid_plane->SetExtent(0,uiQty[0]-1, plane_pos,plane_pos, 0,uiQty[2]-1);
			break;
		}
		case 0:
		{
			grid_plane->SetExtent(plane_pos,plane_pos, 0,uiQty[1]-1, 0,uiQty[2]-1);
			break;
		}
		}
	}
	else if (CSGrid->GetMeshType()==CYLINDRICAL)
	{
		if (m_Struct_Grid==NULL)
		{
			ActorGridPlane[dir]->Delete();
			gridMapper->Delete();
			ActorGridPlane[dir]=NULL;
			cerr << "QVTKStructure::RenderGridDir: Error, structured grid mesh was not created, skipping drawing..." << endl;
			return;
		}

		vtkStructuredGridGeometryFilter *grid_plane = vtkStructuredGridGeometryFilter::New();
		plane = grid_plane;
#if VTK_MAJOR_VERSION>=6
		grid_plane->SetInputData(m_Struct_Grid);
#else
		grid_plane->SetInput(m_Struct_Grid);
#endif
		switch (dir)
		{
		case 2:
		{
			grid_plane->SetExtent(0,uiQty[0]-1, 0,uiQty[1]-1, plane_pos,plane_pos);
			break;
		}
		case 1:
		{
			grid_plane->SetExtent(0,uiQty[0]-1, plane_pos,plane_pos, 0,uiQty[2]-1);
			break;
		}
		case 0:
		{
			grid_plane->SetExtent(plane_pos,plane_pos, 0,uiQty[1]-1, 0,uiQty[2]-1);
			break;
		}
		}
	}
	else
		cerr << "QVTKStructure::RenderGrid(): Error, unknown grid type!" << endl;

	gridMapper->SetInputConnection(plane->GetOutputPort());
	ActorGridPlane[dir]->SetMapper(gridMapper);
	ActorGridPlane[dir]->GetProperty()->SetColor(0,0,0);
	ActorGridPlane[dir]->GetProperty()->SetDiffuse(0);
	ActorGridPlane[dir]->GetProperty()->SetAmbient(1);
	ActorGridPlane[dir]->GetProperty()->SetRepresentationToWireframe();
	ActorGridPlane[dir]->GetProperty()->SetOpacity((double)GridOpacity/255.0);
	ren->AddActor(ActorGridPlane[dir]);
	gridMapper->Delete();
	plane->Delete();

}

void QVTKStructure::SetGridOpacity(int val)
{
	GridOpacity = val;
	if (AllowUpdate==false) return;
	for (int i=0;i<3;++i)
	{
		if (ActorGridPlane[i]!=NULL) ActorGridPlane[i]->GetProperty()->SetOpacity((double)val/255.0);
	}
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::ResetView()
{
	ren->ResetCamera();
    VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::setXY()
{
	vtkCamera* cam=ren->GetActiveCamera();
	ren->ResetCamera();
	double fp[3];
	cam->SetViewUp(0.5,0.5,0.5);
	cam->GetFocalPoint(fp);
	fp[2]+=1;
	cam->SetPosition(fp);
	cam->SetRoll(0);
	ResetView();
}

void QVTKStructure::setYZ()
{
	vtkCamera* cam=ren->GetActiveCamera();
	ren->ResetCamera();
	double fp[3];
	cam->SetViewUp(0.5,0.5,0.5);
	cam->GetFocalPoint(fp);
	fp[0]+=1;
	cam->SetPosition(fp);

	cam->SetRoll(-90);
	ResetView();
}


void QVTKStructure::setZX()
{
	vtkCamera* cam=ren->GetActiveCamera();
	ren->ResetCamera();
	double fp[3];
	cam->SetViewUp(0.5,0.5,0.5);
	cam->GetFocalPoint(fp);
	fp[1]+=1;
	cam->SetPosition(fp);
	cam->SetRoll(90);
	ResetView();
}

void QVTKStructure::SetPropOpacity(unsigned int uiID, int val)
{
	for (int i=0;i<LayerPrimitives.size();++i)
	{
		if (LayerPrimitives.at(i).uID==uiID)
		{
			if (LayerPrimitives.at(i).VTKProp!=NULL) LayerPrimitives.at(i).VTKProp->SetOpacity2All((double)val/255.0);
		}
	}
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::RenderGeometry()
{
	for (int i=0;i<LayerPrimitives.size();++i)
	{
		delete LayerPrimitives.at(i).VTKProp;
	}
	LayerPrimitives.clear();
	if (clCS==NULL) return;
	int QtyProp=clCS->GetQtyProperties();
	for (int i=0;i<QtyProp;++i)
	{
		CSProperties* prop = clCS->GetProperty(i);
		if (prop==NULL) return;
		int QtyPrim=prop->GetQtyPrimitives();
		if (QtyPrim>0)
		{
			VTKLayerStruct layStruct;
			VTKPrimitives* vtkPrims= new VTKPrimitives(ren);
			layStruct.VTKProp=vtkPrims;
			layStruct.uID=prop->GetUniqueID();
			LayerPrimitives.append(layStruct);
			RGBa col=prop->GetFillColor();
			if (prop->GetVisibility()==false) col.a=0;
			double rgb[3]={(double)col.R/255.0,(double)col.G/255.0,(double)col.B/255.0};
			for (int n=0;n<QtyPrim;++n)
			{
				CSPrimitives* prim = prop->GetPrimitive(n);
				if (prim==NULL) return;
				CoordinateSystem primCS = prim->GetCoordinateSystem();
				CSTransform* transform = prim->GetTransform();
				double* transform_matrix = NULL;
				if (transform)
					transform_matrix = transform->GetMatrix();
				if (primCS==UNDEFINED_CS)
					primCS=clCS->GetCoordInputType();
				switch (prim->GetType())
				{
				case CSPrimitives::BOX:
				{
					CSPrimBox* box = prim->ToBox();
					if (primCS==CARTESIAN)
						vtkPrims->AddCube(box->GetStartCoord()->GetCartesianCoords(),box->GetStopCoord()->GetCartesianCoords(),rgb,(double)col.a/255.0,transform_matrix);
					else if (primCS==CYLINDRICAL)
						vtkPrims->AddCylindricalCube(box->GetStartCoord()->GetCylindricalCoords(),box->GetStopCoord()->GetCylindricalCoords(),rgb,(double)col.a/255.0,transform_matrix);
					break;
				}
				case CSPrimitives::MULTIBOX:
				{
					CSPrimMultiBox* multibox = prim->ToMultiBox();
					int qtyPts=multibox->GetQtyBoxes()*2;
					double *coords = new double[qtyPts*3];
					for (int a=0;a<qtyPts;a=a+2)
					{
						coords[a]=multibox->GetCoord(3*a);
						coords[a+1]=multibox->GetCoord(3*a+1);
						coords[qtyPts+a]=multibox->GetCoord(3*a+2);
						coords[qtyPts+a+1]=multibox->GetCoord(3*a+3);
						coords[2*qtyPts+a]=multibox->GetCoord(3*a+4);
						coords[2*qtyPts+a+1]=multibox->GetCoord(3*a+5);
					}
					vtkPrims->AddDisc(coords,qtyPts,rgb,(double)col.a/255.0,transform_matrix);
					delete[] coords;
					break;
				}
				case CSPrimitives::SPHERE:
				{
					CSPrimSphere* sphere = prim->ToSphere();
					vtkPrims->AddSphere(sphere->GetCenter()->GetCartesianCoords(),sphere->GetRadius(),rgb,(double)col.a/255.0,iResolution,transform_matrix);
					break;
				}
				case CSPrimitives::SPHERICALSHELL:
				{
					CSPrimSphericalShell* sphereshell = prim->ToSphericalShell();
					const double* center = sphereshell->GetCenter()->GetCartesianCoords();
					const double radius = sphereshell->GetRadius();
					const double shellWidth = sphereshell->GetShellWidth();
					vtkPrims->AddSphericalShell(center, radius-shellWidth/2, radius+shellWidth/2, rgb, (double)col.a/255.0, iResolution, transform_matrix);
					break;
				}
				case CSPrimitives::CYLINDER:
				{
					CSPrimCylinder* cylinder = prim->ToCylinder();
					vtkPrims->AddCylinder2(cylinder->GetAxisStartCoord()->GetCartesianCoords(),cylinder->GetAxisStopCoord()->GetCartesianCoords(),cylinder->GetRadius(),rgb,(double)col.a/255.0,iResolution,transform_matrix);
					break;
				}
				case CSPrimitives::CYLINDRICALSHELL:
				{
					CSPrimCylindricalShell* cylinder = prim->ToCylindricalShell();
					const double* start = cylinder->GetAxisStartCoord()->GetCartesianCoords();
					const double* stop  = cylinder->GetAxisStopCoord()->GetCartesianCoords();
					const double radius = cylinder->GetRadius();
					const double shellWidth = cylinder->GetShellWidth();
					vtkPrims->AddCylindricalShell( start, stop, radius-shellWidth/2, radius+shellWidth/2, rgb, (double)col.a/255.0, iResolution, transform_matrix );
					break;
				}
				case CSPrimitives::POLYGON:
				case CSPrimitives::LINPOLY:
				case CSPrimitives::ROTPOLY:
				{
					CSPrimPolygon* poly = NULL;
					if (prim->GetType()==CSPrimitives::POLYGON)
						poly = prim->ToPolygon();
					else if (prim->GetType()==CSPrimitives::LINPOLY)
						poly = prim->ToLinPoly();
					else if (prim->GetType()==CSPrimitives::ROTPOLY)
						poly = prim->ToRotPoly();
					int normDir = poly->GetNormDir();
					double elev = poly->GetElevation();
					int nP = (normDir+1)%3;
					int nPP = (normDir+2)%3;
					int nrPts = poly->GetQtyCoords();
					double* dCoords = new double[3*nrPts];
					for (int n=0;n<nrPts;++n)
					{
						dCoords[normDir*nrPts + n] = elev;
						dCoords[nP*nrPts + n] = poly->GetCoord(2*n);
						dCoords[nPP*nrPts + n] = poly->GetCoord(2*n+1);
					}
					double dVector[6] = {0,0,0,0,0,0};
					if (prim->GetType()==CSPrimitives::POLYGON)
						vtkPrims->AddClosedPoly(dCoords,nrPts,dVector,rgb,(double)col.a/255.0,transform_matrix);
					if (prim->GetType()==CSPrimitives::LINPOLY)
					{
						dVector[normDir] = prim->ToLinPoly()->GetLength();
						vtkPrims->AddClosedPoly(dCoords,nrPts,dVector,rgb,(double)col.a/255.0,transform_matrix);
					}
					if (prim->GetType()==CSPrimitives::ROTPOLY)
					{
						dVector[2*prim->ToRotPoly()->GetRotAxisDir()+1]=1;
						double angles[2] = {prim->ToRotPoly()->GetAngle(0)*180/PI,prim->ToRotPoly()->GetAngle(1)*180/PI};
						vtkPrims->AddRotationalPoly(dCoords,nrPts,dVector,angles,rgb,(double)col.a/255.0,32,transform_matrix);
					}
					delete[] dCoords; dCoords=NULL;
					break;
				}
				case CSPrimitives::POLYHEDRONREADER:
				case CSPrimitives::POLYHEDRON:
				{
					CSPrimPolyhedron* polyhedron = dynamic_cast<CSPrimPolyhedron*>(prim);
					vtkCellArray *poly = vtkCellArray::New();
					vtkPolyData* polydata=vtkPolyData::New();
					vtkPoints *points = vtkPoints::New();
					for (unsigned int i=0; i<polyhedron->GetNumVertices();i++)
						points->InsertPoint(i,polyhedron->GetVertex(i));
					unsigned int numVertex;
					int* vertices;
					for (unsigned int i=0; i<polyhedron->GetNumFaces();++i)
					{
						if (polyhedron->GetFaceValid(i)==false)
							continue;
						vertices=polyhedron->GetFace(i,numVertex);
						poly->InsertNextCell(numVertex);
						for (unsigned int p=0; p<numVertex;++p)
							poly->InsertCellPoint(vertices[p]);
					}
					polydata->SetPoints(points);
					polydata->SetPolys(poly);
					vtkPrims->AddPolyData(polydata,rgb,(double)col.a/255.0,transform_matrix);
					poly->Delete();
					points->Delete();
					polydata->Delete();
					break;
				}
//				case CSPrimitives::POLYHEDRONREADER:
//				{
//					CSPrimPolyhedronReader* reader = prim->ToPolyhedronReader();
//					double center[]={0,0,0};
//					vtkPrims->AddSTLObject(reader->GetFilename().c_str(),center,rgb,(double)col.a/255.0,transform_matrix);
//					break;
//				}
				case CSPrimitives::CURVE:
				case CSPrimitives::WIRE:
				{
					CSPrimCurve* curve = NULL;
					if (prim->GetType()==CSPrimitives::CURVE)
						curve = prim->ToCurve();
					else
						curve = prim->ToWire();

					unsigned int nrP = (unsigned int)curve->GetNumberOfPoints();
					double* dCoords = new double[3*nrP];
					double xyz[3];
					bool isCurve = (prim->GetType()==CSPrimitives::CURVE);
					for (unsigned int n=0;n<nrP;++n)
					{
						curve->GetPoint(n,xyz,CARTESIAN,isCurve);
						dCoords[0*nrP+n] = xyz[0];
						dCoords[1*nrP+n] = xyz[1];
						dCoords[2*nrP+n] = xyz[2];
					}
					if (isCurve)
						vtkPrims->AddLinePoly(dCoords,nrP,1,rgb,(double)col.a/255.0);
					else
					{
						CSPrimWire* wire = prim->ToWire();
						vtkPrims->AddTubePoly(dCoords,nrP,wire->GetWireRadius(),rgb,(double)col.a/255.0,8,transform_matrix);
					}
					delete[] dCoords; dCoords=NULL;
					break;
				}
				}
			}
		}
	}
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::RenderDiscMaterialModel()
{
	for (int i=0;i<m_DiscMatModels.size();++i)
	{
		delete m_DiscMatModels.at(i).vtk_model;
	}
	m_DiscMatModels.clear();

	if (clCS==NULL) return;

	for (unsigned int i=0;i<clCS->GetQtyProperties();++i)
	{
		CSProperties* prop = clCS->GetProperty(i);
		CSPropDiscMaterial* dm_prop = prop->ToDiscMaterial();
		if (dm_prop)
		{
			VTKDiscModel model;
			VTKPrimitives* vtkPrims= new VTKPrimitives(ren);
			model.vtk_model = vtkPrims;
			model.uID = dm_prop->GetUniqueID();
			m_DiscMatModels.append(model);
			vtkPolyData* polydata = dm_prop->CreatePolyDataModel();

			double rgb[3] = {1,1,1};
			CSTransform* transform = new CSTransform(dm_prop->GetTransform());
			transform->SetPreMultiply();
			transform->Scale(dm_prop->GetScale());
			double* transform_matrix = NULL;
			if (transform)
				transform_matrix = transform->GetMatrix();
			vtkPrims->AddPolyData(polydata,rgb,1.0,transform_matrix);
			delete transform;
		}
	}
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::SetParallelProjection(bool val, bool render)
{
	vtkCamera* cam = ren->GetActiveCamera();
	cam->SetParallelProjection(val);

	if (render)
		VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::Set2DInteractionStyle(bool val, bool render)
{
	if (val)
		VTKWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(vtkInteractorStyleRubberBand2DPlane::New());
	else
		VTKWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(vtkInteractorStyleTrackballCamera::New());

	if (render)
		VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::SaveCamData()
{
	if (m_CamData==NULL)
		m_CamData = new CamData;

	vtkCamera *Camera = ren->GetActiveCamera();

	Camera->GetPosition(m_CamData->pos);
	Camera->GetFocalPoint(m_CamData->focalPoint);
	Camera->GetViewUp(m_CamData->viewUp);
	m_CamData->viewAngle = Camera->GetViewAngle();
}

void QVTKStructure::RestoreCamData(bool render)
{
	if (m_CamData==NULL)
		return;

	vtkCamera *Camera = ren->GetActiveCamera();
	Camera->SetPosition( m_CamData->pos );
	Camera->SetFocalPoint( m_CamData->focalPoint );
	Camera->SetViewUp( m_CamData->viewUp );
	Camera->SetViewAngle( m_CamData->viewAngle );
	Camera->Modified();

	if (render)
		VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}


void QVTKStructure::ExportView2Image()
{
	QString filename = QFileDialog::getSaveFileName(VTKWidget, tr("Choose file to save image"), QString(), tr("Images (*.png)"));

	if (filename.isEmpty())
		return;

	vtkWindowToImageFilter* filter = vtkWindowToImageFilter::New();
	filter->SetInput(VTKWidget->GetRenderWindow());

	vtkPNGWriter* png_writer= vtkPNGWriter::New();
	png_writer->SetInputConnection(filter->GetOutputPort());
	//png_writer->SetQuality(100);
	png_writer->SetFileName(filename.toStdString().c_str());
	png_writer->Write();
}

void QVTKStructure::ExportProperty2PolyDataVTK(unsigned int uiID, QString filename, double scale)
{
	for (int i=0;i<LayerPrimitives.size();++i)
	{
		if (LayerPrimitives.at(i).uID==uiID)
		{
			if (LayerPrimitives.at(i).VTKProp!=NULL)
			{
				QString name = filename + ".vtp";
				LayerPrimitives.at(i).VTKProp->WritePolyData2File(name.toStdString().c_str(), scale);
			}
		}
	}

	for (int i=0;i<m_DiscMatModels.size();++i)
	{
		if (m_DiscMatModels.at(i).uID==uiID)
		{
			if (m_DiscMatModels.at(i).vtk_model!=NULL)
			{
				QString name = filename + "_DiscMaterial" + ".vtp";
				m_DiscMatModels.at(i).vtk_model->WritePolyData2File(name.toStdString().c_str(), scale);
			}
		}
	}
}

void QVTKStructure::ExportProperty2STL(unsigned int uiID, QString filename, double scale)
{
	for (int i=0;i<LayerPrimitives.size();++i)
	{
		if (LayerPrimitives.at(i).uID==uiID)
		{
			if (LayerPrimitives.at(i).VTKProp!=NULL)
			{
				QString name = filename + ".stl";
				LayerPrimitives.at(i).VTKProp->WritePolyData2STL(name.toStdString().c_str(), scale);
			}
		}
	}
}

void QVTKStructure::ExportProperty2PLY(unsigned int uiID, QString filename, double scale)
{
	for (int i=0;i<LayerPrimitives.size();++i)
	{
		if (LayerPrimitives.at(i).uID==uiID)
		{
			if (LayerPrimitives.at(i).VTKProp!=NULL)
			{
				QString name = filename + ".ply";
				LayerPrimitives.at(i).VTKProp->WritePolyData2PLY(name.toStdString().c_str(), scale);
			}
		}
	}
}

void QVTKStructure::KeyPress(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata)
{
	UNUSED(caller);
	UNUSED(eid);
	UNUSED(calldata);

	//vtkInteractorStyle * istyle = (vtkInteractorStyle *) caller;
  	vtkRenderWindowInteractor * iren = ((KeyPressData *)clientdata)->iren;
	//vtkRenderWindow *renWin = iren->GetRenderWindow();
	vtkActor **GridPlanes = ((KeyPressData *)clientdata)->GridPlanes;
	//vtkRenderer *ren = ((KeyPressData *)clientdata)->ren;
	int key;
	key=iren->GetKeyCode();
	//	fprintf(stderr,"Event... EiD: %d Key: %d OpenGL?: %d\n",eid,key,renWin->SupportsOpenGL());
	switch(key)
	{
	case 's':
	{
		for (int n=0;n<3;++n)
		{
			if (GridPlanes[n])
				GridPlanes[n]->GetProperty()->SetRepresentationToWireframe();
		}
		iren->Render();
		break;
	}
	}
}

void QVTKStructure::SetCallback(vtkRenderWindowInteractor *iren)
{
	ren->GetActiveCamera()->SetFocalPoint(0,0,0);
	//Callback
	KeyPressData *cbData = new KeyPressData;
	cbData->GridPlanes=ActorGridPlane;
	cbData->ren=ren;
	cbData->iren=iren;

	vtkCallbackCommand *cb = vtkCallbackCommand::New();
	cb->SetCallback(KeyPress);
	cb->SetClientData((void *)cbData);
	iren->AddObserver(vtkCommand::KeyReleaseEvent, cb);
	//VTKWidget->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::KeyReleaseEvent, cb);

	cb->Delete();
	//	free(cbData);
}
