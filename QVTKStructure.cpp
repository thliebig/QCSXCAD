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

#include "QVTKWidget.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkAxesActor.h"
#include "vtkActor.h"
#include "ContinuousStructure.h"
#include "VTKPrimitives.h"
#include "QVTKStructure.h"
#include "vtkCubeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
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
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"

QVTKStructure::QVTKStructure()
{
	clCS=NULL;
	ActorGridPlane[0]=NULL;
	ActorGridPlane[1]=NULL;
	ActorGridPlane[2]=NULL;

	iResolution=16;

	ren = vtkRenderer::New();

	VTKWidget= new QVTKWidget();

  	ren = vtkRenderer::New();
  	VTKWidget->GetRenderWindow()->AddRenderer(ren);

	AddAxes();
	SetBackgroundColor(255,255,255);

	SetCallback(VTKWidget->GetRenderWindow()->GetInteractor());

	//test cube....
//	vtkCubeSource *Source = vtkCubeSource::New();
//	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
//	vtkActor *SourceActor = vtkActor::New();
//	Source->SetBounds(0,1000,0,1000,0,2000);
//	SourceMapper->SetInput(Source->GetOutput());
//	SourceActor->SetMapper(SourceMapper);
//	SourceActor->GetProperty()->SetColor(0,1,0);
//	//SourceActor->GetProperty()->SetOpacity(dOpacity);
//	ren->AddActor(SourceActor);
//	//polydata->AddItem(Source->GetOutput());
//	Source->Delete();
//	SourceMapper->Delete();
//	SourceActor->Delete();
}

QVTKStructure::~QVTKStructure()
{
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

	for (int i=0; i<3; i++)
	{
		if (ActorGridPlane[i]!=NULL)
		{
			ren->RemoveActor(ActorGridPlane[i]);
			ActorGridPlane[i]->Delete();
			ActorGridPlane[i]=NULL;
		}
	}
}


void QVTKStructure::RenderGrid()
{
	if (clCS==NULL) return;
	CSRectGrid* CSGrid = clCS->GetGrid();

	vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
	vtkDoubleArray *Coords[3];
	int iQty[3];
	// =  vtkDoubleArray::New();
	//vtkDoubleArray *yCoords =  vtkDoubleArray::New();
	//vtkDoubleArray *zCoords =  vtkDoubleArray::New();

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
    grid->SetDimensions(iQty[0],iQty[1],iQty[2]);
    grid->SetXCoordinates(Coords[0]);
    grid->SetYCoordinates(Coords[1]);
    grid->SetZCoordinates(Coords[2]);
	for (int i=0; i<3; i++)
	{
		if (ActorGridPlane[i]!=NULL)
		{
			ren->RemoveActor(ActorGridPlane[i]);
			ActorGridPlane[i]->Delete();
		}
		ActorGridPlane[i] = vtkActor::New();
		vtkPolyDataMapper *gridMapper = vtkPolyDataMapper::New();
	    vtkRectilinearGridGeometryFilter *plane = vtkRectilinearGridGeometryFilter::New();
	    plane->SetInput(grid);
		switch (i)
		{
		case 0:
			{
				plane->SetExtent(0,iQty[0]-1, 0,iQty[1]-1, 0,0);
				break;
			}
		case 1:
			{
				plane->SetExtent(0,iQty[0]-1, 0,0, 0,iQty[2]-1);
				break;
			}
		case 2:
			{
				plane->SetExtent(0,0, 0,iQty[1]-1, 0,iQty[2]-1);
				break;
			}
		}
		gridMapper->SetInput(plane->GetOutput());
		//WireFrameOnlyActorCol->AddItem(Actor);
		ActorGridPlane[i]->SetMapper(gridMapper);
	    ActorGridPlane[i]->GetProperty()->SetColor(0,0,0);
		ActorGridPlane[i]->GetProperty()->SetDiffuse(0);
		ActorGridPlane[i]->GetProperty()->SetAmbient(1);
	    ActorGridPlane[i]->GetProperty()->SetRepresentationToWireframe();
		ren->AddActor(ActorGridPlane[i]);
		gridMapper->Delete();
		plane->Delete();
		Coords[i]->Delete();
	}
	SetGridOpacity(GridOpacity);
	grid->Delete();
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
				switch (prim->GetType())
				{
					case CSPrimitives::BOX:
					{
						CSPrimBox* box = prim->ToBox();
						double coords[6];
						double val0,val1;
						for (unsigned int a=0;a<3;++a)
						{
							val0=box->GetCoord(2*a);
							val1=box->GetCoord(2*a+1);
							if (val0<=val1)
							{
								coords[2*a]=val0;
								coords[2*a+1]=val1;
							}
							else
							{
								coords[2*a]=val1;
								coords[2*a+1]=val0;
							}
						}
						vtkPrims->AddCube(coords,rgb,(double)col.a/255.0);
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
						vtkPrims->AddDisc(coords,qtyPts,rgb,(double)col.a/255.0);
						delete[] coords;
						break;
					}
					case CSPrimitives::SPHERE:
					{
						CSPrimSphere* sphere = prim->ToSphere();
						double coords[3];
						for (int a=0;a<3;++a) coords[a]=sphere->GetCoord(a);
						vtkPrims->AddSphere(coords,sphere->GetRadius(),rgb,(double)col.a/255.0,iResolution);
						break;
					}
					case CSPrimitives::CYLINDER:
					{
						CSPrimCylinder* cylinder = prim->ToCylinder();
						double start[3];
						double direction[3];
						for (int a=0;a<3;++a) start[a]=cylinder->GetCoord(2*a);
						for (int a=0;a<3;++a) direction[a]=cylinder->GetCoord(2*a+1)-start[a];
						vtkPrims->AddCylinder(start,direction,cylinder->GetRadius(),rgb,(double)col.a/255.0,iResolution);
						break;
					}
				case CSPrimitives::POLYGON:
				case CSPrimitives::LINPOLY:
					{
						CSPrimPolygon* poly = NULL;
						if (prim->GetType()==CSPrimitives::POLYGON)
							poly = prim->ToPolygon();
						else
							poly = prim->ToLinPoly();
						int normDir = 0;
						double elev = poly->GetElevation();
						double dExtrusionVector[3] = {0,0,0};
						if (poly->GetNormDir(0) != 0) normDir = 0;
						else if (poly->GetNormDir(1) != 0) normDir = 1;
						else if (poly->GetNormDir(2) != 0) normDir = 2;
						if (prim->GetType()==CSPrimitives::LINPOLY)
							dExtrusionVector[normDir] = prim->ToLinPoly()->GetLength();
						int nP = (normDir+1)%3;
						int nPP = (normDir+2)%3;
						int nrPts = poly->GetQtyCoords();
						double dCoords[3*nrPts];
						for (int n=0;n<nrPts;++n)
						{
							dCoords[normDir*nrPts + n] = elev;
							dCoords[nP*nrPts + n] = poly->GetCoord(2*n);
							dCoords[nPP*nrPts + n] = poly->GetCoord(2*n+1);
						}
						vtkPrims->AddClosedPoly(dCoords,nrPts,dExtrusionVector,rgb,(double)col.a/255.0);
						break;
					}
				}
			}
		}
	}
	VTKWidget->GetRenderWindow()->GetInteractor()->Render();
}

void QVTKStructure::ExportView2Image()
{
	QString filename = QFileDialog::getSaveFileName(VTKWidget, tr("Choose file to save image"), QString(), tr("Images (*.png)"));

	if (filename.isEmpty())
		return;
	filename += ".png";

	vtkWindowToImageFilter* filter = vtkWindowToImageFilter::New();
	filter->SetInput(VTKWidget->GetRenderWindow());

	vtkPNGWriter* png_writer= vtkPNGWriter::New();
	png_writer->SetInput(filter->GetOutput());
	//png_writer->SetQuality(100);
	png_writer->SetFileName(filename.toStdString().c_str());
	png_writer->Write();
}

void QVTKStructure::KeyPress(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata)
{
	//vtkInteractorStyle * istyle = (vtkInteractorStyle *) caller;
  	vtkRenderWindowInteractor * iren = ((KeyPressData *)clientdata)->iren;
	//vtkRenderWindow *renWin = iren->GetRenderWindow();
	vtkActor **GridPlanes = ((KeyPressData *)clientdata)->GridPlanes;
	vtkRenderer *ren = ((KeyPressData *)clientdata)->ren;
	int key;
	key=iren->GetKeyCode();
//	fprintf(stderr,"Event... EiD: %d Key: %d OpenGL?: %d\n",eid,key,renWin->SupportsOpenGL());
	switch(key)
	{
		case 's':
			{
//				vtkActor *Actor;
//				WireFrameOnlyActorCol->InitTraversal();
//				while (Actor=WireFrameOnlyActorCol->GetNextActor() )  Actor->GetProperty()->SetRepresentationToWireframe();
				GridPlanes[0]->GetProperty()->SetRepresentationToWireframe();
				GridPlanes[1]->GetProperty()->SetRepresentationToWireframe();
				GridPlanes[2]->GetProperty()->SetRepresentationToWireframe();
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
