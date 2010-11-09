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

#ifndef QGEOMETRYPLOT_H
#define QGEOMETRYPLOT_H

#include <QtGui>
#include "QCSXCAD_Global.h"

class QCSXCAD;

class QGeometryPlot : public QWidget
{
	Q_OBJECT
public:
	QGeometryPlot(QCSXCAD* CS);
	~QGeometryPlot();

	void SetStatusBar(QStatusBar* bar) {statBar=bar;};

public slots:
	void setXY();
	void setYZ();
	void setZX();
	void setDrawArea(double* area);
	void setGridOpacity(int val);

	void Reset();

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void wheelEvent(QWheelEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);

	double* GetMouseXY(const QPoint qp, bool bRound=true);
	double lastMouseXY[2];

	bool bArrow;
	double dArrow[4];

	QCSXCAD* clCS;
	int direct;
	int spacer;
	int GridOpacity;
	double DrawArea[6];

	double factor;
	double offsetX,offsetY;

	QStatusBar* statBar;

	QPoint Pos;
};

#endif // QGEOMETRYPLOT_H
