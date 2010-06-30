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

class QVTKWidget;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkAxesActor;
class vtkActor;
class ContinuousStructure;
class VTKPrimitives;
class vtkObject;

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

	void SetUpdateMode(bool mode) {AllowUpdate=mode;};

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
	void RenderGeometry();

	void ExportView2Image();

protected:
	typedef struct
	{
		VTKPrimitives* VTKProp;
		unsigned int uID;
	} VTKLayerStruct;

	QVTKWidget *VTKWidget;

	bool AllowUpdate;

	vtkRenderer *ren;

	vtkAxesActor* Axes;

	ContinuousStructure* clCS;
	QVector<VTKLayerStruct> LayerPrimitives;

	vtkActor* ActorGridPlane[3];

	int iResolution;

	struct KeyPressData
	{
		vtkActor **GridPlanes;
		vtkRenderer *ren;
		vtkRenderWindowInteractor * iren;
	};

	static void KeyPress(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);
};

#endif //_QVTKSTRUCTURE_H_
