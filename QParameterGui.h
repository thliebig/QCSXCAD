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

#ifndef _QPARAMETERGUI_H_
#define _QPARAMETERGUI_H_

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QSlider>

#include "ParameterObjects.h"

class QParameter : public QGroupBox
{
	Q_OBJECT
public:
	QParameter(Parameter* para, QWidget* parent=NULL);
	virtual ~QParameter();
	
	Parameter* GetParameter() {return clPara;};
	
signals:
 	void ParameterChanged();
 	void Delete(Parameter* para);
	
public slots:
	virtual bool Edit();
	virtual void Update();
	void DeleteParameter();
	void SweepState(int state);

protected slots:
	virtual void Changed();
		
protected:
	Parameter* clPara;
	QLabel* Name;
	QLineEdit* Value;
	QGridLayout *lay;
	QCheckBox* SweepCB;
};

class QLinearParameter : public QParameter
{
	Q_OBJECT
public:
	QLinearParameter(LinearParameter* para, QWidget* parent=NULL);
	virtual ~QLinearParameter();
 		
signals:

public slots:
	virtual bool Edit();
	virtual void Update();

protected slots:
	virtual void Changed();
 		
protected:
	QSlider* slider;
};

class QParameterSet : public QWidget, public ParameterSet
{
	Q_OBJECT
public:
	QParameterSet(QWidget* parent=NULL);
	virtual ~QParameterSet();

	virtual size_t LinkParameter(Parameter* newPara);

signals:
 	void ParameterChanged();
 	
public slots:
 	void NewParameter();
	virtual void SetModified(bool mod=true);
	
	virtual void clear();
 
protected slots:
	virtual size_t DeleteParameter(Parameter* para); 		
	
protected:
	void AddParaWid(Parameter* newPara);
	QVBoxLayout* ParaLay;
	QVector<QParameter*> vecQPara;
};

#endif //_QPARAMETERGUI_H_
