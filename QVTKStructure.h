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

#ifndef _QVTKSTRUCTURE_H_
#define _QVTKSTRUCTURE_H_

#include <QtGui>

#include "vtkVersion.h"
#if VTK_MAJOR_VERSION>=9
  class QVTKOpenGLStereoWidget;
#elif VTK_MAJOR_VERSION==8
  class QVTKOpenGLWidget;
#else
  class QVTKWidget;
#endif
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkAxesActor;
class vtkActor;
class ContinuousStructure;
class VTKPrimitives;
class vtkObject;
class vtkRectilinearGrid;
class vtkStructuredGrid;

class QVTKStructure : public QObject
{
	Q_OBJECT
public:
	QVTKStructure();
	virtual ~QVTKStructure();

	QWidget* GetVTKWidget() {return (QWidget *)VTKWidget;};

	void AddAxes();

	void SetGeometry(ContinuousStructure *CS);
	void clear();

	void SetBackgroundColor(int r, int g, int b);

	//! Set the update mode to false if this widget should not be rendered e.g. because it is hidden
	void SetUpdateMode(bool mode) {AllowUpdate=mode;}

	//Callback
	void SetCallback(vtkRenderWindowInteractor *iren);

public slots:
	void SetGridOpacity(int val);
	void ResetView();
	void setXY();
	void setYZ();
	void setZX();
	void SetPropOpacity(unsigned int uiID, int val);
	void RenderGrid();
	void RenderGridX(int plane_pos);
	void RenderGridY(int plane_pos);
	void RenderGridZ(int plane_pos);
	void RenderGeometry();

	//! Enable/Disable 3D parallel projection
	void SetParallelProjection(bool val, bool render=true);

	//! Enable/Disable 2D interaction style
	void Set2DInteractionStyle(bool val, bool render=true);

	void SaveCamData();
	void RestoreCamData(bool render);

	void RenderDiscMaterialModel();

	//! Export the current view to an image file (currently only png)
	void ExportView2Image();

	void ExportProperty2PolyDataVTK(unsigned int uiID, QString filename, double scale = 1.0);
	void ExportProperty2STL(unsigned int uiID, QString filename, double scale = 1.0);
	void ExportProperty2PLY(unsigned int uiID, QString filename, double scale = 1.0);

protected slots:
	void RenderGridDir(int dir, unsigned int plane_pos);

protected:
	typedef struct
	{
		VTKPrimitives* VTKProp;
		unsigned int uID;
	} VTKLayerStruct;

#if VTK_MAJOR_VERSION>=9
	QVTKOpenGLStereoWidget *VTKWidget;
#elif VTK_MAJOR_VERSION==8
	QVTKOpenGLWidget *VTKWidget;
#else
	QVTKWidget *VTKWidget;
#endif

	//set to false if this widget is hidden
	bool AllowUpdate;

	vtkRenderer *ren;

	vtkAxesActor* Axes;

	ContinuousStructure* clCS;
	QVector<VTKLayerStruct> LayerPrimitives;

	vtkRectilinearGrid *m_Rect_Grid;
	vtkStructuredGrid *m_Struct_Grid;
	vtkActor* ActorGridPlane[3];
	int GridOpacity; //remember the grid opacity

	int iResolution;

	typedef struct
	{
		VTKPrimitives* vtk_model;
		unsigned int uID;
	} VTKDiscModel;
	QVector<VTKDiscModel> m_DiscMatModels;

	typedef struct
	{
		double pos[3];
		double focalPoint[3];
		double viewUp[3];
		double viewAngle;
	} CamData;
	CamData* m_CamData;

	struct KeyPressData
	{
		vtkActor **GridPlanes;
		vtkRenderer *ren;
		vtkRenderWindowInteractor * iren;
	};

	static void KeyPress(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);
};

#endif //_QVTKSTRUCTURE_H_
