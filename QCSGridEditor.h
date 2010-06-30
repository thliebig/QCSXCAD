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

#include <QtGui>

class CSRectGrid;

class QCSGridEditor : public QWidget
{
	Q_OBJECT
public:
	QCSGridEditor(CSRectGrid* grid, QWidget* parent = 0);
	virtual ~QCSGridEditor();

	QToolBar* BuildToolbar();

signals:
	void OpacityChange(int);
	void signalDetectEdges(int);
	void GridChanged();

public slots:
	void Update();
	void SetOpacity(int val);
	void EditX();
	void EditY();
	void EditZ();
	void Edit(int direct);

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

	double* GetDoubleArrayFromString(int *count, QString qsValue);
};

#endif //_QCSGRIDEDITOR_H_
