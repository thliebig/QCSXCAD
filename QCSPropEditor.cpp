#include "QCSPropEditor.h"
#include "iostream"
#include "time.h";

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
		case CSProperties::ELECTRODE:
			propGB = new QCSPropElectrodeGB(clProp->ToElectrode());
			break;
		case CSProperties::CHARGEBOX:
			propGB = new QCSPropChargeBoxGB(clProp->ToChargeBox());
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
		case CSProperties::ELECTRODE:
			clProp = new CSPropElectrode(saveProp);
			break;
		case CSProperties::CHARGEBOX:
			clProp = new CSPropChargeBox(saveProp);
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

	Name = new QLineEdit(QString(clProp->GetName()));
	grid->addWidget(new QLabel(tr("Name: ")),0,0);
	grid->addWidget(Name,0,1);
	grid->addWidget(new QLabel(QString(tr("ID: %1")).arg(clProp->GetID())),0,2);

	grid->addWidget(new QLabel(tr("Type: ")),1,0);
	TypeCB = new QComboBox();
	TypeCB->addItem(tr("Unknown"),QVariant(CSProperties::UNKNOWN));
	TypeCB->addItem(tr("Material"),QVariant(CSProperties::MATERIAL));
	TypeCB->addItem(tr("Metal"),QVariant(CSProperties::METAL));
	TypeCB->addItem(tr("Electrode"),QVariant(CSProperties::ELECTRODE));
	TypeCB->addItem(tr("Probe Box"),QVariant(CSProperties::CHARGEBOX));
	TypeCB->addItem(tr("Res Box"),QVariant(CSProperties::RESBOX));
	TypeCB->addItem(tr("Dump Box"),QVariant(CSProperties::DUMPBOX));
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

void QCSPropEditor::GetValues()
{
	if (clProp!=saveProp)
	{
		delete clProp;
		delete propGB;
		clProp=saveProp;
	}

	propGB = BuildPropGroupBox(clProp);

	Name->setText(QString(clProp->GetName()));
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
	Property->setText(clProp->GetProperty());
}


/***************************QCSPropMaterialGB**************************************/
QCSPropMaterialGB::QCSPropMaterialGB(CSPropMaterial *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Material"));
	setTitle(tr("Material Property"));

	QGridLayout* layout = new QGridLayout();

	EpsLine = new QLineEdit();
	layout->addWidget(new QLabel(tr("Epsilon: ")),0,0);
	layout->addWidget(EpsLine,0,1);

	MueLine = new QLineEdit();
	layout->addWidget(new QLabel(tr("Mue: ")),1,0);
	layout->addWidget(MueLine,1,1);

	KappaLine = new QLineEdit();
	layout->addWidget(new QLabel(tr("Kappa: ")),2,0);
	layout->addWidget(KappaLine,2,1);

	GetValues();

	setLayout(layout);
}

QCSPropMaterialGB::~QCSPropMaterialGB()
{
}

void QCSPropMaterialGB::SetValues()
{
	bool bOk;
	double dVal;
	QString line;

	line=EpsLine->text();
	dVal=line.toDouble(&bOk);
	if (bOk) clProp->SetEpsilon(dVal);
	else clProp->SetEpsilon(line.toLatin1().data());

	line=MueLine->text();
	dVal=line.toDouble(&bOk);
	if (bOk) clProp->SetMue(dVal);
	else clProp->SetMue(line.toLatin1().data());

	line=KappaLine->text();
	dVal=line.toDouble(&bOk);
	if (bOk) clProp->SetKappa(dVal);
	else clProp->SetKappa(line.toLatin1().data());

}

void QCSPropMaterialGB::GetValues()
{
	const char* line = NULL;
	line=clProp->GetEpsilonTerm();
	if (line!=NULL) EpsLine->setText(line);
	else EpsLine->setText(QString("%1").arg(clProp->GetEpsilon()));

	line=clProp->GetMueTerm();
	if (line!=NULL) MueLine->setText(line);
	else MueLine->setText(QString("%1").arg(clProp->GetMue()));

	line=clProp->GetKappaTerm();
	if (line!=NULL) KappaLine->setText(line);
	else KappaLine->setText(QString("%1").arg(clProp->GetKappa()));

}

/***************************QCSPropElectrodeGB**************************************/
QCSPropElectrodeGB::QCSPropElectrodeGB(CSPropElectrode *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Electrode"));
	setTitle(tr("Electrode Property"));

	QGridLayout* layout = new QGridLayout();

	Number = new QSpinBox();
	Number->setRange(0,999);
	layout->addWidget(new QLabel(tr("Number: ")),0,0);
	layout->addWidget(Number,0,1);

	Type = new QComboBox();
	Type->addItem(tr("Const. Potential (e.g. Voltage)"));
	Type->addItem(tr("Const. Charge (Current)"));
	Type->addItem(tr("Var. Potential (e.g. Voltage)"));
	Type->addItem(tr("Var. Charge (Current)"));
	Type->addItem(tr("Const. Polarisation"));
	Type->addItem(tr("Var. Polarisation"));
	Type->addItem(tr("Const. Vector (e.g. Current-Dens.)"));
	Type->addItem(tr("Var. Vector (e.g. Current-Dens.)"));
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

//	layout->addWidget(new QLabel(tr("Analytic Vars (X): ")),2,3);
//	VarLine[0] = new QLineEdit();
//	layout->addWidget(VarLine[0],2,4,1,2);

	layout->addWidget(new QLabel(tr("Analytic Fct (Y): ")),3,0);
	FctLine[1] = new QLineEdit();
	layout->addWidget(FctLine[1],3,1,1,5);

//	layout->addWidget(new QLabel(tr("Analytic Vars (Y): ")),3,3);
//	VarLine[1] = new QLineEdit();
//	layout->addWidget(VarLine[1],3,4,1,2);

	layout->addWidget(new QLabel(tr("Analytic Fct (Z): ")),4,0);
	FctLine[2] = new QLineEdit();
	layout->addWidget(FctLine[2],4,1,1,5);

//	layout->addWidget(new QLabel(tr("Analytic Vars (Z): ")),4,3);
//	VarLine[2] = new QLineEdit();
//	layout->addWidget(VarLine[2],2,4,1,2);
	GetValues();

	setLayout(layout);
}

QCSPropElectrodeGB::~QCSPropElectrodeGB()
{
}

void QCSPropElectrodeGB::SetValues()
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
	{
		clProp->SetWeightFct(FctLine[i]->text().toLatin1().data(),i);
		//clProp->SetWeightVars(VarLine[i]->text().toLatin1().data(),i);
	}
}

void QCSPropElectrodeGB::GetValues()
{
	const char* line = NULL;

	for (unsigned int i=0;i<3;++i)
	{
		line=clProp->GetExcitationString(i);
		if (line!=NULL) Excitation[i]->setText(line);
		else Excitation[i]->setText(QString("%1").arg(clProp->GetExcitation(i)));
	}

	Number->setValue(clProp->GetNumber());
	Type->setCurrentIndex(clProp->GetExcitType()-1);
	TypeChanged(clProp->GetExcitType()-1);

	for (unsigned int i=0;i<3;++i)
	{
		FctLine[i]->setText(clProp->GetWeightFct(i));
		//VarLine[i]->setText(clProp->GetWeightVars(i));
	}
}

void QCSPropElectrodeGB::TypeChanged(int index)
{
	switch (index)
	{
	default:
		Excitation[1]->setEnabled(false);
		Excitation[2]->setEnabled(false);
		FctLine[0]->setEnabled(false);
		//VarLine[0]->setEnabled(true);
		FctLine[1]->setEnabled(false);
		//VarLine[1]->setEnabled(false);
		FctLine[2]->setEnabled(false);
		//VarLine[2]->setEnabled(false);
		break;
	case 2:
	case 3:
		Excitation[1]->setEnabled(false);
		Excitation[2]->setEnabled(false);
		FctLine[0]->setEnabled(true);
		//VarLine[0]->setEnabled(true);
		FctLine[1]->setEnabled(false);
		//VarLine[1]->setEnabled(false);
		FctLine[2]->setEnabled(false);
		//VarLine[2]->setEnabled(false);
		break;
	case 4:
	case 6:
		Excitation[1]->setEnabled(true);
		Excitation[2]->setEnabled(true);
		FctLine[0]->setEnabled(false);
		//VarLine[0]->setEnabled(false);
		FctLine[1]->setEnabled(false);
		//VarLine[1]->setEnabled(false);
		FctLine[2]->setEnabled(false);
		//VarLine[2]->setEnabled(false);
		break;
	case 5:
	case 7:
		Excitation[1]->setEnabled(true);
		Excitation[2]->setEnabled(true);
		FctLine[0]->setEnabled(true);
		//VarLine[0]->setEnabled(true);
		FctLine[1]->setEnabled(true);
		//VarLine[1]->setEnabled(true);
		FctLine[2]->setEnabled(true);
		//VarLine[2]->setEnabled(true);
		break;
	}
}

/***************************QCSPropChargeBoxGB**************************************/
QCSPropChargeBoxGB::QCSPropChargeBoxGB(CSPropChargeBox *prop, QWidget *parent) : QCSPropertyGroupBox(parent)
{
	clProp=prop;
	TypeName=QString(tr("Probe Box"));
	setTitle(tr("ProbeBox Property"));

	QGridLayout* layout = new QGridLayout();

	Number = new QSpinBox();
	Number->setRange(0,999);
	layout->addWidget(new QLabel(tr("Number: ")),0,0);
	layout->addWidget(Number,0,1);

	GetValues();

	setLayout(layout);
}

QCSPropChargeBoxGB::~QCSPropChargeBoxGB()
{
}

void QCSPropChargeBoxGB::SetValues()
{
	clProp->SetNumber((unsigned int)Number->value());
}

void QCSPropChargeBoxGB::GetValues()
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
	clProp=prop;
	TypeName=QString(tr("Dump Box"));
	setTitle(tr("Dump Property"));

	QVBoxLayout* layout = new QVBoxLayout();

	GlobSet = new QCheckBox(tr("Use Global Dump Settings"));
	layout->addWidget(GlobSet);

	QGroupBox* ScalarGB = new QGroupBox(tr("Scalar Field Dump Options"));
	QVBoxLayout* scalLay = new QVBoxLayout();
	DumpPhi = new QCheckBox(tr("Phi Dump"));
	scalLay->addWidget(DumpPhi);
	DumpDivE = new QCheckBox(tr("div(E) Dump"));
	scalLay->addWidget(DumpDivE);
	DumpDivD = new QCheckBox(tr("div(D) Dump"));
	scalLay->addWidget(DumpDivD);
	DumpDivP = new QCheckBox(tr("div(P) Dump"));
	scalLay->addWidget(DumpDivP);
	DumpFieldW = new QCheckBox(tr("Field Energy Dump (1/2*E*D*dV)"));
	scalLay->addWidget(DumpFieldW);
	DumpChargeW = new QCheckBox(tr("Charge Energy Dump (1/2*Phi*div(D)*dV)"));
	scalLay->addWidget(DumpChargeW);
	ScalarGB->setLayout(scalLay);
	layout->addWidget(ScalarGB);

	QGroupBox* VectorGB = new QGroupBox(tr("Vector Field Dump Options"));
	QVBoxLayout* vecLay = new QVBoxLayout();
	DumpEField = new QCheckBox(tr("E-Field Dump"));
	vecLay->addWidget(DumpEField);
	DumpDField = new QCheckBox(tr("D-Field Dump"));
	vecLay->addWidget(DumpDField);
	DumpPField = new QCheckBox(tr("P-Field Dump"));
	vecLay->addWidget(DumpPField);
	VectorGB->setLayout(vecLay);
	layout->addWidget(VectorGB);

	//cout << "SimMode: " << SimMode << endl;
	switch (SimMode)
	{
	case 1:
		DumpDivE->setText(tr("div(H) Dump"));
		DumpDivD->setText(tr("div(B) Dump"));
		DumpDivP->setText(tr("div(M) Dump"));
		DumpFieldW->setText(tr("Field Energy (1/2*H*B*dV)"));
		DumpChargeW->hide();
		DumpEField->setText(tr("H-Field"));
		DumpDField->setText(tr("B-Field"));
		DumpPField->setText(tr("M-Field"));
		break;
	case 2:
		DumpDivE->setText(tr("div(E) Dump"));
		DumpDivD->setText(tr("div(J) Dump"));
		DumpDivP->hide();
		DumpFieldW->setText(tr("dissipation loss (1/2*E*J*dV)"));
		DumpChargeW->hide();
		DumpEField->setText(tr("E-Field"));
		DumpDField->setText(tr("J-Field"));
		DumpPField->hide();
		break;
	case 3:
		DumpDivE->hide();
		DumpDivD->hide();
		DumpDivP->hide();
		DumpFieldW->setText(tr("Field Energy (1/2*H*B*dV)"));
		DumpChargeW->hide();
		DumpEField->setText(tr("H-Field"));
		DumpDField->setText(tr("B-Field"));
		DumpPField->setText(tr("M-Field"));
		break;
	}

	QGroupBox* SubGridGB = new QGroupBox(tr("Sub-Grid Dump Options"));
	SubGridGB->setToolTip(tr("only possible with active Sub-Grids and Sub-Grid-Postprocessing"));
	QVBoxLayout* SGLay = new QVBoxLayout();
	DumpSubGrid = new QCheckBox(tr("Use Sub Grid's for Dump - max Level:"));
	SGLay->addWidget(DumpSubGrid);
	SGLevel = new QSpinBox();
	SGLevel->setMinimum(-1);
	SGLay->addWidget(SGLevel);
	SimpleGrid = new QCheckBox(tr("User only simple Dump Grid! (Rectilinear Grid)"));
	SGLay->addWidget(SimpleGrid);
	SimpleGrid->hide(); //not yet used
	SubGridGB->setLayout(SGLay);
	layout->addWidget(SubGridGB);

	QObject::connect(GlobSet,SIGNAL(toggled(bool)),ScalarGB,SLOT(setDisabled(bool)));
	QObject::connect(GlobSet,SIGNAL(toggled(bool)),VectorGB,SLOT(setDisabled(bool)));

	GetValues();

	setLayout(layout);
}

QCSPropDumpBoxGB::~QCSPropDumpBoxGB()
{
}

void QCSPropDumpBoxGB::SetValues()
{
	clProp->SetGlobalSetting(GlobSet->isChecked());
	clProp->SetPhiDump(DumpPhi->isChecked());
	clProp->SetDivEDump(DumpDivE->isChecked());
	clProp->SetDivDDump(DumpDivD->isChecked());
	clProp->SetDivPDump(DumpDivP->isChecked());
	clProp->SetFieldWDump(DumpFieldW->isChecked());
	clProp->SetChargeWDump(DumpChargeW->isChecked());
	clProp->SetEFieldDump(DumpEField->isChecked());
	clProp->SetDFieldDump(DumpDField->isChecked());
	clProp->SetPFieldDump(DumpPField->isChecked());
	clProp->SetSGDump(DumpSubGrid->isChecked());
	clProp->SetSimpleDump(SimpleGrid->isChecked());
	clProp->SetSGLevel(SGLevel->value());
}

void QCSPropDumpBoxGB::GetValues()
{
	GlobSet->setChecked(clProp->GetGlobalSetting());
	DumpPhi->setChecked(clProp->GetPhiDump());
	DumpDivE->setChecked(clProp->GetDivEDump());
	DumpDivD->setChecked(clProp->GetDivDDump());
	DumpDivP->setChecked(clProp->GetDivPDump());
	DumpFieldW->setChecked(clProp->GetFieldWDump());
	DumpChargeW->setChecked(clProp->GetChargeWDump());
	DumpEField->setChecked(clProp->GetEFieldDump());
	DumpDField->setChecked(clProp->GetDFieldDump());
	DumpPField->setChecked(clProp->GetPFieldDump());
	DumpSubGrid->setChecked(clProp->GetSGDump());
	SimpleGrid->setChecked(clProp->GetSimpleDump());
	SGLevel->setValue(clProp->GetSGLevel());
}

