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

#ifndef _QCSPROPEDITOR_H_
#define _QCSPROPEDITOR_H_

#include <QPushButton>
#include <QDialog>
#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>

#include "ContinuousStructure.h"

class QColorPushButton : public QPushButton
{
	Q_OBJECT
public:
	QColorPushButton(const QString & text, QWidget * parent = 0 );
	~QColorPushButton();

	void SetColor(RGBa c);
	RGBa GetColor();

protected slots:
	void ChangeColor();

protected:
	QColor Color;
};

class QCSPropertyGroupBox;
class QCSPropMaterialGB;

class QCSPropEditor : public QDialog
{
	Q_OBJECT
public:
	QCSPropEditor(ContinuousStructure *CS, CSProperties* prop, int iSimMode, QWidget* parent=NULL);
	~QCSPropEditor() {};

protected slots:
	void Reset();
	void Save();
	void Cancel();

	void ChangeType(int item);

	void Fill2EdgeColor();
	void Edge2FillColor();

	void GetValues();
	void SetValues();

protected:
	QComboBox* TypeCB;

	QGroupBox* BuildGeneral();
	QLayout* BuildButtons();

	QVBoxLayout* MainLayout;
	//QCSPropertyLayout* CSPropEdit;
	ContinuousStructure *clCS;

	QLineEdit* Name;
	QLabel* TypeLbl;

	QColorPushButton* FCButton;
	QColorPushButton* ECButton;

	CSProperties* clProp;
	CSProperties* saveProp;

	QCSPropertyGroupBox* BuildPropGroupBox(CSProperties* clProp);
	QCSPropertyGroupBox* propGB;

	int GetIndexOfType();

	int m_SimMode;
};

//dummy class
class QCSPropertyGroupBox : public QGroupBox
{
	Q_OBJECT
public:
	virtual ~QCSPropertyGroupBox();

protected:
	QCSPropertyGroupBox(QWidget *parent=NULL);

	QString TypeName;
	QString GetTypeName() {return TypeName;};

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

};

class QCSPropUnknownGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropUnknownGB(CSPropUnknown *prop, QWidget *parent=NULL);
	virtual ~QCSPropUnknownGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropUnknown* clProp;

	QLineEdit* Property;
};


class QCSPropMaterialGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropMaterialGB(CSPropMaterial *prop, QWidget *parent=NULL);
	virtual ~QCSPropMaterialGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropMaterial* clProp;
};

class QCSPropExcitationGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropExcitationGB(CSPropExcitation *prop, QWidget *parent=NULL);
	virtual ~QCSPropExcitationGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

protected slots:
	void TypeChanged(int index);

signals:
	void modified();

protected:
	CSPropExcitation* clProp;

	QLineEdit* FctLine[3];

	QSpinBox* Number;
	QComboBox* Type;

	QLineEdit* Excitation[3];
};

class QCSPropProbeBoxGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropProbeBoxGB(CSPropProbeBox *prop, QWidget *parent=NULL);
	virtual ~QCSPropProbeBoxGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropProbeBox* clProp;

	QSpinBox* Number;
};

class QCSPropResBoxGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropResBoxGB(CSPropResBox *prop, QWidget *parent=NULL);
	virtual ~QCSPropResBoxGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropResBox* clProp;

	QSpinBox* Factor;
};

class QCSPropDumpBoxGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropDumpBoxGB(CSPropDumpBox *prop, int SimMode, QWidget *parent=NULL);
	virtual ~QCSPropDumpBoxGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropDumpBox* clProp;
};


#endif //_QCSPROPEDITOR_H_
