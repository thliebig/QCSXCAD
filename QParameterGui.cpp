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

#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QButtonGroup>
#include <QRadioButton>

#include "QParameterGui.h"
#include "QCSXCAD_Global.h"

QParameter::QParameter(Parameter* para, QWidget* parent) : QGroupBox(parent)
{
	clPara=para;
	Name=new QLabel("");
	Value=new QLineEdit("");
	QObject::connect(Value,SIGNAL(textEdited(QString)),this,SLOT(Changed()));
	
	
	lay = new QGridLayout();
//	lay->addWidget(new QLabel(tr("Name: ")),0,0);
//	lay->addWidget(Name,0,1);

	lay->addWidget(new QLabel(tr("Value: ")),0,0);
	lay->addWidget(Value,0,1);
	
	QPushButton* btn = new QPushButton(QIcon(":/images/failed.png"),QString());
	QObject::connect(btn,SIGNAL(clicked()),this,SLOT(deleteLater()));	
	QObject::connect(btn,SIGNAL(clicked()),this,SLOT(DeleteParameter()));
	btn->setToolTip(tr("Delete parameter"));	
	lay->addWidget(btn,0,3);
	
	SweepCB = new QCheckBox();
	QObject::connect(SweepCB,SIGNAL(stateChanged(int)),this,SLOT(SweepState(int)));	
	SweepCB->setToolTip(tr("Sweep this parameter"));
	lay->addWidget(SweepCB,0,2);
	
	setLayout(lay);	
	//setFrameStyle(QFrame::Box);
}

QParameter::~QParameter()
{
}

void QParameter::DeleteParameter()
{
	emit Delete(clPara);
}

void QParameter::SweepState(int state)
{
	clPara->SetSweep(state);
}

void QParameter::Changed()
{
	double val=Value->text().toDouble();
	clPara->SetValue(val);
	emit ParameterChanged();
}

bool QParameter::Edit()
{
	QDialog* diag = new QDialog(this);
	
	QGroupBox* Group=new QGroupBox(tr("Edit Constant Parameter"));

	QGridLayout* lay = new QGridLayout();
	
	lay->addWidget(new QLabel(tr("Name: ")),0,0);
	QLineEdit* eName= new QLineEdit(QString::fromUtf8(clPara->GetName().c_str()));
	lay->addWidget(eName,0,1);
	
	lay->addWidget(new QLabel(tr("Value: ")),1,0);
	QLineEdit* eValue=new QLineEdit(QString("%1").arg(clPara->GetValue()));
	lay->addWidget(eValue,1,1);
	
	Group->setLayout(lay);
	
	QGridLayout* Grid = new QGridLayout();
	Grid->addWidget(Group,0,0,1,2);
	
	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),diag,SLOT(accept()));
	Grid->addWidget(PB,1,0);
	PB = new QPushButton(tr("Cancel"));
	QObject::connect(PB,SIGNAL(clicked()),diag,SLOT(reject()));
	Grid->addWidget(PB,1,1);
	
	diag->setLayout(Grid);
	diag->show();
	diag->setFixedSize(diag->size());
		
	if (diag->exec()==QDialog::Accepted) 
	{
		if (eName->text().isEmpty())
		{
			QMessageBox::warning(this,tr("Edit Parameter"),tr("Parameter-Name is invalid!"));
			return false;
		}
		clPara->SetName(eName->text().toStdString());
		clPara->SetValue(eValue->text().toDouble());
		Update();
		return true;
	}		
	return false;
}

void QParameter::Update()
{
	//Name->setText(clPara->GetName());
	setTitle(QString::fromUtf8(clPara->GetName().c_str()));
	Value->setText(QString("%1").arg(clPara->GetValue()));
	if (clPara->GetSweep()) SweepCB->setCheckState(Qt::Checked);
	else SweepCB->setCheckState(Qt::Unchecked);
}

QLinearParameter::QLinearParameter(LinearParameter* para, QWidget* parent) : QParameter(para,parent)
{
	slider = new QSlider(Qt::Horizontal);
	lay->addWidget(slider,1,0,1,3);
	Value->setReadOnly(true);
	QObject::connect(slider,SIGNAL(valueChanged(int)),this,SLOT(Changed()));

	QPushButton* btn = new QPushButton(QIcon(":/images/edit.png"),QString());
	QObject::connect(btn,SIGNAL(clicked()),this,SLOT(Edit()));	
	lay->addWidget(btn,1,3);
	btn->setToolTip(tr("Edit Parameter"));
}


QLinearParameter::~QLinearParameter()
{
}

bool QLinearParameter::Edit()
{
	LinearParameter* LP = clPara->ToLinear();
	if (LP==NULL) return false;

	QDialog* diag = new QDialog(this);
	
	QGroupBox* Group=new QGroupBox(tr("Edit Linear Parameter"));

	QGridLayout* lay = new QGridLayout();
	
	lay->addWidget(new QLabel(tr("Name: ")),0,0);
	QLineEdit* eName= new QLineEdit(QString::fromUtf8(LP->GetName().c_str()));
	lay->addWidget(eName,0,1);
	
	lay->addWidget(new QLabel(tr("Value: ")),1,0);
	QLineEdit* eValue=new QLineEdit(QString("%1").arg(LP->GetValue()));
	lay->addWidget(eValue,1,1);	
	
	lay->addWidget(new QLabel(tr("Start: ")),2,0);
	QLineEdit* Start=new QLineEdit(QString("%1").arg(LP->GetMin()));
	lay->addWidget(Start,2,1);	
	
	lay->addWidget(new QLabel(tr("Stop: ")),3,0);
	QLineEdit* Stop=new QLineEdit(QString("%1").arg(LP->GetMax()));
	lay->addWidget(Stop,3,1);	
	
	lay->addWidget(new QLabel(tr("Step: ")),4,0);
	QLineEdit* Step=new QLineEdit(QString("%1").arg(LP->GetStep()));
	lay->addWidget(Step,4,1);
	
	Group->setLayout(lay);
	
	QGridLayout* Grid = new QGridLayout();
	Grid->addWidget(Group,0,0,1,2);
	
	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),diag,SLOT(accept()));
	Grid->addWidget(PB,1,0);
	PB = new QPushButton(tr("Cancel"));
	QObject::connect(PB,SIGNAL(clicked()),diag,SLOT(reject()));
	Grid->addWidget(PB,1,1);
	
	diag->setLayout(Grid);
	diag->show();
	diag->setFixedSize(diag->size());
		
	if (diag->exec()==QDialog::Accepted) 
	{
		if (eName->text().isEmpty())
		{
			QMessageBox::warning(this,tr("Edit Parameter"),tr("Parameter-Name is invalid!"));
			return false;
		}
		LP->SetName(eName->text().toStdString());
		LP->SetMin(Start->text().toDouble());
		LP->SetMax(Stop->text().toDouble());
		LP->SetStep(Step->text().toDouble());
		LP->SetValue(eValue->text().toDouble());
		Update();
		return true;
	}		
	return false;
}

void QLinearParameter::Update()
{
	QParameter::Update();
	LinearParameter* LP=clPara->ToLinear();
	if (LP==NULL) return;
	if ((LP->GetStep()>0) && (LP->GetMax()>LP->GetMin()))
	{
		double steps=(LP->GetMax()-LP->GetMin())/LP->GetStep();
		double val=(LP->GetValue()-LP->GetMin())/LP->GetStep();
		slider->setRange(1,(int)steps+1);
		slider->setValue((int)val+1);
	}
	else slider->setRange(1,1);
}

void QLinearParameter::Changed()
{
	LinearParameter* LP=clPara->ToLinear();
	if (LP==NULL) return;
	double val=LP->GetMin()+LP->GetStep()*(slider->value()-1);
	LP->SetValue(val);
	Value->setText(QString("%1").arg(LP->GetValue()));
	emit ParameterChanged();
}

QParameterSet::QParameterSet(QWidget* parent) : QWidget(parent), ParameterSet()
{
	QGridLayout* grid = new QGridLayout();
	
	ParaLay = new QVBoxLayout();

	QScrollArea* QSA = new QScrollArea();
	QWidget* QW = new QWidget();
	
	QVBoxLayout* QVBL = new QVBoxLayout();
	QVBL->addLayout(ParaLay);
	QVBL->addStretch(1);
	QW->setLayout(QVBL);
	QSA->setWidget(QW);
	QSA->setWidgetResizable(true);
	
	grid->addWidget(QSA);
//	grid->addLayout(ParaLay,0,0,1,2);

//	grid->setRowStretch(1,1);
	
	QPushButton* btn = new QPushButton(tr("New"));
	QObject::connect(btn,SIGNAL(clicked()),this,SLOT(NewParameter()));
	btn->setEnabled(QCSX_Settings.GetEdit());
	grid->addWidget(btn,2,0);
	
	setLayout(grid);
}

QParameterSet::~QParameterSet()
{
}

void QParameterSet::SetModified(bool mod)
{
	emit ParameterChanged();
	ParameterSet::SetModified(mod);
}

void QParameterSet::clear()
{
	ParameterSet::clear();
	for (int i=0;i<vecQPara.size();++i)
	{
		QParameter* QPara=vecQPara.at(i);
		delete QPara;
	}
	vecQPara.clear();
}

void QParameterSet::NewParameter()
{
	QDialog* diag = new QDialog(this);
	
	QGroupBox* Group=new QGroupBox(tr("Choose Type of Parameter"));

	QVBoxLayout* lay = new QVBoxLayout();
	
	QButtonGroup* BG = new QButtonGroup();
	QRadioButton* RB = new QRadioButton("Constant");
	RB->setChecked(true);
	BG->addButton(RB,0);		
	lay->addWidget(RB);
	RB = new QRadioButton("Linear");
	BG->addButton(RB,1);		
	lay->addWidget(RB);
	
	Group->setLayout(lay);
	
	QGridLayout* Grid = new QGridLayout();
	Grid->addWidget(Group,0,0,1,2);
	
	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),diag,SLOT(accept()));
	Grid->addWidget(PB,1,0);
	PB = new QPushButton(tr("Cancel"));
	QObject::connect(PB,SIGNAL(clicked()),diag,SLOT(reject()));
	Grid->addWidget(PB,1,1);
	
	diag->setLayout(Grid);
	diag->show();
	diag->setFixedSize(diag->size());
		
	if (diag->exec()==QDialog::Accepted) 
	{
		Parameter* newPara=NULL;
		QParameter* QPara=NULL;
		switch (BG->checkedId())
		{
			case 0:
				newPara = new Parameter();
				QPara = new QParameter(newPara);
				break;
			case 1:
				newPara = new LinearParameter();
				QPara = new QLinearParameter(newPara->ToLinear());
				break;				
		}
		if (newPara!=NULL) 
		{
			if (QPara->Edit()==true) LinkParameter(newPara);
			else 
			{
				delete newPara;
				delete QPara;
			}
		}
	}
}

size_t QParameterSet::DeleteParameter(Parameter* para)
{
	if (para!=NULL) 
	for (int i=0;i<vecQPara.size();++i)
	{
		QParameter* QPara=vecQPara.at(i);
		if (QPara->GetParameter()==para) 
		{
			delete QPara;
			vecQPara.remove(i);
		}
	}
	return ParameterSet::DeleteParameter(para);
}

void QParameterSet::AddParaWid(Parameter* newPara)
{
	if (newPara!=NULL)
	{
		QParameter* QPara=NULL;
		switch (newPara->GetType())
		{
			case 0:
				QPara = new QParameter(newPara);
				break;
			case 1:
				QPara = new QLinearParameter(newPara->ToLinear());
				break;				
			default:
				QPara=NULL;
				break;
		}	
		if (QPara!=NULL) 
		{
			ParaLay->addWidget(QPara);
			QObject::connect(QPara,SIGNAL(Delete(Parameter*)),this,SLOT(DeleteParameter(Parameter*)));	
			QObject::connect(QPara,SIGNAL(ParameterChanged()),this,SLOT(SetModified()));	
			QPara->Update();
			vecQPara.append(QPara);
		}
	}	
}

size_t QParameterSet::LinkParameter(Parameter* newPara)
{
	AddParaWid(newPara);
	return ParameterSet::LinkParameter(newPara);
	
}
