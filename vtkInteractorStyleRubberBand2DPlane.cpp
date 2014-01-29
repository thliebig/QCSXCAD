/*
*	Copyright (C) 2010-2013 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "vtkInteractorStyleRubberBand2DPlane.h"

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

vtkStandardNewMacro(vtkInteractorStyleRubberBand2DPlane);

vtkInteractorStyleRubberBand2DPlane::vtkInteractorStyleRubberBand2DPlane() : vtkInteractorStyleRubberBand2D()
{
}

void vtkInteractorStyleRubberBand2DPlane::OnMouseMove()
{
	if (this->Interaction == PANNING)
	{
		vtkRenderWindowInteractor* rwi = this->GetInteractor();
		int lastPt[] = {0, 0};
		rwi->GetLastEventPosition(lastPt);
		int curPt[] = {0, 0};
		rwi->GetEventPosition(curPt);

		vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
		double lastScale = 2.0 * camera->GetParallelScale() / this->CurrentRenderer->GetSize()[1];
		double lastFocalPt[] = {0, 0, 0};
		camera->GetFocalPoint(lastFocalPt);
		double lastPos[] = {0, 0, 0};
		camera->GetPosition(lastPos);

		double dop[3];
		camera->GetDirectionOfProjection(dop);

		int n_dir=-1;
		for (int n=0;n<3;++n)
			if ((dop[n]!=0) && (dop[(n+1)%2]==0) && (dop[(n+2)%3]==0))
				n_dir = n;
		if (n_dir<0)
		{
			cerr << __func__ << ": Error, no view in x-, y- or z-direction, skipping panning!";
			return;
		}

		double delta[] = {0, 0, 0};
		delta[(n_dir+1)%3] = -lastScale*(curPt[0] - lastPt[0]);
		delta[(n_dir+2)%3] = -lastScale*(curPt[1] - lastPt[1]);
		camera->SetFocalPoint(lastFocalPt[0] + delta[0], lastFocalPt[1] + delta[1], lastFocalPt[2] + delta[2]);
		camera->SetPosition(lastPos[0] + delta[0], lastPos[1] + delta[1], lastPos[2] + delta[2]);
		this->InvokeEvent(vtkCommand::InteractionEvent);
		rwi->Render();
	}
	else
		vtkInteractorStyleRubberBand2D::OnMouseMove();
}
