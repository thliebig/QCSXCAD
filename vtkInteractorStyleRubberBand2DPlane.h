#ifndef VTKINTERACTORSTYLERUBBERBAND2DPLANE_H
#define VTKINTERACTORSTYLERUBBERBAND2DPLANE_H

#include "vtkInteractorStyleRubberBand2D.h"

class vtkInteractorStyleRubberBand2DPlane : public vtkInteractorStyleRubberBand2D
{
public:
	static vtkInteractorStyleRubberBand2DPlane *New();
	vtkTypeMacro(vtkInteractorStyleRubberBand2DPlane, vtkInteractorStyle);

	virtual void OnMouseMove();

protected:
	vtkInteractorStyleRubberBand2DPlane();
};

#endif // VTKINTERACTORSTYLERUBBERBAND2DPLANE_H
