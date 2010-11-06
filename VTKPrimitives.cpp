/*
*	Copyright (C) 2005,2006,2007,2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "VTKPrimitives.h"
#include "vtkRenderer.h"
#include "vtkActorCollection.h"

#include "vtkPolyDataMapper.h"
#include "vtkDoubleArray.h"
#include "vtkLODActor.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkCubeSource.h"
#include "vtkPlaneSource.h"
#include "vtkCylinderSource.h"
#include "vtkLineSource.h"
#include "vtkSphereSource.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkRotationalExtrusionFilter.h"
#include "vtkLinearExtrusionFilter.h"
#include "vtkArrowSource.h"
#include "vtkSTLReader.h"
#include "vtkVectorText.h"
#include "vtkFollower.h"
#include "vtkTriangleFilter.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkCollectionIterator.h"
#include "vtkConeSource.h"
#include "vtkTubeFilter.h"
#include "vtkAppendPolyData.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkSTLWriter.h"


VTKPrimitives::VTKPrimitives(vtkRenderer *Renderer)
{
	ren = Renderer;

	ActorColl = vtkActorCollection::New();

	m_PolyDataCollection = vtkAppendPolyData::New();
	m_ArcDelta = 3.0/180.0*PI; // default of 5° resolution for representing angles
}

VTKPrimitives::~VTKPrimitives()
{
	ActorColl->InitTraversal();
	vtkActor* act=NULL;
	while ((act=ActorColl->GetNextActor()))
	{
		ren->RemoveActor(act);
		act->Delete();
	}
	ActorColl->Delete();
	ActorColl=NULL;

	if (m_PolyDataCollection)
		m_PolyDataCollection->Delete();
	m_PolyDataCollection=NULL;
}

VTKPrimitives::VTKPrimitives()
{
}

void VTKPrimitives::AddCube(const double *start, const double *stop, double *dRGB, double dOpacity)
{
	double coords[6] = {start[0],stop[0],start[1],stop[1],start[2],stop[2]};
	double help;
	//swap start stop if start>stop
	for (int n=0;n<3;++n)
	{
		if (coords[2*n]>coords[2*n+1])
		{
			help=coords[2*n+1];
			coords[2*n+1]=coords[2*n];
			coords[2*n]=help;
		}
	}
	AddCube(coords,dRGB,dOpacity);
}

void VTKPrimitives::AddCube(double *dCoords, double *dRGB, double dOpacity)
{
	//create a simple cartesian cube...
	double CC[6];
	TransformCylindricalCoords(dCoords,CC,2);

	vtkCubeSource *Source = vtkCubeSource::New();
	Source->SetBounds(CC);
	m_PolyDataCollection->AddInput(Source->GetOutput());
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	SourceMapper->SetInput(Source->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);
	Source->Delete();
//	SourceMapper->Delete();
	//SourceActor->Delete();
}

void VTKPrimitives::AddCylindricalCube(const double *start, const double *stop, double *dRGB, double dOpacity)
{
	double coords[6] = {start[0],stop[0],start[1],stop[1],start[2],stop[2]};
	double help;
	//swap start stop if start>stop
	for (int n=0;n<3;++n)
	{
		if (coords[2*n]>coords[2*n+1])
		{
			help=coords[2*n+1];
			coords[2*n+1]=coords[2*n];
			coords[2*n]=help;
		}
	}
	AddCylindricalCube(coords,dRGB,dOpacity);
}


void VTKPrimitives::AddCylindricalCube(double *dCoords, double *dRGB, double dOpacity)
{
	vtkPolyDataAlgorithm* PDSource = NULL;
	vtkPolyDataAlgorithm* PDFilter = NULL;

	if ( (dCoords[2]!=dCoords[3]) )
	{
		if ((dCoords[0]!=dCoords[1]) && ((dCoords[4]!=dCoords[5])))  //3D object
		{
			double dO[3] = {dCoords[0],dCoords[2],dCoords[4]};
			double dP1[3] = {dCoords[0],dCoords[2],dCoords[5]};
			double dP2[3] = {dCoords[1],dCoords[2],dCoords[4]};
			double out[3];
			vtkPlaneSource *Source = vtkPlaneSource::New();
			Source->SetOrigin(TransformCylindricalCoords(dO,out));
			Source->SetPoint1(TransformCylindricalCoords(dP1,out));
			Source->SetPoint2(TransformCylindricalCoords(dP2,out));
			PDSource = Source;
		}
		else if (dCoords[4]!=dCoords[5])	// alpha-z plane
		{
			double dP1[3] = {dCoords[0],dCoords[2],dCoords[4]};
			double dP2[3] = {dCoords[0],dCoords[2],dCoords[5]};
			double out[3];

			vtkLineSource *Source = vtkLineSource::New();
			Source->SetPoint1(TransformCylindricalCoords(dP1,out));
			Source->SetPoint2(TransformCylindricalCoords(dP2,out));
			PDSource = Source;
		}
		else if (dCoords[0]!=dCoords[1])	// alpha-r plane
		{
			double dP1[3] = {dCoords[0],dCoords[2],dCoords[4]};
			double dP2[3] = {dCoords[1],dCoords[2],dCoords[4]};
			double out[3];

			vtkLineSource *Source = vtkLineSource::New();
			Source->SetPoint1(TransformCylindricalCoords(dP1,out));
			Source->SetPoint2(TransformCylindricalCoords(dP2,out));
			PDSource = Source;
		}

		if (PDSource==NULL)
			return;

		vtkRotationalExtrusionFilter *extrude = vtkRotationalExtrusionFilter::New();
		extrude->SetInput(PDSource->GetOutput());
		int nrSteps = ceil(fabs(dCoords[3]-dCoords[2])/m_ArcDelta);
		extrude->SetResolution(nrSteps);
		extrude->SetAngle( (dCoords[3]-dCoords[2])*180/PI );
		PDFilter = extrude;
	}
	else //rz-plane or line
	{
		if (dCoords[0]==dCoords[1]) //line in z-direction
		{
			return;
		}
		if (dCoords[4]==dCoords[5]) //line in r-direction
		{
			return;
		}

		//rz-plane
		double dO[3] = {dCoords[0],dCoords[2],dCoords[4]};
		double dP1[3] = {dCoords[0],dCoords[2],dCoords[5]};
		double dP2[3] = {dCoords[1],dCoords[2],dCoords[4]};
		double out[3];
		vtkPlaneSource *Source = vtkPlaneSource::New();
		Source->SetOrigin(TransformCylindricalCoords(dO,out));
		Source->SetPoint1(TransformCylindricalCoords(dP1,out));
		Source->SetPoint2(TransformCylindricalCoords(dP2,out));
		PDFilter = Source;
	}

	if (PDFilter==NULL)
		return;

	m_PolyDataCollection->AddInput(PDFilter->GetOutput());
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	SourceMapper->SetInput(PDFilter->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);

	if (PDSource)
		PDSource->Delete();
	PDFilter->Delete();
}

void VTKPrimitives::AddPlane(double *dOrigin, double* dP1, double* dP2, double *dRGB, double dOpacity)
{
	double out[3];
	vtkPlaneSource *Source = vtkPlaneSource::New();
	Source->SetOrigin(TransformCylindricalCoords(dOrigin,out));
	Source->SetPoint1(TransformCylindricalCoords(dP1,out));
	Source->SetPoint2(TransformCylindricalCoords(dP2,out));

	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	SourceMapper->SetInput(Source->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);
	m_PolyDataCollection->AddInput(Source->GetOutput());
	Source->Delete();
	SourceMapper->Delete();
}


void VTKPrimitives::AddDisc(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity)
{
	unsigned int i=0,j=0;//,h=0;//,k=0;
//	vtkIdType pts[6][4]={{0,1,2,3}, {4,5,6,7}, {0,1,5,4},
//                        {1,2,6,5}, {2,3,7,6}, {3,0,4,7}};
	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();
	//vtkFloatArray *scalars = vtkFloatArray::New();
	for (i=0; i<uiQtyCoords;i=i+2) 
	{
		points->InsertPoint(j++,dCoords[i],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i]);//0
		points->InsertPoint(j++,dCoords[i+1],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i]);//1
		points->InsertPoint(j++,dCoords[i+1],dCoords[uiQtyCoords+i+1],dCoords[2*uiQtyCoords+i]);//2
		points->InsertPoint(j++,dCoords[i],dCoords[uiQtyCoords+i+1],dCoords[2*uiQtyCoords+i]);//3
		points->InsertPoint(j++,dCoords[i],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i+1]);//4
		points->InsertPoint(j++,dCoords[i+1],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i+1]);//5
		points->InsertPoint(j++,dCoords[i+1],dCoords[uiQtyCoords+i+1],dCoords[2*uiQtyCoords+i+1]);//6
		points->InsertPoint(j++,dCoords[i],dCoords[uiQtyCoords+i+1],dCoords[2*uiQtyCoords+i+1]);//7
		poly->InsertNextCell(4);poly->InsertCellPoint(0+i*4);poly->InsertCellPoint(1+i*4);poly->InsertCellPoint(2+i*4);poly->InsertCellPoint(3+i*4);
		poly->InsertNextCell(4);poly->InsertCellPoint(4+i*4);poly->InsertCellPoint(5+i*4);poly->InsertCellPoint(6+i*4);poly->InsertCellPoint(7+i*4);
		poly->InsertNextCell(4);poly->InsertCellPoint(0+i*4);poly->InsertCellPoint(1+i*4);poly->InsertCellPoint(5+i*4);poly->InsertCellPoint(4+i*4);
		poly->InsertNextCell(4);poly->InsertCellPoint(1+i*4);poly->InsertCellPoint(2+i*4);poly->InsertCellPoint(6+i*4);poly->InsertCellPoint(5+i*4);
		poly->InsertNextCell(4);poly->InsertCellPoint(2+i*4);poly->InsertCellPoint(3+i*4);poly->InsertCellPoint(7+i*4);poly->InsertCellPoint(6+i*4);
		poly->InsertNextCell(4);poly->InsertCellPoint(3+i*4);poly->InsertCellPoint(0+i*4);poly->InsertCellPoint(4+i*4);poly->InsertCellPoint(7+i*4);
		//for (h=j-8;h<j;h++) scalars->InsertTuple1(h,h%8); // not in use jet
		//fprintf(stderr,".");
	}
	profile->SetPoints(points);
	profile->SetPolys(poly);
	//profile->GetPointData()->SetScalars(scalars);
	m_PolyDataCollection->AddInput(profile);
	Mapper->SetInput(profile);
	//Mapper->SetScalarRange(0,8);
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);
    ren->AddActor(Actor);
	ActorColl->AddItem(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	Mapper->Delete();
	//Actor->Delete();
	//scalars->Delete();
}


void VTKPrimitives::AddClosedPoly(double *dCoords, unsigned int uiQtyCoords, double *dExtrusionVector, double *dRGB, double dOpacity)
{  //complete
	unsigned int i=0;
	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkLinearExtrusionFilter *extrude = vtkLinearExtrusionFilter::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();
	for (i=0; i<uiQtyCoords;i++) points->InsertPoint(i,dCoords[i],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i]);
	poly->InsertNextCell(uiQtyCoords+1);
	for (i=0; i<uiQtyCoords ;i++) poly->InsertCellPoint(i);
	poly->InsertCellPoint(0);
	profile->SetPoints(points);
	profile->SetPolys(poly);
	vtkTriangleFilter *tf = vtkTriangleFilter::New();
	tf->SetInput(profile);

	extrude->SetInput(tf->GetOutput());
	extrude->SetExtrusionTypeToVectorExtrusion();
	extrude->SetVector(dExtrusionVector);
	extrude->CappingOn();

//	fprintf(stderr,"\n Vector: %f %f %f", dExtrusionVector[0],dExtrusionVector[1],dExtrusionVector[2]);

	m_PolyDataCollection->AddInput(extrude->GetOutput());
	Mapper->SetInput(extrude->GetOutput());
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);
    ren->AddActor(Actor);
	ActorColl->AddItem(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	tf->Delete();
	extrude->Delete();

	Mapper->Delete();
	//Actor->Delete();
}

void VTKPrimitives::AddLinePoly(double *dCoords, unsigned int uiQtyCoords, unsigned int LineWidth, double *dRGB, double dOpacity)
{ //complete
	unsigned int i=0;
	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();
	for (i=0; i<uiQtyCoords;i++) points->InsertPoint(i,dCoords[i],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i]);
	for (i=1; i<uiQtyCoords;i++)
	{
		poly->InsertNextCell(2);
		poly->InsertCellPoint(i-1);
		poly->InsertCellPoint(i);
	}

	profile->SetPoints(points);
	profile->SetLines(poly);

	m_PolyDataCollection->AddInput(profile);
	Mapper->SetInput(profile);
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);
	Actor->GetProperty()->SetLineWidth(LineWidth);

	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	Mapper->Delete();
	//Actor->Delete();
}

void VTKPrimitives::AddTubePoly(double *dCoords, unsigned int uiQtyCoords, double TubeRadius, double *dRGB, double dOpacity, int iResolution)
{ //complete
	unsigned int i=0;
	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();
	for (i=0; i<uiQtyCoords;i++) points->InsertPoint(i,dCoords[i],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i]);
	for (i=1; i<uiQtyCoords;i++)
	{
		poly->InsertNextCell(2);
		poly->InsertCellPoint(i-1);
		poly->InsertCellPoint(i);
	}

	profile->SetPoints(points);
	profile->SetLines(poly);

	vtkTubeFilter* m_profileTubes = vtkTubeFilter::New();
	m_profileTubes->SetNumberOfSides(iResolution);
	m_profileTubes->SetInput(profile);
	m_profileTubes->SetRadius(TubeRadius);

	Mapper->SetInputConnection( m_profileTubes->GetOutputPort());

	m_PolyDataCollection->AddInput(m_profileTubes->GetOutput());

	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);

	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	Mapper->Delete();
	//Actor->Delete();
}

void VTKPrimitives::AddCylinder2(const double *dAxisStart, const double* dAxisStop, float fRadius, double *dRGB, double dOpacity, int iResolution)
{
	double direction[3] = {dAxisStop[0]-dAxisStart[0],dAxisStop[1]-dAxisStart[1],dAxisStop[2]-dAxisStart[2]};
	AddCylinder(dAxisStart,direction,fRadius,dRGB,dOpacity,iResolution);
}

void VTKPrimitives::AddCylinder(const double *dCenter, const double *dExtrusionVector, float fRadius, double *dRGB, double dOpacity, int iResolution)
{
	double alpha=0,beta=0;
	double length=sqrt( dExtrusionVector[0]*dExtrusionVector[0]+dExtrusionVector[1]*dExtrusionVector[1]+dExtrusionVector[2]*dExtrusionVector[2] ) ;
	//if (length==0) { fprintf(stderr," Error Cylinder Extrusion Vector ist Zero.. Abort..."); exit(1); } 
	vtkCylinderSource *Source = vtkCylinderSource::New();
	vtkTransform *transform = vtkTransform::New();
	vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	Source->SetResolution(iResolution);
	Source->SetRadius(fRadius);
	Source->SetHeight(length);

	alpha=VectorAngel(dExtrusionVector[0],sqrt(dExtrusionVector[1]*dExtrusionVector[1]+dExtrusionVector[2]*dExtrusionVector[2]),0,0,1,0);
	beta=VectorAngel(0,dExtrusionVector[1],dExtrusionVector[2],0,1,0);
	// cout << alpha << "   "  << beta << endl;  //debuging
	if (dExtrusionVector[0]>0) alpha=-alpha;
	if (dExtrusionVector[2]<0) beta=-beta;
	transform->Translate(dExtrusionVector[0]/2+dCenter[0],dExtrusionVector[1]/2+dCenter[1],dExtrusionVector[2]/2+dCenter[2]);
	transform->RotateWXYZ(beta,1,0,0);
	transform->RotateWXYZ(alpha,0,0,1);

	transformFilter->SetInput(Source->GetOutput());
	transformFilter->SetTransform(transform);

	m_PolyDataCollection->AddInput(transformFilter->GetOutput());
	SourceMapper->SetInput(transformFilter->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);

	ActorColl->AddItem(SourceActor);
	ren->AddActor(SourceActor);

	Source->Delete();
	transform->Delete();
	transformFilter->Delete();
	SourceMapper->Delete();
	//SourceActor->Delete();
}

void VTKPrimitives::AddSphere(const double *dCenter, float fRadius, double *dRGB, double dOpacity, int iResolution)
{//complete
	vtkSphereSource *Source = vtkSphereSource::New();
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	double center[3]={dCenter[0],dCenter[1],dCenter[2]};
	Source->SetCenter(center);
	Source->SetRadius(fRadius);
	Source->SetPhiResolution(iResolution);
	Source->SetThetaResolution(iResolution);
	m_PolyDataCollection->AddInput(Source->GetOutput());
	SourceMapper->SetInput(Source->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);
	Source->Delete();
	SourceMapper->Delete();
	//SourceActor->Delete();
}

void VTKPrimitives::AddArrow(double *dStart, double *dEnd, double *dRGB, double dOpacity, int iResolution)
{
	double alpha=0,beta=0;
	double dvector[3]={dEnd[0]-dStart[0],dEnd[1]-dStart[1],dEnd[2]-dStart[2]};
	double length=sqrt( dvector[0]*dvector[0]+dvector[1]*dvector[1]+dvector[2]*dvector[2] ) ;
	if (length==0) { fprintf(stderr," Error Arrow Length ist Zero.. Abort..."); exit(1); } 
	vtkArrowSource *Source = vtkArrowSource::New();
	vtkTransform *transform = vtkTransform::New();
	vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	Source->SetTipResolution(iResolution);
	Source->SetShaftResolution(iResolution);
	Source->SetTipLength(0.15);
	Source->SetTipRadius(0.03);
	Source->SetShaftRadius(0.01);

	alpha=VectorAngel(dvector[0],sqrt(dvector[1]*dvector[1]+dvector[2]*dvector[2]),0,0,1,0);
	beta=VectorAngel(0,dvector[1],dvector[2],0,1,0);
	if (dvector[0]>0) alpha=-alpha;
	if (dvector[2]<0) beta=-beta;
//	fprintf(stderr,"aplha (um z): %f; beta (um x): %f",alpha,beta);
	transform->RotateWXYZ(beta,1,0,0);
	transform->RotateWXYZ(alpha,0,0,1);
	transform->RotateWXYZ(90,0,0,1);
	
	transformFilter->SetInput(Source->GetOutput());
	transformFilter->SetTransform(transform);

	m_PolyDataCollection->AddInput(transformFilter->GetOutput());
	SourceMapper->SetInput(transformFilter->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	SourceActor->SetScale(length);
	SourceActor->SetPosition(dStart);
	ActorColl->AddItem(SourceActor);
	ren->AddActor(SourceActor);

	Source->Delete();
	transform->Delete();
	transformFilter->Delete();
	SourceMapper->Delete();
	//SourceActor->Delete();
}

void VTKPrimitives::AddLabel(char *cText, double *dCoords, double *dRGB, double dOpacity, double dscale)
{
	vtkVectorText *text = vtkVectorText::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkFollower *Actor = vtkFollower::New();
	text->SetText(cText);
	m_PolyDataCollection->AddInput(text->GetOutput());
	Mapper->SetInput(text->GetOutput());
	Actor->SetMapper(Mapper);
	Actor->SetScale(dscale);
	Actor->SetCamera(ren->GetActiveCamera());
    Actor->GetProperty()->SetColor(dRGB); 
	Actor->GetProperty()->SetOpacity(dOpacity);
	Actor->SetPosition(dCoords);
    
	ren->AddActor(Actor);
	ActorColl->AddItem(Actor);

	//Actor->Delete();
	Mapper->Delete();
	text->Delete();
}

void VTKPrimitives::AddRotationalPoly(double *dCoords, unsigned int uiQtyCoords, double *fRotAxis, double *dRGB, double dOpacity, int iResolution)
{//complete, noch nicht ausgiebig getestet!!!
	unsigned int i=0;
	double start[3]={0,0,0},vector[3]={0,0,0},origin[3]={0,0,0},Footpoint[3]={0,0,0},Basepoint[3]={0,0,0},point[3]={0,0,0};
	double *radius,*level;

	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkRotationalExtrusionFilter *extrude = vtkRotationalExtrusionFilter::New();
	vtkTransform *transform = vtkTransform::New();
	vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();

	vector[0]=fRotAxis[1]-fRotAxis[0];
	vector[1]=fRotAxis[3]-fRotAxis[2];
	vector[2]=fRotAxis[5]-fRotAxis[4];
	start[0]=fRotAxis[0];
	start[1]=fRotAxis[2];
	start[2]=fRotAxis[4];

	DistancePointLine(origin,start,vector,Footpoint);

	//debugging
//	for (i=0; i<6; i++) fprintf(stderr,"\n Achse: [%i]: %f",i,fRotAxis[i]);
//	fprintf(stderr,"\n Richtungsvektor: %f %f %f",vector[0],vector[1],vector[2]);
//	fprintf(stderr,"\n Fusspunkt: %f %f %f",Footpoint[0],Footpoint[1],Footpoint[2]);
//	fprintf(stderr,"\n Start: %f %f %f",start[0],start[1],start[2]);

	radius=(double *)calloc(uiQtyCoords,sizeof(double));
	level=(double *)calloc(uiQtyCoords,sizeof(double));

	poly->InsertNextCell(uiQtyCoords+1);

	for (i=0;i<uiQtyCoords;i++)
	{
		for (int j=0; j<3; j++) point[j]=dCoords[uiQtyCoords*j+i];
		radius[i]=DistancePointLine(point,start,vector,Basepoint);
		level[i]=DistancePointPoint(Basepoint,Footpoint);
		points->InsertPoint(i,radius[i],0,level[i]);
		poly->InsertCellPoint(i);
	}
	poly->InsertCellPoint(0);
	profile->SetPoints(points);
	profile->SetLines(poly);
	extrude->SetInput(profile);
	extrude->SetResolution(iResolution);
	extrude->SetAngle(90.0);


	double alpha=VectorAngel(vector[0],sqrt(vector[1]*vector[1]+vector[2]*vector[2]),0,0,1,0);
	double beta=VectorAngel(0,vector[1],vector[2],0,1,0);
	if (fRotAxis[0]>0) alpha=-alpha;
	if (fRotAxis[2]<0) beta=-beta;
	fprintf(stderr," alpha: %f  beta: %f \n",alpha,beta);
	transform->Translate(Footpoint);
//	transform->RotateX(-beta);
//	transform->RotateZ(alpha);
//	transform->RotateWXYZ(-90,1,0,0);

	transform->RotateWXYZ(beta,1,0,0);
	transform->RotateWXYZ(alpha,0,0,1);
	transform->RotateWXYZ(-90,1,0,0);

	transformFilter->SetInput(extrude->GetOutput());
	transformFilter->SetTransform(transform);

	m_PolyDataCollection->AddInput(transformFilter->GetOutput());
	Mapper->SetInput(transformFilter->GetOutput());
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);

	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	free(level);
	free(radius);
	points->Delete();
	poly->Delete();
	profile->Delete();
	extrude->Delete();
	transform->Delete();
	transformFilter->Delete();
	Mapper->Delete();
	//Actor->Delete();
}

void VTKPrimitives::AddRotationalSolid(double *dPoint, double fRadius, double *fRotAxis, double *dRGB, double dOpacity, int iResolution)
{
	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkRotationalExtrusionFilter *extrude = vtkRotationalExtrusionFilter::New();
	vtkTransform *transform = vtkTransform::New();
	vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();

	double vector[3]={0,0,0},start[3]={0,0,0},Footpoint[3]={0,0,0};
//	for (int i=0; i<6; i++) fprintf(stderr,"\n fRotAxis[%i]: %f ",i, fRotAxis[i]); //debugging
	vector[0]=fRotAxis[1]-fRotAxis[0];
	vector[1]=fRotAxis[3]-fRotAxis[2];
	vector[2]=fRotAxis[5]-fRotAxis[4];
	start[0]=fRotAxis[0];
	start[1]=fRotAxis[2];
	start[2]=fRotAxis[4];
//	for (int i=0; i<3; i++) fprintf(stderr,"\n Coords[%i]: %f start[%i]: %f vector[%i]: %f ",i,dCoords[i],i,start[i],i,vector[i]); //debugging
	double dSolidRadius=DistancePointLine(dPoint,start,vector,Footpoint);

//	fprintf(stderr,"\n dSolidRadius: %f Footpoint: %f %f %f\n",dSolidRadius,Footpoint[0],Footpoint[1],Footpoint[2]); //debugging

	poly->InsertNextCell(iResolution+1);
	for (int i=0; i<iResolution; i++) 
	{
		points->InsertPoint(i,dSolidRadius+fRadius*cos(i*2*PI/iResolution),0,fRadius*sin(i*2*PI/iResolution));
		poly->InsertCellPoint(i);
	}
	poly->InsertCellPoint(0);
	profile->SetPoints(points);
	profile->SetLines(poly);

	extrude->SetInput(profile);
	extrude->SetResolution(iResolution);
	extrude->SetAngle(360.0);



	double alpha=VectorAngel(vector[0],sqrt(vector[1]*vector[1]+vector[2]*vector[2]),0,0,1,0);
	double beta=VectorAngel(0,vector[1],vector[2],0,1,0);
	if (vector[0]>0) alpha=-alpha;
	if (vector[2]<0) beta=-beta;
	transform->Translate(Footpoint);
	transform->RotateWXYZ(beta,1,0,0);
	transform->RotateWXYZ(alpha,0,0,1);
	transform->RotateWXYZ(-90,1,0,0);

	transformFilter->SetInput(extrude->GetOutput());
	transformFilter->SetTransform(transform);

	m_PolyDataCollection->AddInput(transformFilter->GetOutput());
	Mapper->SetInput(transformFilter->GetOutput());
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);

	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	extrude->Delete();
	transform->Delete();
	transformFilter->Delete();
	Mapper->Delete();
	//Actor->Delete();
}


void VTKPrimitives::AddSurface(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity)
{//complete
	unsigned int i=0;
	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();
	for (i=0; i<uiQtyCoords;i++) points->InsertPoint(i,dCoords[3*i],dCoords[3*i+1],dCoords[3*i+2]);
	for (i=0; i<uiQtyCoords;)
	{
		poly->InsertNextCell(3);
		poly->InsertCellPoint(i++);
		poly->InsertCellPoint(i++);
		poly->InsertCellPoint(i++);
	}
	profile->SetPoints(points);
	profile->SetPolys(poly);
	m_PolyDataCollection->AddInput(profile);
	Mapper->SetInput(profile);
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);


	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	Mapper->Delete();
	//Actor->Delete();

}



void VTKPrimitives::AddSTLObject(char *Filename, double *dCenter, double *dRGB, double dOpacity)
{ //complete??
	vtkSTLReader *part = vtkSTLReader::New();
	part->SetFileName(Filename);
	vtkPolyDataMapper *partMapper = vtkPolyDataMapper::New();
	m_PolyDataCollection->AddInput(part->GetOutput());
	partMapper->SetInput(part->GetOutput());
	partMapper->ScalarVisibilityOff();
	vtkActor *partActor = vtkActor::New();
	partActor->SetMapper(partMapper);
	partActor->GetProperty()->SetColor(dRGB);
	partActor->GetProperty()->SetOpacity(dOpacity);
	partActor->SetPosition(dCenter);
	ren->AddActor(partActor);
	ActorColl->AddItem(partActor);
	part->Delete();
	partMapper->Delete();
	//partActor->Delete();
}

void VTKPrimitives::SetOpacity2All(double opacity)
{
	ActorColl->InitTraversal();
	vtkActor* act=NULL;
	while ((act=ActorColl->GetNextActor()))
	{
		act->GetProperty()->SetOpacity(opacity);
	}
}

double VTKPrimitives::VectorAngel(double dV1_1, double dV1_2, double dV1_3, double dV2_1, double dV2_2, double dV2_3)
{
	double angel=0,dV1L,dV2L;
	double scalarP;
	dV1L=sqrt(dV1_1*dV1_1+dV1_2*dV1_2+dV1_3*dV1_3);
	dV2L=sqrt(dV2_1*dV2_1+dV2_2*dV2_2+dV2_3*dV2_3);
	scalarP=dV1_1*dV2_1+dV1_2*dV2_2+dV1_3*dV2_3;
	if ((dV1L*dV2L)==0) return 0;
	angel=scalarP/(dV1L*dV2L);
	if (angel>1) angel=0.0;
	else if (angel<-1) angel=180.0;
	else angel=acos(angel)*180/PI;
	return angel;
}


double VTKPrimitives::DistancePointLine(double *dpoint,double *dstart,double *dvector, double *dFootpoint)
{
	double dpos=0;
	dpos= ( (dpoint[0]-dstart[0])*dvector[0]+(dpoint[1]-dstart[1])*dvector[1]+(dpoint[2]-dstart[2])*dvector[2] ) /(dvector[0]*dvector[0]+dvector[1]*dvector[1]+dvector[2]*dvector[2]);
	for (int i=0; i<3; i++) dFootpoint[i]=dstart[i]+dpos*dvector[i];
	return sqrt( (dpoint[0]-dFootpoint[0])*(dpoint[0]-dFootpoint[0]) + (dpoint[1]-dFootpoint[1])*(dpoint[1]-dFootpoint[1]) + (dpoint[2]-dFootpoint[2])*(dpoint[2]-dFootpoint[2]) );
}

double VTKPrimitives::DistancePointPoint(double *dpoint1, double *dpoint2)
{
	return sqrt( (dpoint1[0]-dpoint2[0])*(dpoint1[0]-dpoint2[0]) + (dpoint1[1]-dpoint2[1])*(dpoint1[1]-dpoint2[1]) + (dpoint1[2]-dpoint2[2])*(dpoint1[2]-dpoint2[2]) );
}

double* VTKPrimitives::TransformCylindricalCoords(double* in, double* out, unsigned int nrPoints)
{
	unsigned int i,j,k;
	for (unsigned int n=0;n<nrPoints;++n)
	{
		i = n;
		j = nrPoints + n;
		k = 2*nrPoints + n;

		out[i] = in[i] * cos(in[j]);
		out[j] = in[i] * sin(in[j]);
		out[k] = in[k];
	}
	return out;
}
