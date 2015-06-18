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

#include <QGridLayout>
#include <QPushButton>
#include <QDialog>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QTextEdit>

#include "QCSGridEditor.h"
#include "CSRectGrid.h"
#include "QCSXCAD_Global.h"

QCSGridEditor::QCSGridEditor(CSRectGrid* grid, QWidget* parent) : QWidget(parent)
{
	clGrid=grid;
	QGridLayout* lay = new QGridLayout();
	setLayout(lay);

	//lay->addWidget(new QLabel(tr("Direction")),0,0);
	lay->addWidget(new QLabel(tr("Min")),0,1);
	lay->addWidget(new QLabel(tr("Max")),0,2);
	lay->addWidget(new QLabel(tr("Lines")),0,3);

	for (int n=0;n<3;++n)
	{
		m_DirNames[n] = new QLabel(GetDirName(n));
		lay->addWidget(m_DirNames[n],n+1,0);
	}

	QLabel* label=NULL;
	for (unsigned int i=0;i<6;++i)
	{
		label=new QLabel("0");
		lay->addWidget(label,i/2+1,i%2+1);
		lay->setAlignment(label,Qt::AlignCenter);
		SimBox.append(label);
	}

	for (unsigned int i=0;i<3;++i)
	{
		label=new QLabel("0");
		lay->addWidget(label,i+1,3);
		lay->setAlignment(label,Qt::AlignCenter);
		NodeQty.append(label);
	}

	QString EditText = tr("Edit");
	if (QCSX_Settings.GetEdit()==false)
		EditText = tr("View");
	QPushButton* PB = new QPushButton(QIcon(":/images/edit.png"),EditText);
	QObject::connect(PB,SIGNAL(clicked()),this,SLOT(EditX()));
	lay->addWidget(PB,1,4);

	PB = new QPushButton(QIcon(":/images/edit.png"),EditText);
	QObject::connect(PB,SIGNAL(clicked()),this,SLOT(EditY()));
	lay->addWidget(PB,2,4);

	PB = new QPushButton(QIcon(":/images/edit.png"),EditText);
	QObject::connect(PB,SIGNAL(clicked()),this,SLOT(EditZ()));
	lay->addWidget(PB,3,4);

	lay->addWidget(new QLabel(tr("Drawing unit [m]:")),4,0,1,2);
	UnitLength = new QLineEdit("1");
	UnitLength->setEnabled(QCSX_Settings.GetEdit());
	QObject::connect(UnitLength,SIGNAL(textEdited(QString)),this,SLOT(SetDeltaUnit(QString)));
	lay->addWidget(UnitLength,4,2,1,3);

	OpacitySlider = new QSlider(Qt::Horizontal);
	OpacitySlider->setRange(0,255);
	QObject::connect(OpacitySlider,SIGNAL(valueChanged(int)),this,SIGNAL(OpacityChange(int)));
	lay->addWidget(new QLabel(tr("Grid opacity")),5,0,1,3);
	lay->addWidget(OpacitySlider,5,2,1,3);

	lay->setRowStretch(6,1);
	lay->setColumnStretch(5,1);
}

QCSGridEditor::~QCSGridEditor()
{
}

QToolBar* QCSGridEditor::BuildToolbar()
{
	QToolBar *TB = new QToolBar(tr("Discretisation"));
	TB->setObjectName("Discretisation_ToolBar");
	TB->addAction(tr("homo\nDisc"),this,SLOT(BuildHomogenDisc()));
	TB->addAction(tr("inhomo\nDisc"),this,SLOT(BuildInHomogenDisc()));
	TB->addAction(tr("increase \nDisc"),this,SLOT(IncreaseResolution()));
	TB->addAction(tr("detect \nedges"),this,SLOT(DetectEdges()));
	return TB;
}

QWidget* QCSGridEditor::BuildPlanePosWidget()
{
	QWidget* PPWid = new QWidget();
	QGridLayout* lay = new QGridLayout();

	for (int n=0;n<3;++n)
	{
		m_NormNames[n] = new QLabel(GetNormName(n)+ tr(" plane:"));
		lay->addWidget( m_NormNames[n] ,n,0);
		m_PlanePos[n] = new QSlider();
		m_PlanePos[n]->setOrientation(Qt::Horizontal);
		lay->addWidget(m_PlanePos[n],n,1);
		m_PlanePosValue[n] = new QLabel();
		lay->addWidget(m_PlanePosValue[n],n,2);
	}
	QObject::connect(m_PlanePos[0],SIGNAL(valueChanged(int)),this,SIGNAL(GridPlaneXChanged(int)));
	QObject::connect(m_PlanePos[1],SIGNAL(valueChanged(int)),this,SIGNAL(GridPlaneYChanged(int)));
	QObject::connect(m_PlanePos[2],SIGNAL(valueChanged(int)),this,SIGNAL(GridPlaneZChanged(int)));

	QObject::connect(m_PlanePos[0],SIGNAL(valueChanged(int)),this,SLOT(SetGridPlaneX(int)));
	QObject::connect(m_PlanePos[1],SIGNAL(valueChanged(int)),this,SLOT(SetGridPlaneY(int)));
	QObject::connect(m_PlanePos[2],SIGNAL(valueChanged(int)),this,SLOT(SetGridPlaneZ(int)));

	PPWid->setLayout(lay);
	return PPWid;
}

void QCSGridEditor::SetGridPlaneX(int pos)
{
	m_PlanePosValue[0]->setText(QString("%1 = %2").arg(GetDirName(0)).arg(clGrid->GetLine(0,pos)));
}

void QCSGridEditor::SetGridPlaneY(int pos)
{
	m_PlanePosValue[1]->setText(QString("%1 = %2").arg(GetDirName(1)).arg(clGrid->GetLine(1,pos)));
}

void QCSGridEditor::SetGridPlaneZ(int pos)
{
	m_PlanePosValue[2]->setText(QString("%1 = %2").arg(GetDirName(2)).arg(clGrid->GetLine(2,pos)));
}

void QCSGridEditor::BuildInHomogenDisc()
{
	QDialog* HomogenDisc = new QDialog();
	QGridLayout* lay = new QGridLayout();

	lay->addWidget(new QLabel(tr("Create inhomogenous discretisation:")),0,0,1,3);

	QCheckBox* DelOldGrid = new QCheckBox(tr("Clear old mesh"));
	DelOldGrid->setChecked(true);
	lay->addWidget(DelOldGrid,0,5);

	lay->addWidget(new QLabel(tr("min")),1,1);
	lay->addWidget(new QLabel(tr("max")),1,2);
	lay->addWidget(new QLabel(tr("lines")),1,3);
	lay->addWidget(new QLabel(tr("grid distribution function:")),1,4,1,2);
//	lay->addWidget(new QLabel(tr("X")),2,0);
//	lay->addWidget(new QLabel(tr("Y")),3,0);
//	lay->addWidget(new QLabel(tr("Z")),4,0);
	QLineEdit* function[3];
	function[0] = new QLineEdit(GetDirName(0));
	function[1] = new QLineEdit(GetDirName(1));
	function[2] = new QLineEdit(GetDirName(2));
	lay->addWidget(new QLabel("X:"),2,0);
	lay->addWidget(new QLabel("f(x)="),2,4);
	lay->addWidget(function[0],2,5);
	lay->addWidget(new QLabel("Y:"),3,0);
	lay->addWidget(new QLabel("f(y)="),3,4);
	lay->addWidget(function[1],3,5);
	lay->addWidget(new QLabel("Z:"),4,0);
	lay->addWidget(new QLabel("f(z)="),4,4);
	lay->addWidget(function[2],4,5);
	QLineEdit* BoxLine[6];
	QSpinBox* Nodes[3];
	for (int i=0;i<3;++i)
	{
		BoxLine[2*i]= new QLineEdit(QString("0"));
		lay->addWidget(BoxLine[2*i],i+2,1);
		BoxLine[2*i+1]= new QLineEdit(QString("1"));
		lay->addWidget(BoxLine[2*i+1],i+2,2);
		Nodes[i] = new QSpinBox();
		Nodes[i]->setRange(0,9999);
		Nodes[i]->setValue(0);
		lay->addWidget(Nodes[i],i+2,3);
	}

	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),HomogenDisc,SLOT(accept()));
	QPushButton* PBcancel = new QPushButton(tr("Cancel"));
	QObject::connect(PBcancel,SIGNAL(clicked()),HomogenDisc,SLOT(reject()));

	QHBoxLayout* hLay = new QHBoxLayout();
	hLay->addStretch(1);
	hLay->addWidget(PB);
	hLay->addWidget(PBcancel);
	hLay->addStretch(1);

	lay->addLayout(hLay,5,0,1,6);

	HomogenDisc->setLayout(lay);
	if (HomogenDisc->exec()==QDialog::Accepted)
	{
		double dSimBox[6];
		for (int i=0;i<6;++i) dSimBox[i]=BoxLine[i]->text().toDouble();
		for (int i=0;i<3;++i)
		{
//			if ((Check[i]->checkState()==Qt::Checked))// && (dSimBox[2*i+1]>dSimBox[2*i]))
//			{
//				FunctionParser fParse;
//				fParse.AddConstant("pi", 3.1415926535897932);
//				fParse.Parse(function[i]->text().toStdString(),coordVars.at(i).toStdString());
//				if (fParse.GetParseErrorType()!=FunctionParser::FP_NO_ERROR)
//					QMessageBox::warning(HomogenDisc,tr("Error reading grid function!"),QString(tr("fparser error message:\n%1").arg(fParse.ErrorMsg())));
//				else
//				{
//					double dStep=0;
//					if (Nodes[i]->value()>1) dStep=(dSimBox[2*i+1]-dSimBox[2*i])/(Nodes[i]->value()-1);
//					if (DelOldGrid->isChecked()) clGrid->ClearLines(i);
//					double dValue=0;
//					double dPos=0;
//					bool error=false;
//					for (int n=0;n<Nodes[i]->value();++n)
//					{
//						dPos=dSimBox[2*i]+n*dStep;
//						dValue=fParse.Eval(&dPos);
//						if (fParse.EvalError()!=0) error=true;
//						clGrid->AddDiscLine(i,dValue);
//					}
//					if (error) QMessageBox::warning(HomogenDisc,tr("Error evaluation grid function!"),QString(tr("An error occured evaluation the grid function f(%1)!").arg(coordVars.at(i))));
//				}
//				clGrid->Sort(i);
//			}
			if (Nodes[i]->value()>0)// && (dSimBox[2*i+1]>dSimBox[2*i]))
			{
				double dStep=0;
				if (Nodes[i]->value()>1) dStep=(dSimBox[2*i+1]-dSimBox[2*i])/(Nodes[i]->value()-1);
				if (DelOldGrid->isChecked()) clGrid->ClearLines(i);
				double* dValue=new double[Nodes[i]->value()];
				for (int n=0;n<Nodes[i]->value();++n)
					dValue[n]=dSimBox[2*i]+n*dStep;
				std::string error = clGrid->AddDiscLines(i,Nodes[i]->value(),dValue,function[i]->text().toStdString());
				delete[] dValue;
				if (error.empty()==false) QMessageBox::warning(HomogenDisc,tr("Error evaluation grid function!"),QString(error.c_str()));
			}
			clGrid->Sort(i);
		}
	}
	delete HomogenDisc;
	Update();
}

void QCSGridEditor::BuildHomogenDisc()
{
	QDialog* HomogenDisc = new QDialog();
	QGridLayout* lay = new QGridLayout();

	lay->addWidget(new QLabel(tr("Create homogenous discretisation:")),0,0,1,3);
	QCheckBox* DelOldGrid = new QCheckBox(tr("Clear old mesh"));
	DelOldGrid->setChecked(true);
	lay->addWidget(DelOldGrid,0,3);

	lay->addWidget(new QLabel(tr("Min")),1,1);
	lay->addWidget(new QLabel(tr("Max")),1,2);
	lay->addWidget(new QLabel(tr("Lines")),1,3);
//	lay->addWidget(new QLabel(tr("X")),2,0);
//	lay->addWidget(new QLabel(tr("Y")),3,0);
//	lay->addWidget(new QLabel(tr("Z")),4,0);
	QCheckBox* Check[3];
	Check[0] = new QCheckBox(GetDirName(0));
	lay->addWidget(Check[0],2,0);
	Check[1] = new QCheckBox(GetDirName(1));
	lay->addWidget(Check[1],3,0);
	Check[2] = new QCheckBox(GetDirName(2));
	lay->addWidget(Check[2],4,0);
	QLineEdit* BoxLine[6];
	QSpinBox* Nodes[3];
	for (int i=0;i<3;++i)
	{
		BoxLine[2*i]= new QLineEdit(QString("%1").arg(clGrid->GetLine(i,0)));
		lay->addWidget(BoxLine[2*i],i+2,1);
		BoxLine[2*i+1]= new QLineEdit(QString("%1").arg(clGrid->GetLine(i,clGrid->GetQtyLines(i)-1)));
		lay->addWidget(BoxLine[2*i+1],i+2,2);
		Nodes[i] = new QSpinBox();
		Nodes[i]->setRange(1,9999);
		Nodes[i]->setValue(clGrid->GetQtyLines(i));
		lay->addWidget(Nodes[i],i+2,3);
		Check[i]->setChecked(true);
	}
	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),HomogenDisc,SLOT(accept()));
	QPushButton* PBcancel = new QPushButton(tr("Cancel"));
	QObject::connect(PBcancel,SIGNAL(clicked()),HomogenDisc,SLOT(reject()));

	QHBoxLayout* hLay = new QHBoxLayout();
	hLay->addStretch(1);
	hLay->addWidget(PB);
	hLay->addWidget(PBcancel);
	hLay->addStretch(1);

	lay->addLayout(hLay,5,0,1,4);

	HomogenDisc->setLayout(lay);
	if (HomogenDisc->exec()==QDialog::Accepted)
	{
		double dSimBox[6];
		for (int i=0;i<6;++i) dSimBox[i]=BoxLine[i]->text().toDouble();
		for (int i=0;i<3;++i)
		{
			if ((Check[i]->checkState()==Qt::Checked))// && (dSimBox[2*i+1]>dSimBox[2*i]))
			{
				double dStep=0;
				if (Nodes[i]->value()>1) dStep=(dSimBox[2*i+1]-dSimBox[2*i])/(Nodes[i]->value()-1);
				if (DelOldGrid->isChecked()) clGrid->ClearLines(i);
				for (int n=0;n<Nodes[i]->value();++n) clGrid->AddDiscLine(i,dSimBox[2*i]+n*dStep);
				clGrid->Sort(i);
			}
//			else if (Check[i]->checkState()==Qt::Checked)
//			{
//				clGrid->ClearLines(i);
//				if (Nodes[i]->value()>0) clGrid->AddDiscLine(i,dSimBox[2*i]);
//			}
		}
	}
	delete HomogenDisc;
	Update();
}

void QCSGridEditor::IncreaseResolution()
{
	QDialog* DiscInc = new QDialog();
	QGridLayout* lay = new QGridLayout();

	lay->addWidget(new QLabel(tr("Increase Resolution of Discretisation:")),0,0,1,2);

	QCheckBox* Check[3];
	QSpinBox* Spin[3];
	Check[0] = new QCheckBox(tr("X - Factor: "));
	Check[0]->setChecked(true);
	Spin[0]=new QSpinBox();Spin[0]->setRange(2,9);
	lay->addWidget(Check[0],2,0);
	lay->addWidget(Spin[0],2,1);
	Check[1] = new QCheckBox(tr("Y - Factor: "));
	Check[1]->setChecked(true);
	Spin[1]=new QSpinBox();Spin[1]->setRange(2,9);
	lay->addWidget(Check[1],3,0);
	lay->addWidget(Spin[1],3,1);
	Check[2] = new QCheckBox(tr("Z - Factor: "));
	Check[2]->setChecked(true);
	Spin[2]=new QSpinBox();Spin[2]->setRange(2,9);
	lay->addWidget(Check[2],4,0);
	lay->addWidget(Spin[2],4,1);

	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),DiscInc,SLOT(accept()));
	QPushButton* PBcancel = new QPushButton(tr("Cancel"));
	QObject::connect(PBcancel,SIGNAL(clicked()),DiscInc,SLOT(reject()));

	QHBoxLayout* hLay = new QHBoxLayout();
	hLay->addStretch(1);
	hLay->addWidget(PB);
	hLay->addWidget(PBcancel);
	hLay->addStretch(1);

	lay->addLayout(hLay,5,0,1,2);

	DiscInc->setLayout(lay);

	if (DiscInc->exec()==QDialog::Accepted)
	{
		for (int i=0;i<3;++i)
			if (Check[i]->checkState()==Qt::Checked)
			{
				clGrid->IncreaseResolution(i,Spin[i]->value());
			}
	}
	Update();
}

void QCSGridEditor::EditX()
{
	Edit(0);
}

void QCSGridEditor::EditY()
{
	Edit(1);
}

void QCSGridEditor::EditZ()
{
	Edit(2);
}

double* QCSGridEditor::GetDoubleArrayFromString(int *count, QString qsValue)
{
	double* values=NULL;
	*count = 0;
	QStringList seq = qsValue.split(":");
	if (seq.size()==3)
	{
		double start,step,stop;
		bool ok;
		start = seq.at(0).toDouble(&ok);
		if (ok) step = seq.at(1).toDouble(&ok);
		if (ok) stop = seq.at(2).toDouble(&ok);
		if (ok)
		{
			int NrSteps = (int)((stop-start)/step);
			if ((NrSteps>=0) && (NrSteps<1000))
			{
				values = new double[NrSteps+1];
				*count = NrSteps+1;
				for (int n=0;n<=NrSteps;++n)
					values[n]=start+n*step;
			}
		}
	}
	return values;
}

void QCSGridEditor::Edit(int direct)
{
	QTextEdit *Line = new QTextEdit(clGrid->GetLinesAsString(direct).c_str());
	Line->setReadOnly(QCSX_Settings.GetEdit()==false);
	QDialog* EditDisc = new QDialog(this);

	if (QCSX_Settings.GetEdit())
		EditDisc->setWindowTitle(tr("Edit Discratisation"));
	else
		EditDisc->setWindowTitle(tr("View Discratisation"));
	QGridLayout* lay = new QGridLayout();

	lay->addWidget(Line,0,0,1,4);

	if (QCSX_Settings.GetEdit())
		lay->addWidget(new QLabel(tr("Allowed syntax example: 0,10, 20, 30:1.5e1:100, 50 , sqrt(1000:-10:200)")),1,0,1,4);
	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),EditDisc,SLOT(accept()));
	lay->addWidget(PB,2,1);
	if (QCSX_Settings.GetEdit())
	{
		PB = new QPushButton(tr("Cancel"));
		QObject::connect(PB,SIGNAL(clicked()),EditDisc,SLOT(reject()));
		lay->addWidget(PB,2,2);
	}

	lay->setRowStretch(0,1);
	lay->setColumnStretch(0,1);
	lay->setColumnStretch(3,1);

	EditDisc->setLayout(lay);
	if (EditDisc->exec()==QDialog::Accepted && QCSX_Settings.GetEdit())
	{
		clGrid->ClearLines(direct);
		QStringList gridValues = Line->toPlainText().split(",");
		for (int n=0;n<gridValues.size();++n)
		{
			QString qsValue = gridValues.at(n);
			double val;
			bool ok;
			val = qsValue.toDouble(&ok);
			if (ok) clGrid->AddDiscLine(direct,val);
			else
			{
				QString FloatExp("-?\\d+(.\\d+)?(e-?\\d+)?");
				QRegExp qre(FloatExp + ":" + FloatExp + ":" + FloatExp);
				if (qre.exactMatch(qsValue))
				{
					int count=0;
					double* values = GetDoubleArrayFromString(&count,qsValue);
					if (values) clGrid->AddDiscLines(direct,count,values);
				}
				else if (qsValue.contains(QRegExp(FloatExp + ":" + FloatExp + ":" + FloatExp)))
				{
					qre.indexIn(qsValue);
					QString seq = qre.cap();
//					cerr << "found-->" << seq.toStdString() <<  endl;
					QString function = qsValue.replace(QRegExp(FloatExp + ":" + FloatExp + ":" + FloatExp),GetDirName(direct));
//					cerr << function.toStdString() << endl;
					int count=0;
					double* values = GetDoubleArrayFromString(&count,seq);
					if (values) clGrid->AddDiscLines(direct,count,values,function.toStdString());
				}
			}
		}
	}
	Update();
}


void QCSGridEditor::DetectEdges()
{
	QDialog* Edges = new QDialog();
	QGridLayout* lay = new QGridLayout();

	lay->addWidget(new QLabel(tr("Detect Edges for Discretisation:")),0,0,1,1);

	QCheckBox* Check[3];
	Check[0] = new QCheckBox(tr("X - Active "));
	Check[0]->setChecked(true);
	lay->addWidget(Check[0],2,0);
	Check[1] = new QCheckBox(tr("Y - Active "));
	Check[1]->setChecked(true);
	lay->addWidget(Check[1],3,0);
	Check[2] = new QCheckBox(tr("Z - Active "));
	Check[2]->setChecked(true);
	lay->addWidget(Check[2],4,0);

	QPushButton* PB = new QPushButton(tr("Ok"));
	QObject::connect(PB,SIGNAL(clicked()),Edges,SLOT(accept()));
	QPushButton* PBcancel = new QPushButton(tr("Cancel"));
	QObject::connect(PBcancel,SIGNAL(clicked()),Edges,SLOT(reject()));

	QHBoxLayout* hLay = new QHBoxLayout();
	hLay->addStretch(1);
	hLay->addWidget(PB);
	hLay->addWidget(PBcancel);
	hLay->addStretch(1);

	lay->addLayout(hLay,5,0,1,1);

	Edges->setLayout(lay);

	if (Edges->exec()==QDialog::Accepted)
	{
		for (int i=0;i<3;++i)
			if (Check[i]->checkState()==Qt::Checked)
			{
				emit signalDetectEdges(i);
			}
	}
	Update();
}

void QCSGridEditor::SetDeltaUnit(QString val)
{
	bool succ=false;
	double dDal=val.toDouble(&succ);
	if ((val>0) && succ) clGrid->SetDeltaUnit(dDal);
}

void QCSGridEditor::Update()
{
	clGrid->Sort(0);
	clGrid->Sort(1);
	clGrid->Sort(2);
	for (size_t i=0;i<3;++i)
	{
		SimBox.at(2*i)->setText(QString("%1").arg(clGrid->GetLine(i,0)));
		SimBox.at(2*i+1)->setText(QString("%1").arg(clGrid->GetLine(i,clGrid->GetQtyLines(i)-1)));
		NodeQty.at(i)->setText(QString("%1").arg(clGrid->GetQtyLines(i)));
		m_DirNames[i]->setText(GetDirName(i));
		m_PlanePos[i]->setRange(0,clGrid->GetQtyLines(i)-1);
		m_NormNames[i]->setText(GetNormName(i)+ tr(" plane: "));
	}
	SetGridPlaneX(m_PlanePos[0]->value());
	SetGridPlaneY(m_PlanePos[0]->value());
	SetGridPlaneZ(m_PlanePos[0]->value());
	UnitLength->setText(QString("%1").arg(clGrid->GetDeltaUnit()));
	emit GridChanged();
}

void QCSGridEditor::SetOpacity(int val)
{
	OpacitySlider->setValue(val);
}

int QCSGridEditor::GetOpacity()
{
	return OpacitySlider->value();
}

QString QCSGridEditor::GetDirName(int ny)
{
	if (clGrid->GetMeshType()==0)
	{
		switch (ny)
		{
		case 0:
			return "x";
		case 1:
			return "y";
		case 2:
			return "z";
		}
	}
	if (clGrid->GetMeshType()==1)
	{
		switch (ny)
		{
		case 0:
			return "r";
		case 1:
			return QChar(0xb1, 0x03);
		case 2:
			return "z";
		}
	}
	return "";
}

QString QCSGridEditor::GetNormName(int ny)
{
	if (clGrid->GetMeshType()==0)
	{
		switch (ny)
		{
		case 0:
			return "yz";
		case 1:
			return "zx";
		case 2:
			return "xy";
		}
	}
	if (clGrid->GetMeshType()==1)
	{
		switch (ny)
		{
		case 0:
			return QString(QChar(0xb1, 0x03)) + "z";
		case 1:
			return "zr";
		case 2:
			return "r" + QString(QChar(0xb1, 0x03));
		}
	}
	return "";
}
