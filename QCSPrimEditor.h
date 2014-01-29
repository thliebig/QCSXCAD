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

#ifndef _QCSPRIMEDITOR_H_
#define _QCSPRIMEDITOR_H_

#include <QDialog>
#include <QWidget>
#include <QLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QListWidgetItem>

#include "ContinuousStructure.h"

class QCSPrimitiveLayout;
class QCSPrimBoxLayout;
class QCSPrimSphereLayout;
class QCSPrimCylinderLayout;
class QCSPrimMultiBoxLayout;
class QCSPrimPolygon;
	class QCSPrimLinPoly;
	class QCSPrimRotPoly;
class QCSPrimUserDefinedLayout;

class QCSPrimEditor : public QDialog
{
	Q_OBJECT
public:
	QCSPrimEditor(ContinuousStructure *CS, CSPrimitives* prim, QWidget* parent=NULL);
	virtual ~QCSPrimEditor();
	
protected slots:
	void Reset();
	void Save();
	void Cancel();

	void UpdatePropertyCB();
	
protected:
	QGroupBox* BuildGeneral();
	QLayout* BuildButtons();
	
	QSpinBox* PrioSpinBox;
	QComboBox* PropertiesComboBox;
	
	QVBoxLayout* MainLayout;
	QCSPrimitiveLayout* CSPrimEdit;
	ContinuousStructure *clCS;
	CSPrimitives* CSPrim;
};

class QCSPrimitiveLayout : public QGridLayout
{
	Q_OBJECT
public:
	QCSPrimitiveLayout(CSPrimitives* prim, QWidget *parent=NULL);
	virtual ~QCSPrimitiveLayout();
	
public slots:
	virtual void SetValues();
	virtual void GetValues();
	
signals:
	void modified();

protected:
	CSPrimitives* clPrim;
	
};


class QCSPrimBoxLayout : public QCSPrimitiveLayout
{
	Q_OBJECT
public:
	QCSPrimBoxLayout(CSPrimBox* prim, QWidget *parent=NULL);
	virtual ~QCSPrimBoxLayout();
	
public slots:
	virtual void SetValues();
	virtual void GetValues();
	
signals:
	void modified();
	
protected:
	CSPrimBox* clBox;
	QLineEdit *Lines[6];
};

class QCSPrimSphereLayout : public QCSPrimitiveLayout
{
	Q_OBJECT
public:
	QCSPrimSphereLayout(CSPrimSphere* prim, QWidget *parent=NULL);
	virtual ~QCSPrimSphereLayout();
	
public slots:
	virtual void SetValues();
	virtual void GetValues();
	
signals:
	void modified();
	
protected:
	CSPrimSphere* clSphere;
	QLineEdit *Lines[4];
};

class QCSPrimCylinderLayout : public QCSPrimitiveLayout
{
	Q_OBJECT
public:
	QCSPrimCylinderLayout(CSPrimCylinder* prim, QWidget *parent=NULL);
	virtual ~QCSPrimCylinderLayout();
	
public slots:
	virtual void SetValues();
	virtual void GetValues();
	
signals:
	void modified();
	
protected:
	CSPrimCylinder* clCylinder;
	QLineEdit *Lines[7];
};

class QCSPrimCylindricalShellLayout : public QCSPrimCylinderLayout
{
	Q_OBJECT
public:
	QCSPrimCylindricalShellLayout(CSPrimCylindricalShell* prim, QWidget *parent=NULL);
	virtual ~QCSPrimCylindricalShellLayout();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPrimCylindricalShell* clCylindricalShell;
	QLineEdit *m_ShellWidth;
};

class QCSPrimMultiBoxLayout : public QCSPrimitiveLayout
{
	Q_OBJECT
public:
	QCSPrimMultiBoxLayout(CSPrimMultiBox* prim, QWidget *parent=NULL);
	virtual ~QCSPrimMultiBoxLayout();
	
public slots:
	virtual void SetValues();
	virtual void GetValues();
	
protected slots:
	void NewBox(QListWidgetItem* item=NULL);
	void DeleteBox(QListWidgetItem* item=NULL);
	void EditBox(QListWidgetItem* item=NULL);
	
signals:
	void modified();
	
protected:
	CSPrimMultiBox* clMultiBox;
	
	QListWidget* qBoxList;
	QVector<QString> vLines;
};

class QCSPrimPolygonLayout : public QCSPrimitiveLayout
{
	Q_OBJECT
public:
	QCSPrimPolygonLayout(CSPrimPolygon* prim, QWidget *parent=NULL);
	virtual ~QCSPrimPolygonLayout();
	
public slots:
	virtual void SetValues();
	virtual void GetValues();
	
protected slots:
	void NormVecChanged();
	
signals:
	void modified();
	
protected:
	CSPrimPolygon* clPoly;
	QComboBox* NormVec;
	QLineEdit* Elevation;
	QLineEdit* CoordLineX;
	QLineEdit* CoordLineY;
};

class QCSPrimUserDefinedLayout : public QCSPrimitiveLayout
{
	Q_OBJECT
public:
	QCSPrimUserDefinedLayout(CSPrimUserDefined* prim, QWidget *parent=NULL);
	virtual ~QCSPrimUserDefinedLayout();
	
public slots:
	virtual void SetValues();
	virtual void GetValues();
	
signals:
	void modified();
	
protected:
	CSPrimUserDefined* clUserDef;
	QLineEdit *FunctionLine;
	QComboBox *CoordSystem;
	QLineEdit *CoordShift[3];
};

#endif //_QCSPRIMEDITOR_H_
