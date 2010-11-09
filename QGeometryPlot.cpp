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

#include "QGeometryPlot.h"
#include "QCSXCAD.h"

QGeometryPlot::QGeometryPlot(QCSXCAD* CS) : QWidget()
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	direct=2;
	clCS=CS;
	spacer=10;
	GridOpacity=0;
	for (unsigned int i=0;i<6;++i) DrawArea[i]=(2.0*(double)(i%2)-1.0)*1000.0;
	setCursor(Qt::CrossCursor);
	setMouseTracking(true);
	statBar=NULL;
	bArrow=false;
}

QGeometryPlot::~QGeometryPlot()
{
}

void QGeometryPlot::setXY()
{
	direct=2;
	Reset();
}

void QGeometryPlot::setYZ()
{
	direct=0;
	Reset();
}

void QGeometryPlot::setZX()
{
	direct=1;
	Reset();
}

void QGeometryPlot::setDrawArea(double* area)
{
	if ((area[0]==area[1])  && (area[2]==area[3])  && (area[4]==area[5])) return;
	for (unsigned int i=0;i<6;++i) DrawArea[i]=area[i];
	update();
}

void QGeometryPlot::setGridOpacity(int val)
{
	GridOpacity=val;
	update();
}

void QGeometryPlot::Reset()
{
	bArrow = false;
	update();
}

void QGeometryPlot::paintEvent(QPaintEvent * /* event */)
{
	CSPrimitives* prim=NULL;
	CSProperties* prop=NULL;
	QPainter painter(this);

	int x=(direct+1)%3;
	int y=(direct+2)%3;

	double widthX = (DrawArea[2*x+1]-DrawArea[2*x]);
	double widthY = (DrawArea[2*y+1]-DrawArea[2*y]);

	double factorX=widthX/(double)(width()-2*spacer);
	double factorY=widthY/(double)(height()-2*spacer);
	factor=factorX;
	if (factor<factorY) factor=factorY;
//	factor*=1.05;
	offsetX=DrawArea[2*x]+(widthX-(double)(width())*factor)/2;
	offsetY=DrawArea[2*y]+(widthY-(double)(height())*factor)/2;;
	double help;

	if (GridOpacity>0)
	{
		QPen pen(QColor(0,0,0,GridOpacity));
		painter.setPen(pen);

		CSRectGrid* grid = clCS->GetGrid();
		int iX=0;
		int iY1=height()-(int)((grid->GetLine(y,0)-offsetY)/factor);
		int iY2=height()-(int)((grid->GetLine(y,grid->GetQtyLines(y)-1)-offsetY)/factor);
		for (size_t n=0;n<grid->GetQtyLines(x);++n)
		{
			iX=(int)((grid->GetLine(x,n)-offsetX)/factor);
			painter.drawLine(iX,iY1,iX,iY2);
		}
		int iY=0;
		int iX1=(int)((grid->GetLine(x,0)-offsetX)/factor);
		int iX2=(int)((grid->GetLine(x,grid->GetQtyLines(x)-1)-offsetX)/factor);
		for (size_t n=0;n<grid->GetQtyLines(y);++n)
		{
			iY=height()-(int)((grid->GetLine(y,n)-offsetY)/factor);
			painter.drawLine(iX1,iY,iX2,iY);
		}
	}
	int currPrim=clCS->GetCurrentPrimitive();
	int currProp=clCS->GetCurrentProperty();

	for (size_t i=0;i<clCS->GetQtyPrimitives();++i)
	{
		prim=clCS->GetPrimitive(i);
		if (prim==NULL) break;
		prop=prim->GetProperty();
		if (prop==NULL) break;
		if (prop->GetVisibility())
		{
			RGBa color=prop->GetFillColor();
			//QBrush brush(QColor(color.R,color.G,color.B,0));
			//painter.setBrush(brush);
			color=prop->GetEdgeColor();
			QPen pen(QColor(color.R,color.G,color.B,255));
			if (currPrim>=0) {if ((size_t)currPrim==i) pen.setWidth(2);}
			else
			{
				if (currProp>=0) { if (prop->GetID()==(unsigned int)currProp) pen.setWidth(2);}
			}
			painter.setPen(pen);

			switch (prim->GetType())
			{
				case CSPrimitives::BOX:
				{
					CSPrimBox* box=prim->ToBox();
					double x1=(box->GetCoord(2*x)-offsetX)/factor;
					double y1=(box->GetCoord(2*y)-offsetY)/factor;
					double x2=(box->GetCoord(2*x+1)-offsetX)/factor;
					double y2=(box->GetCoord(2*y+1)-offsetY)/factor;
					if (x1>x2) {help=x2;x2=x1;x1=help;}
					if (y1>y2) {help=y2;y2=y1;y1=help;}
					painter.drawRect(QRect((int)x1,height()-(int)y2,(int)(x2-x1),(int)(y2-y1)));
					break;
				}
				case CSPrimitives::MULTIBOX:
				{
					CSPrimMultiBox* MultiBox=prim->ToMultiBox();
					for (unsigned int n=0;n<MultiBox->GetQtyBoxes();++n)
					{
						double x1=(MultiBox->GetCoord(6*n+2*x)-offsetX)/factor;
						double y1=(MultiBox->GetCoord(6*n+2*y)-offsetY)/factor;
						double x2=(MultiBox->GetCoord(6*n+2*x+1)-offsetX)/factor;
						double y2=(MultiBox->GetCoord(6*n+2*y+1)-offsetY)/factor;
						if (x1>x2) {help=x2;x2=x1;x1=help;}
						if (y1>y2) {help=y2;y2=y1;y1=help;}
						painter.drawRect(QRect((int)x1,height()-(int)y2,(int)(x2-x1),(int)(y2-y1)));
					}
					break;
				}
				case CSPrimitives::SPHERE:
				{
					CSPrimSphere* sphere=prim->ToSphere();
					double x1=(sphere->GetCoord(x)-offsetX)/factor;
					double y1=(sphere->GetCoord(y)-offsetY)/factor;
					double r=sphere->GetRadius()/factor;
					painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
					break;
				}
				case CSPrimitives::CYLINDER:
				{  //erstmal nur kartesische Cylinder
					CSPrimCylinder* cylinder=prim->ToCylinder();
					double r0[3]={cylinder->GetCoord(0),cylinder->GetCoord(2),cylinder->GetCoord(4)};
					double r1[3]={cylinder->GetCoord(1),cylinder->GetCoord(3),cylinder->GetCoord(5)};
					double x1=(r0[x]-offsetX)/factor;
					double y1=(r0[y]-offsetY)/factor;
					double x2=(r1[x]-offsetX)/factor;
					double y2=(r1[y]-offsetY)/factor;
					double r=cylinder->GetRadius()/factor;
					if (r0[1]==r1[1] && r0[2]==r1[2]) //x-ausdehnung
					{
						if (direct==0)
						{
							painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
						}
						else if (direct==2)
						{
							painter.drawRect(QRect((int)x1,height()-(int)(y2+r),(int)(x2-x1),(int)(y2-y1+2*r)));
						}
						else if (direct==1)
						{
							painter.drawRect(QRect((int)(x1-r),height()-(int)y2,(int)(x2-x1+2*r),(int)(y2-y1)));
						}
					}
					else if (r0[0]==r1[0] && r0[2]==r1[2]) //y-ausdehnung
					{
						if (direct==1)
						{
							painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
						}
						else if (direct==0)
						{
							painter.drawRect(QRect((int)x1,height()-(int)(y2+r),(int)(x2-x1),(int)(y2-y1+2*r)));
						}
						else if (direct==2)
						{
							painter.drawRect(QRect((int)(x1-r),height()-(int)y2,(int)(x2-x1+2*r),(int)(y2-y1)));
						}
					}
					else if (r0[1]==r1[1] && r0[0]==r1[0]) //z-ausdehnung
					{
						if (direct==2)
						{
							painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
						}
						else if (direct==1)
						{
							painter.drawRect(QRect((int)x1,height()-(int)(y2+r),(int)(x2-x1),(int)(y2-y1+2*r)));
						}
						else if (direct==0)
						{
							painter.drawRect(QRect((int)(x1-r),height()-(int)y2,(int)(x2-x1+2*r),(int)(y2-y1)));
						}
					}
					break;
				}
				case CSPrimitives::POLYGON:
				{
					CSPrimPolygon* poly=prim->ToPolygon();
					double elev = poly->GetElevation();
					int normDir = 0;
					if (poly->GetNormDir(0) != 0) normDir = 0;
					else if (poly->GetNormDir(1) != 0) normDir = 1;
					else if (poly->GetNormDir(2) != 0) normDir = 2;
					int nrPts = poly->GetQtyCoords();
					QPointF points[nrPts+1];
					for (int n=0;n<nrPts;++n)
					{
						if (normDir == direct)
						{
							points[n].setX((poly->GetCoord(2*n)-offsetX)/factor);
							points[n].setY(height()-(poly->GetCoord(2*n+1)-offsetY)/factor);
						}
						else if ( normDir == ((direct+2)%3) )
						{
							points[n].setX((poly->GetCoord(2*n+1)-offsetX)/factor);
							points[n].setY(height()-(elev-offsetY)/factor);
						}
						else if ( normDir == ((direct+1)%3) )
						{
							points[n].setX((elev-offsetX)/factor);
							points[n].setY(height()-(poly->GetCoord(2*n)-offsetY)/factor);
						}
						else
							break;
					}
					points[nrPts] = points[0];
					painter.drawPolyline(points,nrPts+1);
					break;
				}
				case CSPrimitives::CURVE:
				case CSPrimitives::WIRE:
				{
					CSPrimCurve* curve = NULL;
					if (prim->GetType()==CSPrimitives::CURVE)
						curve = prim->ToCurve();
					else
						curve = prim->ToWire();

					unsigned int nrPts = (unsigned int)curve->GetNumberOfPoints();
					QPointF points[nrPts];
					double xyz[3];
					int nP = (direct+1)%3;
					int nPP = (direct+2)%3;
					for (unsigned int n=0;n<nrPts;++n)
					{
						curve->GetPoint(n,xyz);
						points[n].setX((xyz[nP]-offsetX)/factor);
						points[n].setY(height()-(xyz[nPP]-offsetY)/factor);
					}
					if (prim->GetType()==CSPrimitives::CURVE)
						painter.drawPolyline(points,nrPts);
					else
					{
//						cerr << "QCSXCAD: Warning: 2D painting of wire currently with thin pen only..." << endl;
						painter.drawPolyline(points,nrPts);
					}
					break;
				}
			};
		}
	}

	//calculate and draw an arrow here
	if (bArrow)
	{
		QPen pen(QColor(255,0,0,255));
		pen.setWidth(2);
		painter.setPen(pen);
		int x1 = (int)((dArrow[0]-offsetX)/factor);
		int y1 = height()-(int)((dArrow[1]-offsetY)/factor);
		int x2 = (int)((dArrow[2]-offsetX)/factor);
		int y2 = height()-(int)((dArrow[3]-offsetY)/factor);

		painter.drawLine(x1,y1,x2,y2);
		painter.drawText(x1,y1,QString("(%1,%2)").arg(dArrow[0]).arg(dArrow[1]));
		painter.drawText(x2,y2,QString("(%1,%2)").arg(dArrow[2]).arg(dArrow[3]));

		double angle = atan2(y2-y1,x2-x1);
		double PI = 3.141;
		painter.drawLine(x2,y2,x2-10*cos(angle-PI/4),y2-10*sin(angle-PI/4));
		painter.drawLine(x2,y2,x2-10*cos(angle+PI/4),y2-10*sin(angle+PI/4));
	}

	QRectF target(10, height()-110.0, 100.0, 100.0);
	//QRectF source(0.0, 0.0, 70.0, 40.0);
	QImage image;
	if (direct==2) image=QImage(":/images/ArrowsXY.png");
	if (direct==1) image=QImage(":/images/ArrowsXZ.png");
	if (direct==0) image=QImage(":/images/ArrowsYZ.png");
	QRect source=image.rect();

	painter.drawImage(target, image, source);

//	QPen pen;
//	pen.setWidth(5);
//	painter.setPen(pen);
//	QPointF points[5];
//	if (direct=2)
//	{
//		pen.setColor(QColor(0,0,255,255));
//		points[0]=QPointF(10.0, height()-20.0);
//		points[1]=QPointF(60.0, height()-20.0);
//		points[2]=QPointF(50.0, height()-30.0);
//		points[3]=QPointF(50.0, height()-10.0);
//		points[4]=QPointF(60.0, height()-20.0);
//		painter.drawPolyline(points, 5);
//	}

	// *****
	// * indicate the boundary conditions

	// calculate offset in
	QRect v = painter.viewport();
	QRect w = painter.window();
	int offset1 = 10 * (double)w.width() / (double)v.width();
	int offset2 = 10 * (double)w.height() / (double)v.height();

	CSRectGrid* grid = clCS->GetGrid();
	int iY1 = height()-(int)((grid->GetLine(y,0)-offsetY)/factor);
	int iY2 = height()-(int)((grid->GetLine(y,grid->GetQtyLines(y)-1)-offsetY)/factor);
	int iX1 = (int)((grid->GetLine(x,0)-offsetX)/factor);
	int iX2 = (int)((grid->GetLine(x,grid->GetQtyLines(x)-1)-offsetX)/factor);
	int iY = (iY1 + iY2) / 2.0;
	int iX = (iX1 + iX2) / 2.0;

	QString xmin, xmax, ymin, ymax;
	QHash<QString,QString> BC = clCS->Get_BC();
	if (direct == 0) {
		// y-z plane
		xmin = BC.value("ymin");
		xmax = BC.value("ymax");
		ymin = BC.value("zmin");
		ymax = BC.value("zmax");
	}
	else if (direct == 1) {
		// x-z plane
		xmin = BC.value("zmin");
		xmax = BC.value("zmax");
		ymin = BC.value("xmin");
		ymax = BC.value("xmax");
	}
	else if (direct == 2) {
		// x-y plane
		xmin = BC.value("xmin");
		xmax = BC.value("xmax");
		ymin = BC.value("ymin");
		ymax = BC.value("ymax");
	}

	painter.setPen(Qt::black);
	painter.save();
	painter.translate( iX1-offset1, iY );
	painter.rotate(-90);
	painter.drawText( 0, 0, 0, 0, Qt::AlignCenter|Qt::TextDontClip, xmin );
	painter.restore();
	painter.save();
	painter.translate( iX2+offset1, iY );
	painter.rotate(-90);
	painter.drawText( 0, 0, 0, 0, Qt::AlignCenter|Qt::TextDontClip, xmax );
	painter.restore();
	painter.drawText( iX, iY1+offset2, 0, 0, Qt::AlignCenter|Qt::TextDontClip, ymin );
	painter.drawText( iX, iY2-offset2, 0, 0, Qt::AlignCenter|Qt::TextDontClip, ymax );
}

void QGeometryPlot::wheelEvent(QWheelEvent * event)
{
	double zoomFac=(double)(event->delta());
	zoomFac/=120;
	zoomFac/=-20;

	for (int i=0;i<3;++i)
	{
		double width=(DrawArea[2*i+1]-DrawArea[2*i])*zoomFac;
		DrawArea[2*i]=DrawArea[2*i]-width;
		DrawArea[2*i+1]=DrawArea[2*i+1]+width;
	}
	if (event->buttons()==Qt::LeftButton)
	{
		GetMouseXY(event->pos());
		dArrow[2]=lastMouseXY[0];
		dArrow[3]=lastMouseXY[1];
	}
	update();
}


void QGeometryPlot::mousePressEvent(QMouseEvent * event)
{
	Pos=event->pos();
	if (event->button()==Qt::RightButton)
	{
		setCursor(Qt::ClosedHandCursor);
	}
	if (event->button()==Qt::LeftButton)
	{
		bArrow=true;
		GetMouseXY(event->pos());
		dArrow[0]=lastMouseXY[0];
		dArrow[1]=lastMouseXY[1];
	}
}



void QGeometryPlot::mouseReleaseEvent(QMouseEvent * event)
{
	setCursor(Qt::CrossCursor);
	if (event->button()==Qt::LeftButton)
	{
		GetMouseXY(event->pos());
		dArrow[2]=lastMouseXY[0];
		dArrow[3]=lastMouseXY[1];
		if (dArrow[0]==dArrow[2] && dArrow[1]==dArrow[3]) bArrow=false;
		else bArrow=true;
	}
}

void QGeometryPlot::mouseMoveEvent(QMouseEvent * event)
{
	if (statBar!=NULL)
	{
		GetMouseXY(event->pos());
		if (direct==0) statBar->showMessage(QString("Y: %1  Z: %2").arg(lastMouseXY[0]).arg(lastMouseXY[1]));
		if (direct==1) statBar->showMessage(QString("Z: %1  X: %2").arg(lastMouseXY[0]).arg(lastMouseXY[1]));
		if (direct==2) statBar->showMessage(QString("X: %1  Y: %2").arg(lastMouseXY[0]).arg(lastMouseXY[1]));
	}

	if ((event->buttons()==Qt::LeftButton) && (bArrow))
	{
		GetMouseXY(event->pos());
		dArrow[2]=lastMouseXY[0];
		dArrow[3]=lastMouseXY[1];
		update();
		return;
	}
	if (event->buttons()!=Qt::RightButton) return;

	double shiftX=-1*(Pos.x()-event->x())*factor;
	double shiftY=(Pos.y()-event->y())*factor;


	int x=(direct+1)%3;
	int y=(direct+2)%3;

	DrawArea[2*x]=DrawArea[2*x]-shiftX;
	DrawArea[2*x+1]=DrawArea[2*x+1]-shiftX;

	DrawArea[2*y]=DrawArea[2*y]-shiftY;
	DrawArea[2*y+1]=DrawArea[2*y+1]-shiftY;

	Pos=event->pos();
	update();
}


double* QGeometryPlot::GetMouseXY(const QPoint qp, bool bRound)
{
	lastMouseXY[0]=qp.x()*factor+offsetX;
	lastMouseXY[1]=(height()-qp.y())*factor+offsetY;
	if (bRound==false) return lastMouseXY;
	double logfac = pow(10,round(log10(factor)));
	lastMouseXY[0] = round(lastMouseXY[0]/logfac)*logfac;
	lastMouseXY[1] = round(lastMouseXY[1]/logfac)*logfac;
	return lastMouseXY;
}
