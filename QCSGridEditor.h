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

#ifndef _QCSGRIDEDITOR_H_
#define _QCSGRIDEDITOR_H_

#include <QWidget>
#include <QToolBar>
#include <QString>
#include <QVector>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>

class CSRectGrid;

class QCSGridEditor : public QWidget
{
	Q_OBJECT
public:
	QCSGridEditor(CSRectGrid* grid, QWidget* parent = 0);
	virtual ~QCSGridEditor();

	QToolBar* BuildToolbar();
	QWidget* BuildPlanePosWidget();

	QString GetDirName(int ny);
	QString GetNormName(int ny);

	int GetOpacity();

signals:
	void OpacityChange(int);
	void signalDetectEdges(int);
	void GridChanged();
	void GridPlaneXChanged(int);
	void GridPlaneYChanged(int);
	void GridPlaneZChanged(int);

public slots:
	void Update();
	void SetOpacity(int val);
	void EditX();
	void EditY();
	void EditZ();
	void Edit(int direct);
	void SetGridPlaneX(int pos);
	void SetGridPlaneY(int pos);
	void SetGridPlaneZ(int pos);

protected slots:
	void BuildHomogenDisc();
	void BuildInHomogenDisc();
	void IncreaseResolution();
	void DetectEdges();
	void SetDeltaUnit(QString val);

protected:
	CSRectGrid* clGrid;
	QVector<QLabel*> SimBox;
	QVector<QLabel*> NodeQty;
	//QComboBox* UnitLength;
	QLineEdit* UnitLength;
	QSlider* OpacitySlider;
	QLabel* m_DirNames[3];
	QSlider* m_PlanePos[3];
	QLabel* m_NormNames[3];
	QLabel* m_PlanePosValue[3];

	double* GetDoubleArrayFromString(int *count, QString qsValue);
};

#endif //_QCSGRIDEDITOR_H_
