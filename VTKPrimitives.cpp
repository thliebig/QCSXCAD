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
#include "vtkPLYWriter.h"


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

void VTKPrimitives::AddCube(const double *start, const double *stop, double *dRGB, double dOpacity, const double* tf_matrix)
{
	double coords[6] = {start[0],stop[0],start[1],stop[1],start[2],stop[2]};
	double help;
	int dim=0;
	//swap start stop if start>stop
	for (int n=0;n<3;++n)
	{
		if (coords[2*n]>coords[2*n+1])
		{
			help=coords[2*n+1];
			coords[2*n+1]=coords[2*n];
			coords[2*n]=help;
		}
		if (coords[2*n]!=coords[2*n+1])
			++dim;
	}
	if (dim==0)
	{
		cerr << "VTKPrimitives::AddCube: Warning, can't draw a Point Box... skipping" << endl;
		return;
	}
	if (dim==1)
		return AddLinePoly(coords,2,1,dRGB,dOpacity);
	AddCube(coords,dRGB,dOpacity,tf_matrix);
}

void VTKPrimitives::AddCube(double *dCoords, double *dRGB, double dOpacity, const double* tf_matrix)
{
	//create a simple cartesian cube...
	vtkCubeSource *Source = vtkCubeSource::New();
	Source->SetBounds(dCoords);

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(Source->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	SourceMapper->SetInput(filter->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);
	Source->Delete();
	filter->Delete();
	vtrans->Delete();
	SourceMapper->Delete();
}

void VTKPrimitives::AddCylindricalCube(const double *start, const double *stop, double *dRGB, double dOpacity, const double* tf_matrix)
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
	AddCylindricalCube(coords,dRGB,dOpacity,tf_matrix);
}


void VTKPrimitives::AddCylindricalCube(double *dCoords, double *dRGB, double dOpacity, const double* tf_matrix)
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
		else //line in a-direction
			return AddLinePoly(dCoords,2,1,dRGB,dOpacity);

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
			return AddLinePoly(dCoords,2,1,dRGB,dOpacity);
		}
		if (dCoords[4]==dCoords[5]) //line in r-direction
		{
			return AddLinePoly(dCoords,2,1,dRGB,dOpacity);
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

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(PDFilter->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	SourceMapper->SetInput(filter->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);

	if (PDSource)
		PDSource->Delete();
	PDFilter->Delete();
	filter->Delete();
	vtrans->Delete();
}

void VTKPrimitives::AddPlane(double *dOrigin, double* dP1, double* dP2, double *dRGB, double dOpacity, const double* tf_matrix)
{
	double out[3];
	vtkPlaneSource *Source = vtkPlaneSource::New();
	Source->SetOrigin(TransformCylindricalCoords(dOrigin,out));
	Source->SetPoint1(TransformCylindricalCoords(dP1,out));
	Source->SetPoint2(TransformCylindricalCoords(dP2,out));

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(Source->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	SourceMapper->SetInput(filter->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);
	m_PolyDataCollection->AddInput(filter->GetOutput());
	Source->Delete();
	SourceMapper->Delete();
	filter->Delete();
	vtrans->Delete();
}


void VTKPrimitives::AddDisc(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity, const double* tf_matrix)
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

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(profile);
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	//profile->GetPointData()->SetScalars(scalars);
	m_PolyDataCollection->AddInput(filter->GetOutput());
	Mapper->SetInput(filter->GetOutput());
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
	filter->Delete();
	vtrans->Delete();
}


void VTKPrimitives::AddClosedPoly(double *dCoords, unsigned int uiQtyCoords, double *dExtrusionVector, double *dRGB, double dOpacity, const double* tf_matrix)
{  //complete
	unsigned int i=0;
	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkLinearExtrusionFilter *extrude = vtkLinearExtrusionFilter::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();
	for (i=0; i<uiQtyCoords;i++)
		points->InsertPoint(i,dCoords[i],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i]);
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

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(extrude->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	Mapper->SetInput(filter->GetOutput());
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
	filter->Delete();
	vtrans->Delete();
}

void VTKPrimitives::AddLinePoly(const double *dCoords, unsigned int uiQtyCoords, unsigned int LineWidth, double *dRGB, double dOpacity, const double* tf_matrix)
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

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(profile);
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	Mapper->SetInput(filter->GetOutput());
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
	filter->Delete();
	vtrans->Delete();
}

void VTKPrimitives::AddTubePoly(const double *dCoords, unsigned int uiQtyCoords, double TubeRadius, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix)
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


	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(m_profileTubes->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	Mapper->SetInput( filter->GetOutput());
	m_PolyDataCollection->AddInput(filter->GetOutput());

	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);

	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	Mapper->Delete();
	filter->Delete();
	vtrans->Delete();
}

void VTKPrimitives::AddCylinder2(const double *dAxisStart, const double* dAxisStop, float fRadius, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix)
{
	double direction[3] = {dAxisStop[0]-dAxisStart[0],dAxisStop[1]-dAxisStart[1],dAxisStop[2]-dAxisStart[2]};
	AddCylinder(dAxisStart,direction,fRadius,dRGB,dOpacity,iResolution,tf_matrix);
}

void VTKPrimitives::AddCylinder(const double *dCenter, const double *dExtrusionVector, float fRadius, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix)
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

	transform->PostMultiply();
	if (tf_matrix)
		transform->Concatenate(tf_matrix);

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
}

void VTKPrimitives::AddSphere(const double *dCenter, float fRadius, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix)
{//complete
	vtkSphereSource *Source = vtkSphereSource::New();
	vtkPolyDataMapper *SourceMapper = vtkPolyDataMapper::New();
	vtkActor *SourceActor = vtkActor::New();
	double center[3]={dCenter[0],dCenter[1],dCenter[2]};
	Source->SetCenter(center);
	Source->SetRadius(fRadius);
	Source->SetPhiResolution(iResolution);
	Source->SetThetaResolution(iResolution);

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(Source->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	SourceMapper->SetInput(filter->GetOutput());
	SourceActor->SetMapper(SourceMapper);
	SourceActor->GetProperty()->SetColor(dRGB);
	SourceActor->GetProperty()->SetOpacity(dOpacity);
	ren->AddActor(SourceActor);
	ActorColl->AddItem(SourceActor);
	Source->Delete();
	SourceMapper->Delete();
	filter->Delete();
	vtrans->Delete();
}

void VTKPrimitives::AddArrow(double *dStart, double *dEnd, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix)
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

	transform->PostMultiply();
	if (tf_matrix)
		transform->Concatenate(tf_matrix);
	
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
}

void VTKPrimitives::AddLabel(char *cText, double *dCoords, double *dRGB, double dOpacity, double dscale, const double* tf_matrix)
{
	vtkVectorText *text = vtkVectorText::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkFollower *Actor = vtkFollower::New();
	text->SetText(cText);

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(text->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	Mapper->SetInput(filter->GetOutput());
	Actor->SetMapper(Mapper);
	Actor->SetScale(dscale);
	Actor->SetCamera(ren->GetActiveCamera());
    Actor->GetProperty()->SetColor(dRGB); 
	Actor->GetProperty()->SetOpacity(dOpacity);
	Actor->SetPosition(dCoords);
    
	ren->AddActor(Actor);
	ActorColl->AddItem(Actor);

	Mapper->Delete();
	text->Delete();
	filter->Delete();
	vtrans->Delete();
}

void VTKPrimitives::AddRotationalPoly(const double *dCoords, unsigned int uiQtyCoords, const double *fRotAxis, double StartStopAngle[2], double *dRGB, double dOpacity, int iResolution, const double* tf_matrix)
{
	unsigned int i=0;
	double start[3]={0,0,0},vector[3]={0,0,0};

	vtkPoints *points = vtkPoints::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPolyData *profile = vtkPolyData::New();
	vtkRotationalExtrusionFilter *extrude = vtkRotationalExtrusionFilter::New();
	vtkTransform *transform = vtkTransform::New();
	vtkTransform *InvTransform = vtkTransform::New();
	vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();
	vtkTransformPolyDataFilter *InvTransformFilter = vtkTransformPolyDataFilter::New();
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
	vtkActor *Actor = vtkActor::New();

	vector[0]=fRotAxis[1]-fRotAxis[0];
	vector[1]=fRotAxis[3]-fRotAxis[2];
	vector[2]=fRotAxis[5]-fRotAxis[4];
	double vec_len = sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2]);
	double vec_unit[3]={vector[0]/vec_len,vector[1]/vec_len,vector[2]/vec_len};
	start[0]=fRotAxis[0];
	start[1]=fRotAxis[2];
	start[2]=fRotAxis[4];

	double d = sqrt(vec_unit[1]*vec_unit[1]+vec_unit[2]*vec_unit[2]);
	double alpha = atan2(vec_unit[1],vec_unit[2])/acos(-1)*180;
	double beta = atan2(-vec_unit[0],d)/acos(-1)*180;

	for (i=0; i<uiQtyCoords;i++)
		points->InsertPoint(i,dCoords[i],dCoords[uiQtyCoords+i],dCoords[2*uiQtyCoords+i]);
	poly->InsertNextCell(uiQtyCoords+1);
	for (i=0; i<uiQtyCoords ;i++) poly->InsertCellPoint(i);
	poly->InsertCellPoint(0);
	profile->SetPoints(points);
	profile->SetPolys(poly);
	vtkTriangleFilter *tf = vtkTriangleFilter::New();
	tf->SetInput(profile);

	InvTransform->Translate(-start[0],-start[1],-start[2]);
	InvTransform->RotateX(alpha);
	InvTransform->RotateY(beta);

	InvTransformFilter->SetInput(tf->GetOutput());
	InvTransformFilter->SetTransform(InvTransform);

	extrude->SetInput(InvTransformFilter->GetOutput());
	extrude->SetResolution(iResolution);
	extrude->SetAngle(StartStopAngle[1]-StartStopAngle[0]);

	transform->RotateZ(-StartStopAngle[0]);

	transform->Translate(-start[0],-start[1],-start[2]);
	transform->RotateX(alpha);
	transform->RotateY(beta);
	transform->Inverse();

	transform->PostMultiply();
	if (tf_matrix)
		transform->Concatenate(tf_matrix);

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
	InvTransform->Delete();
	InvTransformFilter->Delete();
	Mapper->Delete();
}

void VTKPrimitives::AddRotationalSolid(const double *dPoint, double fRadius, const double *fRotAxis, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix)
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

	transform->PostMultiply();
	if (tf_matrix)
		transform->Concatenate(tf_matrix);

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
}


void VTKPrimitives::AddSurface(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity, const double* tf_matrix)
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

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(profile);
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	Mapper->SetInput(filter->GetOutput());
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);


	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	points->Delete();
	poly->Delete();
	profile->Delete();
	Mapper->Delete();
	filter->Delete();
	vtrans->Delete();
}



void VTKPrimitives::AddSTLObject(const char *Filename, double *dCenter, double *dRGB, double dOpacity, const double* tf_matrix)
{ //complete??
	vtkSTLReader *part = vtkSTLReader::New();
	part->SetFileName(Filename);
	vtkPolyDataMapper *partMapper = vtkPolyDataMapper::New();

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(part->GetOutput());
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	partMapper->SetInput(filter->GetOutput());
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
	filter->Delete();
	vtrans->Delete();
}

void VTKPrimitives::AddPolyData(vtkPolyData* polydata, double *dRGB, double dOpacity, const double* tf_matrix)
{
	vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();

	vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
	vtkTransform* vtrans = vtkTransform::New();
	filter->SetInput(polydata);
	if (tf_matrix)
		vtrans->SetMatrix(tf_matrix);
	filter->SetTransform(vtrans);

	m_PolyDataCollection->AddInput(filter->GetOutput());
	Mapper->SetInput(filter->GetOutput());
	vtkActor *Actor = vtkActor::New();
	Actor->SetMapper(Mapper);
	Actor->GetProperty()->SetColor(dRGB);
	Actor->GetProperty()->SetOpacity(dOpacity);


	ActorColl->AddItem(Actor);
	ren->AddActor(Actor);

	polydata->Delete();
	Mapper->Delete();
	filter->Delete();
	vtrans->Delete();
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

void VTKPrimitives::WritePolyData2File(const char* filename, double scale)
{
	cout << "VTKPrimitives::WritePolyData2File: Dump to vtk file: " << filename << " -- Using scale: " << scale << endl;
	vtkXMLPolyDataWriter* writer  = vtkXMLPolyDataWriter::New();
	writer->SetFileName(filename);

	if (scale==1.0)
	{
		writer->SetInput(m_PolyDataCollection->GetOutput());
		writer->Write();
	}
	else
	{
		vtkTransform *transform = vtkTransform::New();
		vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();

		transformFilter->SetInput(m_PolyDataCollection->GetOutput());
		transform->Scale(scale,scale,scale);
		transformFilter->SetTransform(transform);

		writer->SetInput(transformFilter->GetOutput());
		writer->Write();

		transform->Delete();
		transformFilter->Delete();
	}

	writer->Delete();
}

void VTKPrimitives::WritePolyData2STL(const char* filename, double scale)
{
	cout << "VTKPrimitives::WritePolyData2STL: Dump to stl file: " << filename << " -- Using scale: " << scale << endl;

	vtkTriangleFilter* filter = vtkTriangleFilter::New();

	filter->SetInput(m_PolyDataCollection->GetOutput());

	vtkSTLWriter* writer  = vtkSTLWriter::New();
	writer->SetFileName(filename);

	if (scale==1.0)
	{
		writer->SetInput(filter->GetOutput());
		writer->Write();
	}
	else
	{
		vtkTransform *transform = vtkTransform::New();
		vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();

		transformFilter->SetInput(filter->GetOutput());
		transform->Scale(scale,scale,scale);
		transformFilter->SetTransform(transform);

		writer->SetInput(transformFilter->GetOutput());
		writer->Write();

		transform->Delete();
		transformFilter->Delete();
	}

	writer->Delete();
}

void VTKPrimitives::WritePolyData2PLY(const char* filename, double scale)
{
	cout << "VTKPrimitives::WritePolyData2PLY: Dump to ply file: " << filename << " -- Using scale: " << scale << endl;

	vtkTriangleFilter* filter = vtkTriangleFilter::New();

	filter->SetInput(m_PolyDataCollection->GetOutput());

	vtkPLYWriter* writer  = vtkPLYWriter::New();
	writer->SetFileName(filename);

	if (scale==1.0)
	{
		writer->SetInput(filter->GetOutput());
		writer->Write();
	}
	else
	{
		vtkTransform *transform = vtkTransform::New();
		vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();

		transformFilter->SetInput(filter->GetOutput());
		transform->Scale(scale,scale,scale);
		transformFilter->SetTransform(transform);

		writer->SetInput(transformFilter->GetOutput());
		writer->Write();

		transform->Delete();
		transformFilter->Delete();
	}

	writer->Delete();
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

double VTKPrimitives::DistancePointLine(const double *dpoint, const double *dstart, const double *dvector, double *dFootpoint)
{
	double dpos=0;
	dpos= ( (dpoint[0]-dstart[0])*dvector[0]+(dpoint[1]-dstart[1])*dvector[1]+(dpoint[2]-dstart[2])*dvector[2] ) /(dvector[0]*dvector[0]+dvector[1]*dvector[1]+dvector[2]*dvector[2]);
	for (int i=0; i<3; i++) dFootpoint[i]=dstart[i]+dpos*dvector[i];
	return sqrt( (dpoint[0]-dFootpoint[0])*(dpoint[0]-dFootpoint[0]) + (dpoint[1]-dFootpoint[1])*(dpoint[1]-dFootpoint[1]) + (dpoint[2]-dFootpoint[2])*(dpoint[2]-dFootpoint[2]) );
}

double VTKPrimitives::DistancePointPoint(const double *dpoint1, const double *dpoint2)
{
	return sqrt( (dpoint1[0]-dpoint2[0])*(dpoint1[0]-dpoint2[0]) + (dpoint1[1]-dpoint2[1])*(dpoint1[1]-dpoint2[1]) + (dpoint1[2]-dpoint2[2])*(dpoint1[2]-dpoint2[2]) );
}

double* VTKPrimitives::TransformCylindricalCoords(const double* in, double* out, unsigned int nrPoints)
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
