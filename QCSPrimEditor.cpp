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

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFormLayout>

#include "QCSPrimEditor.h"
#include "QCSXCAD_Global.h"

#include "CSPrimPoint.h"
#include "CSPrimBox.h"
#include "CSPrimMultiBox.h"
#include "CSPrimSphere.h"
#include "CSPrimSphericalShell.h"
#include "CSPrimCylinder.h"
#include "CSPrimCylindricalShell.h"
#include "CSPrimPolygon.h"
#include "CSPrimLinPoly.h"
#include "CSPrimRotPoly.h"
#include "CSPrimCurve.h"
#include "CSPrimWire.h"
#include "CSPrimUserDefined.h"

QCSPrimEditor::QCSPrimEditor(ContinuousStructure *CS, CSPrimitives* prim, QWidget* parent) : QDialog(parent)
{
	clCS=CS;
	CSPrim=prim;

	MainLayout = new QVBoxLayout();

	switch (CSPrim->GetType())
	{
		case CSPrimitives::BOX:
			CSPrimEdit = new QCSPrimBoxLayout(CSPrim->ToBox());
			setWindowTitle(tr("Box Editor"));
			break;
		case CSPrimitives::MULTIBOX:
			CSPrimEdit = new QCSPrimMultiBoxLayout(CSPrim->ToMultiBox());
			setWindowTitle(tr("Multi-Box Editor"));
			break;
		case CSPrimitives::SPHERE:
			CSPrimEdit = new QCSPrimSphereLayout(CSPrim->ToSphere());
			setWindowTitle(tr("Sphere Editor"));
			break;
		case CSPrimitives::CYLINDER:
			CSPrimEdit = new QCSPrimCylinderLayout(CSPrim->ToCylinder());
			setWindowTitle(tr("Cylinder Editor"));
			break;
		case CSPrimitives::CYLINDRICALSHELL:
			CSPrimEdit = new QCSPrimCylindricalShellLayout(CSPrim->ToCylindricalShell());
			setWindowTitle(tr("CylindricalShell Editor"));
			break;
		case CSPrimitives::POLYGON:
			CSPrimEdit = new QCSPrimPolygonLayout(CSPrim->ToPolygon());
			setWindowTitle(tr("Polygon Editor"));
			break;
		case CSPrimitives::USERDEFINED:
			CSPrimEdit = new QCSPrimUserDefinedLayout(CSPrim->ToUserDefined());
			setWindowTitle(tr("User Defined Primitive Editor"));
			break;
		default:
			setWindowTitle(tr("default Editor"));
			CSPrimEdit = new QCSPrimitiveLayout(CSPrim);
			break;	
	};
		
	MainLayout->addWidget(BuildGeneral()); 
	
	QGroupBox* gb = new QGroupBox(tr("Geometrical Properties"));
	gb->setLayout(CSPrimEdit);

	MainLayout->addWidget(gb);

	MainLayout->addStretch();

	MainLayout->addLayout(BuildButtons());
	
	
	setLayout(MainLayout);
	
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowModality(Qt::ApplicationModal);
}

QCSPrimEditor::~QCSPrimEditor()
{
}

void QCSPrimEditor::Reset()
{
	PrioSpinBox->setValue(CSPrim->GetPriority());
	UpdatePropertyCB();
	CSPrimEdit->GetValues();
}

void QCSPrimEditor::Save()
{
	CSPrim->SetPriority(PrioSpinBox->value());
	CSProperties* prop = clCS->GetProperty((unsigned int)PropertiesComboBox->currentIndex());
	if (prop==NULL)
	{
		std::cerr << __func__ << ": Error, property invalid!" << std::endl;
		reject();
		return;
	}
	if (prop!=CSPrim->GetProperty())
		prop->AddPrimitive(CSPrim);
	CSPrimEdit->SetValues();
	accept();
}

void QCSPrimEditor::Cancel()
{
	reject();
}

QGroupBox* QCSPrimEditor::BuildGeneral()
{
	QGroupBox* box= new QGroupBox("General");
	QGridLayout* grid = new QGridLayout();
	
	grid->addWidget(new QLabel(QString(tr("ID: %1")).arg(CSPrim->GetID())),0,0);
	
	grid->addWidget(new QLabel(tr("Priority: ")),0,1);
	PrioSpinBox = new QSpinBox();
	PrioSpinBox->setRange(-1,1000);
	PrioSpinBox->setValue(CSPrim->GetPriority());	
	PrioSpinBox->setEnabled(QCSX_Settings.GetEdit());
	grid->addWidget(PrioSpinBox,0,2);
	PropertiesComboBox = new QComboBox();
	PropertiesComboBox->setEnabled(QCSX_Settings.GetEdit());
	
	UpdatePropertyCB();
	
	grid->addWidget(new QLabel(QString(tr("Choose Property: "))),1,0);
	grid->addWidget(PropertiesComboBox,1,1,1,2);
	
	box->setLayout(grid);
	
	return box;
}

QLayout* QCSPrimEditor::BuildButtons()
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

void QCSPrimEditor::UpdatePropertyCB()
{	
	PropertiesComboBox->clear();
	for (size_t i=0;i<clCS->GetQtyProperties();++i)
	{
		QString str;
		CSProperties* prop=clCS->GetProperty(i);
		if (prop==NULL) break;
		str=QString(prop->GetName().c_str());
		switch (prop->GetType())
		{
			case CSProperties::UNKNOWN:
				str+=tr(" (Unknown)");
				break;
			case CSProperties::MATERIAL:
				str+=tr(" (Material)");
				break;
			case CSProperties::METAL:
				str+=tr(" (Metal)");
				break;
			case CSProperties::EXCITATION:
				str+=tr(" (Excitation)");
				break;
			case CSProperties::PROBEBOX:
				str+=tr(" (Charge-Box)");
				break;
			case CSProperties::RESBOX:
				str+=tr(" (Res-Box)");
				break;
			case CSProperties::DUMPBOX:
				str+=tr(" (Dump-Box)");
				break;
		};
		PropertiesComboBox->addItem(str);
	}

	CSProperties* prop=CSPrim->GetProperty();
	if (prop==NULL) PropertiesComboBox->setCurrentIndex(0);
	else  PropertiesComboBox->setCurrentIndex(prop->GetID());
}

QCSPrimitiveLayout::QCSPrimitiveLayout(CSPrimitives *prim, QWidget *parent) : QGridLayout(parent)
{
	clPrim = prim;
}

QCSPrimitiveLayout::~QCSPrimitiveLayout()
{
}

void QCSPrimitiveLayout::SetValues()
{
}

void QCSPrimitiveLayout::GetValues()
{
}


QCSPrimBoxLayout::QCSPrimBoxLayout(CSPrimBox* prim, QWidget *parent) : QCSPrimitiveLayout(prim, parent)
{
	clBox=prim;
	
	addWidget(new QLabel(tr("1. Point")),0,0,1,6);
	addWidget(new QLabel("X:"),1,0);
	Lines[0]=new QLineEdit();
	addWidget(Lines[0],1,1);
	addWidget(new QLabel("Y:"),1,2);
	Lines[2]=new QLineEdit();
	addWidget(Lines[2],1,3);
	addWidget(new QLabel("Z:"),1,4);
	Lines[4]=new QLineEdit();
	addWidget(Lines[4],1,5);

	addWidget(new QLabel(tr("2. Point")),2,0,1,6); 
	addWidget(new QLabel("X:"),3,0);
	Lines[1]=new QLineEdit();addWidget(Lines[1],3,1);
	addWidget(new QLabel("Y:"),3,2);
	Lines[3]=new QLineEdit();addWidget(Lines[3],3,3);
	addWidget(new QLabel("Z:"),3,4);
	Lines[5]=new QLineEdit();addWidget(Lines[5],3,5);

	for (int i=0;i<6;++i)
		Lines[i]->setEnabled(QCSX_Settings.GetEdit());
	
	GetValues();
}

QCSPrimBoxLayout::~QCSPrimBoxLayout()
{
}

void QCSPrimBoxLayout::SetValues()
{
	bool bOk;
	double dVal;
	QString line;
	for (size_t i=0; i< 6; ++i)
	{
		line=Lines[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) clBox->SetCoord(i,dVal);
		else clBox->SetCoord(i,line.toStdString());
	}	
}

void QCSPrimBoxLayout::GetValues()
{
	ParameterScalar* ps;
	for (size_t i=0; i< 6; ++i)
	{
		ps=clBox->GetCoordPS(i);
		if (ps->GetMode()) Lines[i]->setText(ps->GetString().c_str());
		else Lines[i]->setText(QString("%1").arg(ps->GetValue()));
	}
}

QCSPrimSphereLayout::QCSPrimSphereLayout(CSPrimSphere* prim, QWidget *parent) : QCSPrimitiveLayout(prim, parent)
{
	clSphere=prim;
	
	addWidget(new QLabel(tr("Center Point")),0,0,1,6);
	addWidget(new QLabel("X:"),1,0);
	Lines[0]=new QLineEdit();addWidget(Lines[0],1,1);
	addWidget(new QLabel("Y:"),1,2);
	Lines[1]=new QLineEdit();addWidget(Lines[1],1,3);
	addWidget(new QLabel("Z:"),1,4);
	Lines[2]=new QLineEdit();addWidget(Lines[2],1,5);
	
	addWidget(new QLabel(tr("Radius")),2,0,1,2); 
	Lines[3]=new QLineEdit();addWidget(Lines[3],2,3,1,4);
	
	for (int i=0;i<4;++i)
		Lines[i]->setEnabled(QCSX_Settings.GetEdit());

	GetValues();
}

QCSPrimSphereLayout::~QCSPrimSphereLayout()
{
}

void QCSPrimSphereLayout::SetValues()
{
	bool bOk;
	double dVal;
	QString line;
	for (size_t i=0; i< 3; ++i)
	{
		line=Lines[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) clSphere->SetCoord(i,dVal);
		else clSphere->SetCoord(i,line.toLatin1().data());
	}	

	ParameterScalar* ps=clSphere->GetRadiusPS();
	line=Lines[3]->text();
	dVal=line.toDouble(&bOk);
	if (bOk) ps->SetValue(dVal);
	else ps->SetValue(line.toLatin1().data());

}

void QCSPrimSphereLayout::GetValues()
{
	ParameterScalar* ps;
	for (size_t i=0; i< 3; ++i)
	{
		ps=clSphere->GetCoordPS(i);
		if (ps->GetMode()) Lines[i]->setText(ps->GetString().c_str());
		else Lines[i]->setText(QString("%1").arg(ps->GetValue()));
	}
	
	ps=clSphere->GetRadiusPS();
	if (ps->GetMode()) Lines[3]->setText(ps->GetString().c_str());
	else Lines[3]->setText(QString("%1").arg(ps->GetValue()));
}

//****Cylinder
QCSPrimCylinderLayout::QCSPrimCylinderLayout(CSPrimCylinder* prim, QWidget *parent) : QCSPrimitiveLayout(prim, parent)
{
	clCylinder=prim;

	addWidget(new QLabel(tr("Start Point")),0,0,1,6);
	addWidget(new QLabel("X:"),1,0);
	Lines[0]=new QLineEdit();addWidget(Lines[0],1,1);
	addWidget(new QLabel("Y:"),1,2);
	Lines[2]=new QLineEdit();addWidget(Lines[2],1,3);
	addWidget(new QLabel("Z:"),1,4);
	Lines[4]=new QLineEdit();addWidget(Lines[4],1,5);

	addWidget(new QLabel(tr("End Point")),2,0,1,6);
	addWidget(new QLabel("X:"),3,0);
	Lines[1]=new QLineEdit();addWidget(Lines[1],3,1);
	addWidget(new QLabel("Y:"),3,2);
	Lines[3]=new QLineEdit();addWidget(Lines[3],3,3);
	addWidget(new QLabel("Z:"),3,4);
	Lines[5]=new QLineEdit();addWidget(Lines[5],3,5);

	addWidget(new QLabel(tr("Radius")),4,0,1,2);
	Lines[6]=new QLineEdit();addWidget(Lines[6],4,3,1,4);

	for (int i=0;i<7;++i)
		Lines[i]->setEnabled(QCSX_Settings.GetEdit());

	GetValues();
}

QCSPrimCylinderLayout::~QCSPrimCylinderLayout()
{
}

void QCSPrimCylinderLayout::SetValues()
{
	bool bOk;
	double dVal;
	QString line;
	for (size_t i=0; i< 6; ++i)
	{
		line=Lines[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) clCylinder->SetCoord(i,dVal);
		else clCylinder->SetCoord(i,line.toLatin1().data());
	}

	ParameterScalar* ps=clCylinder->GetRadiusPS();
	line=Lines[6]->text();
	dVal=line.toDouble(&bOk);
	if (bOk) ps->SetValue(dVal);
	else ps->SetValue(line.toLatin1().data());

}

void QCSPrimCylinderLayout::GetValues()
{
	ParameterScalar* ps;
	for (size_t i=0; i< 6; ++i)
	{
		ps=clCylinder->GetCoordPS(i);
		if (ps->GetMode()) Lines[i]->setText(ps->GetString().c_str());
		else Lines[i]->setText(QString("%1").arg(ps->GetValue()));
	}
	ps=clCylinder->GetRadiusPS();
	if (ps->GetMode()) Lines[6]->setText(ps->GetString().c_str());
	else Lines[6]->setText(QString("%1").arg(ps->GetValue()));
}

//****CylindricalShell
QCSPrimCylindricalShellLayout::QCSPrimCylindricalShellLayout(CSPrimCylindricalShell* prim, QWidget *parent) : QCSPrimCylinderLayout(prim, parent)
{
	clCylindricalShell=prim;

	addWidget(new QLabel(tr("ShellWidth")),5,0,1,2);
	m_ShellWidth=new QLineEdit();addWidget(m_ShellWidth,5,3,1,4);

	for (int i=0;i<8;++i)
		m_ShellWidth->setEnabled(QCSX_Settings.GetEdit());

	GetValues();
}

QCSPrimCylindricalShellLayout::~QCSPrimCylindricalShellLayout()
{
}

void QCSPrimCylindricalShellLayout::SetValues()
{
	QCSPrimCylinderLayout::SetValues();

	bool bOk;
	double dVal;
	QString line;

	ParameterScalar* ps = clCylindricalShell->GetShellWidthPS();
	line = m_ShellWidth->text();
	dVal = line.toDouble(&bOk);
	if (bOk)
		ps->SetValue(dVal);
	else
		ps->SetValue(line.toLatin1().constData());
}

void QCSPrimCylindricalShellLayout::GetValues()
{
	QCSPrimCylinderLayout::GetValues();

	ParameterScalar* ps = clCylindricalShell->GetShellWidthPS();
	if (ps->GetMode())
		m_ShellWidth->setText(ps->GetString().c_str());
	else
		m_ShellWidth->setText(QString("%1").arg(ps->GetValue()));
}

/*****************MultiBox*****/
QCSPrimMultiBoxLayout::QCSPrimMultiBoxLayout(CSPrimMultiBox* prim, QWidget *parent) : QCSPrimitiveLayout(prim, parent)
{
	clMultiBox=prim;
	
	QPushButton *addButton = new QPushButton("Add Box");
	QObject::connect(addButton,SIGNAL(clicked()),this,SLOT(NewBox()));
	addWidget(addButton,0,0);
	addButton->setEnabled(QCSX_Settings.GetEdit());

	QPushButton *editButton = new QPushButton("Edit Box");
	QObject::connect(editButton,SIGNAL(clicked()),this,SLOT(EditBox()));
	addWidget(editButton,0,1);
	editButton->setEnabled(QCSX_Settings.GetEdit());

	QPushButton *deleteButton = new QPushButton("Delete Box");
	QObject::connect(deleteButton,SIGNAL(clicked()),this,SLOT(DeleteBox()));
	addWidget(deleteButton,0,2);
	deleteButton->setEnabled(QCSX_Settings.GetEdit());

	qBoxList = new QListWidget();
	addWidget(qBoxList,1,0,1,3);
	for (unsigned int i=0;i<clMultiBox->GetQtyBoxes();++i)
	{
		qBoxList->addItem(tr("Box #%1").arg(i));
	}
	qBoxList->setEnabled(QCSX_Settings.GetEdit());

	GetValues();
}

QCSPrimMultiBoxLayout::~QCSPrimMultiBoxLayout()
{
}

void QCSPrimMultiBoxLayout::SetValues()
{


}

void QCSPrimMultiBoxLayout::GetValues()
{

}

void QCSPrimMultiBoxLayout::NewBox(QListWidgetItem* item)
{
	UNUSED(item);
//	unsigned int nr=qBoxList->count()
//	clMultiBox->ClearOverlap();
//	qBoxList->addItem(tr("Box #%1").arg(nr));
//	if (item==NULL) clMultiBox->AddBox();
//	else clMultiBox->AddBox(qBoxList->row(item));
}

void QCSPrimMultiBoxLayout::DeleteBox(QListWidgetItem* item)
{
	UNUSED(item);
}

void QCSPrimMultiBoxLayout::EditBox(QListWidgetItem* item)
{
	UNUSED(item);
}

//***********************************************************************************//
QCSPrimPolygonLayout::QCSPrimPolygonLayout(CSPrimPolygon* prim, QWidget *parent) : QCSPrimitiveLayout(prim, parent)
{
	clPoly=prim;
	
	addWidget(new QLabel(tr("Polygon Plane")),0,0);	
	NormVec = new QComboBox();
	NormVec->addItem(tr("yz-plane"));
	NormVec->addItem(tr("zx-plane"));
	NormVec->addItem(tr("xy-plane"));
	QObject::connect(NormVec,SIGNAL(currentIndexChanged(int)),this,SLOT(NormVecChanged()));
	addWidget(NormVec,0,1);
	NormVec->setEnabled(QCSX_Settings.GetEdit());
	
	addWidget(new QLabel(tr("Polygon Elevation")),1,0);	
	Elevation = new QLineEdit();;
	addWidget(Elevation,1,1);
	Elevation->setEnabled(QCSX_Settings.GetEdit());

	QGroupBox* gb = new QGroupBox(tr("Polygon Vertices"));
	QFormLayout* gbl = new QFormLayout();
	gb->setLayout(gbl);
	addWidget(gb,2,0,1,2);
	
	CoordLineX = new QLineEdit();
	gbl->addRow(tr("X_1"),CoordLineX);
	CoordLineX->setEnabled(QCSX_Settings.GetEdit());
	CoordLineY = new QLineEdit();
	gbl->addRow(tr("X_2"),CoordLineY);
	CoordLineY->setEnabled(QCSX_Settings.GetEdit());

	GetValues();
}

QCSPrimPolygonLayout::~QCSPrimPolygonLayout()
{
}

void QCSPrimPolygonLayout::SetValues()
{
	int ind = NormVec->currentIndex();
	clPoly->SetNormDir(ind);
	ParameterScalar* ps;
	bool bOk;
	double dVal;
	QString line;
	
	ps=clPoly->GetElevationPS();
	line=Elevation->text();
	dVal=line.toDouble(&bOk);
	if (bOk) ps->SetValue(dVal);
	else ps->SetValue(line.toStdString());
	
	clPoly->ClearCoords();
	
	QStringList strListX = CoordLineX->text().split(",",QString::SkipEmptyParts);
	QStringList strListY = CoordLineY->text().split(",",QString::SkipEmptyParts);
	
	for (int i=0; (i<strListX.size()) && (i<strListY.size()) ; ++i)
	{

		QString x1 = strListX.at(i); 
		QString x2 = strListY.at(i); 
		
		if (x1.isEmpty() && x2.isEmpty()) return;
		
		dVal=x1.toDouble(&bOk);
		if (bOk) clPoly->AddCoord(dVal);
		else clPoly->AddCoord(line.toStdString());

		dVal=x2.toDouble(&bOk);
		if (bOk) clPoly->AddCoord(dVal);
		else clPoly->AddCoord(line.toStdString());
	}	
}

void QCSPrimPolygonLayout::GetValues()
{
	NormVec->setCurrentIndex(clPoly->GetNormDir());
	ParameterScalar* ps;
	ps=clPoly->GetElevationPS();
	if (ps->GetMode()) Elevation->setText(ps->GetString().c_str());
	else Elevation->setText(QString("%1").arg(ps->GetValue()));

	QStringList strListX;
	QStringList strListY;
	
	for (size_t i=0; i<clPoly->GetQtyCoords(); ++i)
	{
		ps=clPoly->GetCoordPS(2*i);
		if (ps)
		{
			if (ps->GetMode()) strListX.append(ps->GetString().c_str());
			else strListX.append(QString("%1").arg(ps->GetValue()));
		}

		ps=clPoly->GetCoordPS(2*i+1);
		if (ps)
		{
			if (ps->GetMode()) strListY.append(ps->GetString().c_str());
			else strListY.append(QString("%1").arg(ps->GetValue()));
		}
	}
	CoordLineX->setText(strListX.join(", "));
	CoordLineY->setText(strListY.join(", "));
}

void QCSPrimPolygonLayout::NormVecChanged()
{
//	QStringList headers;
//	switch (NormVec->currentIndex())
//	{
//	case 0:
//		headers << "x" << "y";
//		break;
//	case 1:
//		headers << "y" << "z";
//		break;
//	case 2:
//		headers << "z" << "x";
//		break;
//	default:
//		headers << "x" << "y";
//		break;
//	}
//	CoordTable->setHorizontalHeaderLabels(headers);
}

//***********************************************************************************//
QCSPrimUserDefinedLayout::QCSPrimUserDefinedLayout(CSPrimUserDefined* prim, QWidget *parent) : QCSPrimitiveLayout(prim, parent)
{
	clUserDef=prim;
	
	int row=0;
	addWidget(new QLabel(tr("Choose Coordinate System")),row++,0);
	CoordSystem = new QComboBox();
	CoordSystem->addItem("Cartesian Coord. System (x,y,z)");
	CoordSystem->addItem("Cartesian & Cylindrical Coord. System (x,y,z,r,a)");
	CoordSystem->addItem("Cartesian & Spherical Coord. System (x,y,z,r,a,t)");
	addWidget(CoordSystem,row++,0);
	CoordSystem->setEnabled(QCSX_Settings.GetEdit());
	
	QGroupBox* SysShiftGrp = new QGroupBox(tr("Shift Coordinate System"));
	addWidget(SysShiftGrp,row++,0);
	
	QGridLayout* lay = new QGridLayout;
	SysShiftGrp->setLayout(lay);
	
	lay->addWidget(new QLabel(tr("X-Shift:")),0,0);
	CoordShift[0]=new QLineEdit();
	lay->addWidget(CoordShift[0],0,1);
	lay->addWidget(new QLabel(tr("Y-Shift:")),1,0);
	CoordShift[1]=new QLineEdit();
	lay->addWidget(CoordShift[1],1,1);
	lay->addWidget(new QLabel(tr("Z-Shift:")),2,0);
	CoordShift[2]=new QLineEdit();
	lay->addWidget(CoordShift[2],2,1);
	for (int i=0;i<3;++i)
		CoordShift[i]->setEnabled(QCSX_Settings.GetEdit());
	
	addWidget(new QLabel(tr("Define Bool-Function for this Primitive in chosen Coord. System:")),row++,0);
	FunctionLine = new QLineEdit();
	addWidget(FunctionLine,row++,0);
	FunctionLine->setEnabled(QCSX_Settings.GetEdit());
	
	GetValues();
}

QCSPrimUserDefinedLayout::~QCSPrimUserDefinedLayout()
{
}

void QCSPrimUserDefinedLayout::SetValues()
{
	clUserDef->SetCoordSystem((CSPrimUserDefined::UserDefinedCoordSystem)CoordSystem->currentIndex());
	clUserDef->SetFunction(FunctionLine->text().toStdString().c_str());

	bool bOk;
	double dVal;
	QString line;
	ParameterScalar* ps;
	for (size_t i=0; i< 3; ++i)
	{
		ps=clUserDef->GetCoordShiftPS(i);
		line=CoordShift[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) ps->SetValue(dVal);
		else ps->SetValue(line.toLatin1().data());
	}	
}

void QCSPrimUserDefinedLayout::GetValues()
{
	CoordSystem->setCurrentIndex(clUserDef->GetCoordSystem());
	FunctionLine->setText(clUserDef->GetFunction());
	
	ParameterScalar* ps;
	for (size_t i=0; i< 3; ++i)
	{
		ps=clUserDef->GetCoordShiftPS(i);
		if (ps==NULL) return;
		if (ps->GetMode()) CoordShift[i]->setText(ps->GetString().c_str());
		else CoordShift[i]->setText(QString("%1").arg(ps->GetValue()));
	}
}


