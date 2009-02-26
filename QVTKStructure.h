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
