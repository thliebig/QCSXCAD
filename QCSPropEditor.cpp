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

#include <QColorDialog>

#include "QCSPropEditor.h"
#include "QCSXCAD_Global.h"
#include "iostream"
#include "time.h"

#include "CSPropUnknown.h"
#include "CSPropMaterial.h"
#include "CSPropDispersiveMaterial.h"
#include "CSPropLorentzMaterial.h"
#include "CSPropDiscMaterial.h"
#include "CSPropLumpedElement.h"
#include "CSPropMetal.h"
#include "CSPropConductingSheet.h"
#include "CSPropExcitation.h"
#include "CSPropProbeBox.h"
#include "CSPropDumpBox.h"
#include "CSPropResBox.h"

QColorPushButton::QColorPushButton(const QString & text, QWidget * parent ) : QPushButton(text,parent)
{
	setFlat(true);
	setAutoFillBackground(true);
	QObject::connect(this,SIGNAL(clicked()),this,SLOT(ChangeColor()));
	//Color.setAlpha(255);

	RGBa c;
	c.R=1 + (rand()%255);
	c.G=1 + (rand()%255);
	c.B=1 + (rand()%255);
	SetColor(c);
}

QColorPushButton::~QColorPushButton()
{
}

void QColorPushButton::SetColor(RGBa c)
{
	QPalette palette;

	Color=QColor(c.R,c.G,c.B,c.a);

	palette.setColor(QPalette::Button,QColor(c.R,c.G,c.B));
	if (c.R+c.G+c.B>(128*3)) palette.setColor(QPalette::ButtonText,QColor(0,0,0));
	else palette.setColor(QPalette::ButtonText,QColor(255,255,255));
	setPalette(palette);
}

RGBa QColorPushButton::GetColor()
{
	RGBa c;
	c.R=Color.red();
	c.G=Color.green();
	c.B=Color.blue();
	c.a=Color.alpha();
	return c;
}

void QColorPushButton::ChangeColor()
{
	QColor newColor=QColorDialog::getColor(Color,this);
	if (newColor.isValid())
	{
		int alpha=Color.alpha();
		Color=newColor;
		Color.setAlpha(alpha);
		QPalette palette;
		palette.setColor(QPalette::Button,newColor);
		if (Color.red()+Color.blue()+Color.green()>(128*3)) palette.setColor(QPalette::ButtonText,QColor(0,0,0));
		else palette.setColor(QPalette::ButtonText,QColor(255,255,255));
		setPalette(palette);
	}
}

QCSPropEditor::QCSPropEditor(ContinuousStructure *CS, CSProperties* prop, int iSimMode, QWidget* parent) : QDialog(parent)
{
	clCS=CS;
	clProp=prop;
	saveProp=clProp;
	m_SimMode=iSimMode;

	MainLayout = new QVBoxLayout();

	setWindowTitle(tr("Property Editor"));

	MainLayout->addWidget(BuildGeneral());
	GetValues();


	if (propGB!=NULL)
	{
		MainLayout->addWidget(propGB);
	}

	MainLayout->addStretch();

	MainLayout->addLayout(BuildButtons());


	setLayout(MainLayout);

	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowModality(Qt::ApplicationModal);
}

QCSPropertyGroupBox* QCSPropEditor::BuildPropGroupBox(CSProperties* clProp)
{
	QCSPropertyGroupBox* propGB=NULL;
	switch (clProp->GetType())
	{
		case CSProperties::MATERIAL:
			propGB = new QCSPropMaterialGB(clProp->ToMaterial());
			break;
		case CSProperties::METAL:
			break;
		case CSProperties::EXCITATION:
			propGB = new QCSPropExcitationGB(clProp->ToExcitation());
			break;
		case CSProperties::PROBEBOX:
			propGB = new QCSPropProbeBoxGB(clProp->ToProbeBox());
			break;
		case CSProperties::DUMPBOX:
			propGB = new QCSPropDumpBoxGB(clProp->ToDumpBox(),m_SimMode);
			break;
		case CSProperties::RESBOX:
			propGB = new QCSPropResBoxGB(clProp->ToResBox());
			break;
		case CSProperties::UNKNOWN:
			propGB = new QCSPropUnknownGB(clProp->ToUnknown());
			break;
		default:
			return NULL;
			break;
	};
	return propGB;
}


void QCSPropEditor::Reset()
{
	GetValues();
	if (propGB!=NULL) propGB->GetValues();
}

void QCSPropEditor::Save()
{
	SetValues();
	if (propGB!=NULL) propGB->SetValues();

	accept();
}

void QCSPropEditor::Cancel()
{
	reject();
}

void QCSPropEditor::ChangeType(int item)
{
	bool ok=false;
	int type = TypeCB->itemData(item).toInt(&ok);
	if (ok==false) return;
	if (type==clProp->GetType()) return;
	//if (propGB!=NULL) MainLayout->removeWidget(propGB);
	//if (propGB!=savePropGB) delete propGB;
	if (clProp!=saveProp) delete clProp;
	delete propGB;propGB=NULL;
	if (type==saveProp->GetType())
	{
		clProp=saveProp;
	}
	else switch (type)
	{
		case CSProperties::MATERIAL:
			clProp = new CSPropMaterial(saveProp);
			break;
		case CSProperties::METAL:
			clProp = new CSPropMetal(saveProp);
			break;
		case CSProperties::EXCITATION:
			clProp = new CSPropExcitation(saveProp);
			break;
		case CSProperties::PROBEBOX:
			clProp = new CSPropProbeBox(saveProp);
			break;
		case CSProperties::DUMPBOX:
			clProp = new CSPropDumpBox(saveProp);
			break;
		case CSProperties::RESBOX:
			clProp = new CSPropResBox(saveProp);
			break;
		default:
			clProp = new CSPropUnknown(saveProp);
			break;
	};
	propGB=BuildPropGroupBox(clProp);
	if (propGB!=NULL) MainLayout->insertWidget(1,propGB);
	setLayout(MainLayout);
}

void QCSPropEditor::Fill2EdgeColor()
{
	ECButton->SetColor(FCButton->GetColor());
}

void QCSPropEditor::Edge2FillColor()
{
	FCButton->SetColor(ECButton->GetColor());
}

QGroupBox* QCSPropEditor::BuildGeneral()
{
	QGroupBox* box= new QGroupBox("General");
	QGridLayout* grid = new QGridLayout();

	Name = new QLineEdit(QString::fromUtf8(clProp->GetName().c_str()));
	Name->setEnabled(QCSX_Settings.GetEdit());
	grid->addWidget(new QLabel(tr("Name: ")),0,0);
	grid->addWidget(Name,0,1);
	grid->addWidget(new QLabel(QString(tr("ID: %1")).arg(clProp->GetID())),0,2);

	grid->addWidget(new QLabel(tr("Type: ")),1,0);
	TypeCB = new QComboBox();
	TypeCB->addItem(tr("Unknown"),QVariant(CSProperties::UNKNOWN));
	TypeCB->addItem(tr("Material"),QVariant(CSProperties::MATERIAL));
	TypeCB->addItem(tr("Metal"),QVariant(CSProperties::METAL));
	TypeCB->addItem(tr("Excitation"),QVariant(CSProperties::EXCITATION));
	TypeCB->addItem(tr("Probe Box"),QVariant(CSProperties::PROBEBOX));
	TypeCB->addItem(tr("Res Box"),QVariant(CSProperties::RESBOX));
	TypeCB->addItem(tr("Dump Box"),QVariant(CSProperties::DUMPBOX));
	TypeCB->setEnabled(QCSX_Settings.GetEdit());
	grid->addWidget(TypeCB,1,1,1,2);
	QObject::connect(TypeCB,SIGNAL(currentIndexChanged(int)),this,SLOT(ChangeType(int)));

	QHBoxLayout* HLay = new QHBoxLayout();
	HLay->addStretch(1);
	FCButton = new QColorPushButton(tr("Change Fill-Color"));
	FCButton->setFixedSize(100,50);
	HLay->addWidget(FCButton);

	QVBoxLayout* VLay = new QVBoxLayout();
	QPushButton* F2EButton = new QPushButton("-->");
	F2EButton->setFixedSize(30,20);
	QObject::connect(F2EButton,SIGNAL(clicked()),this,SLOT(Fill2EdgeColor()));
	QPushButton* E2FButton = new QPushButton("<--");
	E2FButton->setFixedSize(30,20);
	QObject::connect(E2FButton,SIGNAL(clicked()),this,SLOT(Edge2FillColor()));
	VLay->addWidget(F2EButton);
	VLay->addWidget(E2FButton);
	HLay->addLayout(VLay);

	ECButton = new QColorPushButton(tr("Change Edge-Color"));
	ECButton->setFixedSize(100,50);
	HLay->addWidget(ECButton);
	HLay->addStretch(1);

	grid->addLayout(HLay,2,0,1,3);
	box->setLayout(grid);

	return box;
}

QLayout* QCSPropEditor::BuildButtons()
{
	QHBoxLayout* lay = new QHBoxLayout();

	QPushButton* ok = new QPushButton(tr("Ok"));
	QObject::connect(ok,SIGNAL(clicked()),this,SLOT(Save()));
	lay->addWidget(ok);
	if (QCSX_Settings.GetEdit())
	{
		QPushButton* reset = new QPushButton(tr("Reset"));
		QObject::connect(reset,SIGNAL(clicked()),this,SLOT(Reset()));
		lay->addWidget(reset);
		QPushButton* cancel = new QPushButton(tr("Cancel"));
		QObject::connect(cancel,SIGNAL(clicked()),this,SLOT(Cancel()));
		lay->addWidget(cancel);
	}
	lay->addStretch();

	return lay;
}

void QCSPropEditor::GetValues()
{
	if (clProp!=saveProp)
	{
		delete clProp;
		delete propGB;
		clProp=saveProp;
	}

	propGB = BuildPropGroupBox(clProp);

	Name->setText(QString::fromUtf8(clProp->GetName().c_str()));
	TypeCB->setCurrentIndex(GetIndexOfType());
	FCButton->SetColor(clProp->GetFillColor());
	ECButton->SetColor(clProp->GetEdgeColor());
}

void QCSPropEditor::SetValues()
{
	//RGBa c;
	clProp->SetFillColor(FCButton->GetColor());
	clProp->SetEdgeColor(ECButton->GetColor());
	clProp->SetName(Name->text().toLatin1().data());

	if (saveProp!=clProp) clCS->ReplaceProperty(saveProp,clProp);
}

int QCSPropEditor::GetIndexOfType()
{
	for (int i=0;i<TypeCB->count();++i)
	{
		if (TypeCB->itemData(i).toInt()==clProp->GetType()) return i;
	}
	return 0;
}

/***************************QCSPropertyGroupBox**************************************/
QCSPropertyGroupBox::QCSPropertyGroupBox(QWidget *parent) : QGroupBox(parent)
{
}

QCSPropertyGroupBox::~QCSPropertyGroupBox()
{
}

void QCSPropertyGroupBox::SetValues()
{
}

void QCSPropertyGroupBox::GetValues()
{
}

/***************************QCSPropUnknownGB**************************************/
QCSPropUnknownGB::QCSPropUnknownGB(CSPropUnknown *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Unknown"));
	setTitle(tr("Unknown Property"));

	QGridLayout* layout = new QGridLayout();

	Property = new QLineEdit();
	layout->addWidget(new QLabel(tr("Property: ")),0,0);
	layout->addWidget(Property,0,1);

	GetValues();

	setLayout(layout);
}

QCSPropUnknownGB::~QCSPropUnknownGB()
{
}

void QCSPropUnknownGB::SetValues()
{
	clProp->SetProperty(Property->text().toLatin1().data());
}

void QCSPropUnknownGB::GetValues()
{
	Property->setText(clProp->GetProperty().c_str());
}


/***************************QCSPropMaterialGB**************************************/
QCSPropMaterialGB::QCSPropMaterialGB(CSPropMaterial *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Material"));
	setTitle(tr("Material Property"));

	QGridLayout* layout = new QGridLayout();

	GetValues();

	setLayout(layout);
}

QCSPropMaterialGB::~QCSPropMaterialGB()
{
}

void QCSPropMaterialGB::SetValues()
{
}

void QCSPropMaterialGB::GetValues()
{
}

/***************************QCSPropExcitationGB**************************************/
QCSPropExcitationGB::QCSPropExcitationGB(CSPropExcitation *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Excitation"));
	setTitle(tr("Excitation Property"));

	QGridLayout* layout = new QGridLayout();

	Number = new QSpinBox();
	Number->setRange(0,999);
	Number->setEnabled(QCSX_Settings.GetEdit());
	layout->addWidget(new QLabel(tr("Number: ")),0,0);
	layout->addWidget(Number,0,1);

	Type = new QComboBox();
	Type->addItem(tr("Electric field (soft)"));
	Type->addItem(tr("Electric field (hard)"));
	Type->addItem(tr("Magnetic field (soft)"));
	Type->addItem(tr("Magnetic field (hard)"));
	Type->setEnabled(QCSX_Settings.GetEdit());
	layout->addWidget(new QLabel(tr("Type: ")),0,2);
	layout->addWidget(Type,0,3,1,3);
	QObject::connect(Type,SIGNAL(currentIndexChanged(int)),this,SLOT(TypeChanged(int)));

	layout->addWidget(new QLabel(tr("Excitation (X):")),1,0);
	Excitation[0] = new QLineEdit();
	layout->addWidget(Excitation[0],1,1);

	layout->addWidget(new QLabel(tr("Excitation Y:")),1,2);
	Excitation[1] = new QLineEdit();
	layout->addWidget(Excitation[1],1,3);

	layout->addWidget(new QLabel(tr("Excitation Z:")),1,4);
	Excitation[2] = new QLineEdit();
	layout->addWidget(Excitation[2],1,5);

	layout->addWidget(new QLabel(tr("Analytic Fct (X): ")),2,0);
	FctLine[0] = new QLineEdit();
	layout->addWidget(FctLine[0],2,1,1,5);

	layout->addWidget(new QLabel(tr("Analytic Fct (Y): ")),3,0);
	FctLine[1] = new QLineEdit();
	layout->addWidget(FctLine[1],3,1,1,5);

	layout->addWidget(new QLabel(tr("Analytic Fct (Z): ")),4,0);
	FctLine[2] = new QLineEdit();
	layout->addWidget(FctLine[2],4,1,1,5);

	GetValues();

	setLayout(layout);
}

QCSPropExcitationGB::~QCSPropExcitationGB()
{
}

void QCSPropExcitationGB::SetValues()
{
	bool bOk;
	double dVal;
	QString line;

	for (unsigned int i=0;i<3;++i)
	{
		line=Excitation[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) clProp->SetExcitation(dVal,i);
		else clProp->SetExcitation(line.toLatin1().data(),i);
	}
	clProp->SetNumber((unsigned int)Number->value());
	clProp->SetExcitType(Type->currentIndex()+1);
	for (unsigned int i=0;i<3;++i)
		clProp->SetWeightFunction(FctLine[i]->text().toLatin1().data(),i);
}

void QCSPropExcitationGB::GetValues()
{
	std::string line;

	for (unsigned int i=0;i<3;++i)
	{
		line=clProp->GetExcitationString(i);
		if (!line.empty()) Excitation[i]->setText(line.c_str());
		else Excitation[i]->setText(QString("%1").arg(clProp->GetExcitation(i)));
	}

	Number->setValue(clProp->GetNumber());
	Type->setCurrentIndex(clProp->GetExcitType());
	TypeChanged(clProp->GetExcitType());

	for (unsigned int i=0;i<3;++i)
		FctLine[i]->setText(clProp->GetWeightFunction(i).c_str());
}

void QCSPropExcitationGB::TypeChanged(int index)
{
	//enable/disable certain lines, depending on the excitation typez
	switch (index)
	{
	default:
		Excitation[0]->setEnabled(true & QCSX_Settings.GetEdit());
		Excitation[1]->setEnabled(true & QCSX_Settings.GetEdit());
		Excitation[2]->setEnabled(true & QCSX_Settings.GetEdit());
		FctLine[0]->setEnabled(true & QCSX_Settings.GetEdit());
		FctLine[1]->setEnabled(true & QCSX_Settings.GetEdit());
		FctLine[2]->setEnabled(true & QCSX_Settings.GetEdit());
		break;
	}
}

/***************************QCSPropChargeBoxGB**************************************/
QCSPropProbeBoxGB::QCSPropProbeBoxGB(CSPropProbeBox *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Probe Box"));
	setTitle(tr("ProbeBox Property"));

	QGridLayout* layout = new QGridLayout();

	Number = new QSpinBox();
	Number->setRange(0,999);
	Number->setEnabled(QCSX_Settings.GetEdit());
	layout->addWidget(new QLabel(tr("Number: ")),0,0);
	layout->addWidget(Number,0,1);

	GetValues();

	setLayout(layout);
}

QCSPropProbeBoxGB::~QCSPropProbeBoxGB()
{
}

void QCSPropProbeBoxGB::SetValues()
{
	clProp->SetNumber((unsigned int)Number->value());
}

void QCSPropProbeBoxGB::GetValues()
{
	Number->setValue(clProp->GetNumber());
}

/***************************QCSPropResBoxGB**************************************/
QCSPropResBoxGB::QCSPropResBoxGB(CSPropResBox *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Resolution Box"));
	setTitle(tr("ResBox Property"));

	QGridLayout* layout = new QGridLayout();

	Factor = new QSpinBox();
	Factor->setRange(2,16);
	Factor->setEnabled(QCSX_Settings.GetEdit());
	layout->addWidget(new QLabel(tr("Resolution Factor: ")),0,0);
	layout->addWidget(Factor,0,1);

	GetValues();

	setLayout(layout);
}

QCSPropResBoxGB::~QCSPropResBoxGB()
{
}

void QCSPropResBoxGB::SetValues()
{
	clProp->SetResFactor((unsigned int)Factor->value());
}

void QCSPropResBoxGB::GetValues()
{
	Factor->setValue(clProp->GetResFactor());
}


/***************************QCSPropDumpBoxGB**************************************/
QCSPropDumpBoxGB::QCSPropDumpBoxGB(CSPropDumpBox *prop, int SimMode, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	UNUSED(SimMode);

	clProp=prop;
	TypeName=QString(tr("Dump Box"));
	setTitle(tr("Dump Property"));

	QVBoxLayout* layout = new QVBoxLayout();

	GetValues();

	setLayout(layout);
}

QCSPropDumpBoxGB::~QCSPropDumpBoxGB()
{
}

void QCSPropDumpBoxGB::SetValues()
{
}

void QCSPropDumpBoxGB::GetValues()
{
}

