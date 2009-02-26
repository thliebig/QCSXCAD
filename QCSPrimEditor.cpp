#include "QCSPrimEditor.h"

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
		case CSPrimitives::USERDEFINED:
			CSPrimEdit = new QCSPrimUserDefinedLayout(CSPrim->ToUserDefined());
			setWindowTitle(tr("User Defined Primitive Editor"));
			break;
		default:
			setWindowTitle(tr("default Editor"));
			CSPrimEdit = NULL;
			break;	
	};
		
	MainLayout->addWidget(BuildGeneral()); 
	
	QGroupBox* gb = new QGroupBox(tr("Geometrical Properties"));
	if (CSPrimEdit!=NULL) gb->setLayout(CSPrimEdit);

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
	CSPrim->SetProperty(clCS->GetProperty((unsigned int)PropertiesComboBox->currentIndex()));
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
	grid->addWidget(PrioSpinBox,0,2);
	PropertiesComboBox = new QComboBox();
	
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
	QPushButton* reset = new QPushButton(tr("Reset"));
	QPushButton* cancel = new QPushButton(tr("Cancel"));
	
	QObject::connect(ok,SIGNAL(clicked()),this,SLOT(Save()));
	QObject::connect(reset,SIGNAL(clicked()),this,SLOT(Reset()));
	QObject::connect(cancel,SIGNAL(clicked()),this,SLOT(Cancel()));
	
	lay->addWidget(ok);
	lay->addWidget(reset);
	lay->addWidget(cancel);
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
		str=QString(prop->GetName());
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
			case CSProperties::ELECTRODE:
				str+=tr(" (Electrode)");
				break;
			case CSProperties::CHARGEBOX:
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

QCSPrimitiveLayout::QCSPrimitiveLayout(QWidget *parent) : QGridLayout(parent)
{
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


QCSPrimBoxLayout::QCSPrimBoxLayout(CSPrimBox* prim, QWidget *parent) : QCSPrimitiveLayout(parent)
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
	
	GetValues();
}

QCSPrimBoxLayout::~QCSPrimBoxLayout()
{
}

void QCSPrimBoxLayout::SetValues()
{
	ParameterScalar* ps;
	bool bOk;
	double dVal;
	QString line;
	for (size_t i=0; i< 6; ++i)
	{
		ps=clBox->GetCoordPS(i);
		line=Lines[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) ps->SetValue(dVal);
		else ps->SetValue(line.toLatin1().data());
	}	
}

void QCSPrimBoxLayout::GetValues()
{
	ParameterScalar* ps;
	for (size_t i=0; i< 6; ++i)
	{
		ps=clBox->GetCoordPS(i);
		if (ps->GetMode()) Lines[i]->setText(ps->GetString());
		else Lines[i]->setText(QString("%1").arg(ps->GetValue()));
	}
}

QCSPrimSphereLayout::QCSPrimSphereLayout(CSPrimSphere* prim, QWidget *parent) : QCSPrimitiveLayout(parent)
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
	
	GetValues();
}

QCSPrimSphereLayout::~QCSPrimSphereLayout()
{
}

void QCSPrimSphereLayout::SetValues()
{
	ParameterScalar* ps;
	bool bOk;
	double dVal;
	QString line;
	for (size_t i=0; i< 3; ++i)
	{
		ps=clSphere->GetCoordPS(i);
		line=Lines[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) ps->SetValue(dVal);
		else ps->SetValue(line.toLatin1().data());
	}	

	ps=clSphere->GetRadiusPS();
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
		if (ps->GetMode()) Lines[i]->setText(ps->GetString());
		else Lines[i]->setText(QString("%1").arg(ps->GetValue()));
	}
	
	ps=clSphere->GetRadiusPS();
	if (ps->GetMode()) Lines[3]->setText(ps->GetString());
	else Lines[3]->setText(QString("%1").arg(ps->GetValue()));
}

//****Cylinder
QCSPrimCylinderLayout::QCSPrimCylinderLayout(CSPrimCylinder* prim, QWidget *parent) : QCSPrimitiveLayout(parent)
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
	
	GetValues();
}

QCSPrimCylinderLayout::~QCSPrimCylinderLayout()
{
}

void QCSPrimCylinderLayout::SetValues()
{
	ParameterScalar* ps;
	bool bOk;
	double dVal;
	QString line;
	for (size_t i=0; i< 6; ++i)
	{
		ps=clCylinder->GetCoordPS(i);
		line=Lines[i]->text();
		dVal=line.toDouble(&bOk);
		if (bOk) ps->SetValue(dVal);
		else ps->SetValue(line.toLatin1().data());
	}	

	ps=clCylinder->GetRadiusPS();
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
		if (ps->GetMode()) Lines[i]->setText(ps->GetString());
		else Lines[i]->setText(QString("%1").arg(ps->GetValue()));
	}
	ps=clCylinder->GetRadiusPS();
	if (ps->GetMode()) Lines[6]->setText(ps->GetString());
	else Lines[6]->setText(QString("%1").arg(ps->GetValue()));
}

/*****************MultiBox*****/
QCSPrimMultiBoxLayout::QCSPrimMultiBoxLayout(CSPrimMultiBox* prim, QWidget *parent) : QCSPrimitiveLayout(parent)
{
	clMultiBox=prim;
	
	QPushButton *addButton = new QPushButton("Add Box");
	QObject::connect(addButton,SIGNAL(clicked()),this,SLOT(NewBox()));
	addWidget(addButton,0,0);
	
	QPushButton *editButton = new QPushButton("Edit Box");
	QObject::connect(editButton,SIGNAL(clicked()),this,SLOT(EditBox()));
	addWidget(editButton,0,1);

	QPushButton *deleteButton = new QPushButton("Delete Box");
	QObject::connect(deleteButton,SIGNAL(clicked()),this,SLOT(DeleteBox()));
	addWidget(deleteButton,0,2);

	qBoxList = new QListWidget();
	addWidget(qBoxList,1,0,1,3);
	for (unsigned int i=0;i<clMultiBox->GetQtyBoxes();++i)
	{
		qBoxList->addItem(tr("Box #%1").arg(i));
	}

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
//	unsigned int nr=qBoxList->count()
//	clMultiBox->ClearOverlap();
//	qBoxList->addItem(tr("Box #%1").arg(nr));
//	if (item==NULL) clMultiBox->AddBox();
//	else clMultiBox->AddBox(qBoxList->row(item));
}

void QCSPrimMultiBoxLayout::DeleteBox(QListWidgetItem* item)
{
}

void QCSPrimMultiBoxLayout::EditBox(QListWidgetItem* item)
{
}



QCSPrimUserDefinedLayout::QCSPrimUserDefinedLayout(CSPrimUserDefined* prim, QWidget *parent) : QCSPrimitiveLayout(parent)
{
	clUserDef=prim;
	
	int row=0;
	addWidget(new QLabel(tr("Choose Coordinate System")),row++,0);
	CoordSystem = new QComboBox();
	CoordSystem->addItem("Cartesian Coord. System (x,y,z)");
	CoordSystem->addItem("Cartesian & Cylindrical Coord. System (x,y,z,r,a)");
	CoordSystem->addItem("Cartesian & Spherical Coord. System (x,y,z,r,a,t)");
	addWidget(CoordSystem,row++,0);
	
	
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
	
	addWidget(new QLabel(tr("Define Bool-Function for this Primitive in chosen Coord. System:")),row++,0);
	FunctionLine = new QLineEdit();
	addWidget(FunctionLine,row++,0);

	
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
		if (ps->GetMode()) CoordShift[i]->setText(ps->GetString());
		else CoordShift[i]->setText(QString("%1").arg(ps->GetValue()));
	}
}


