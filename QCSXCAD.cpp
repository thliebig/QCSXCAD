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

#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>

#include "QCSXCAD.h"
#include "QVTKStructure.h"
#include "QCSPrimEditor.h"
#include "QCSPropEditor.h"
#include "QCSTreeWidget.h"
#include "QCSGridEditor.h"
#include "QParameterGui.h"
#include "tinyxml.h"
#include <iostream>

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

#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkPOVExporter.h>

// exporter
#include "export_x3d.h"
#include "export_pov.h"

QCSXCAD::QCSXCAD(QWidget *parent) : QMainWindow(parent)
{
	QStringList argList=qApp->arguments();
	for (int i=1;i<argList.size();++i)
	{
		if (argList.at(i).startsWith("-"))
			QCSX_Settings.parseCommandLineArgument(argList.at(i));
	}

	m_ViewDir = 2;

	m_RenderDiscModels = QCSX_Settings.GetRenderDiscMaterial();

	StructureVTK = new QVTKStructure();
	StructureVTK->SetGeometry(this);

	setCentralWidget(StructureVTK->GetVTKWidget());

	CSTree = new QCSTreeWidget(this);
	QObject::connect(CSTree,SIGNAL(Edit()),this,SLOT(Edit()));
	QObject::connect(CSTree,SIGNAL(Copy()),this,SLOT(Copy()));
	QObject::connect(CSTree,SIGNAL(ShowHide()),this,SLOT(ShowHide()));
	QObject::connect(CSTree,SIGNAL(Delete()),this,SLOT(Delete()));
	QObject::connect(CSTree,SIGNAL(NewBox()),this,SLOT(NewBox()));
	QObject::connect(CSTree,SIGNAL(NewMultiBox()),this,SLOT(NewMultiBox()));
	QObject::connect(CSTree,SIGNAL(NewSphere()),this,SLOT(NewSphere()));
	QObject::connect(CSTree,SIGNAL(NewCylinder()),this,SLOT(NewCylinder()));
	QObject::connect(CSTree,SIGNAL(NewUserDefined()),this,SLOT(NewUserDefined()));

	QObject::connect(CSTree,SIGNAL(NewMaterial()),this,SLOT(NewMaterial()));
	QObject::connect(CSTree,SIGNAL(NewMetal()),this,SLOT(NewMetal()));
	QObject::connect(CSTree,SIGNAL(NewExcitation()),this,SLOT(NewExcitation()));
	QObject::connect(CSTree,SIGNAL(NewChargeBox()),this,SLOT(NewChargeBox()));
	QObject::connect(CSTree,SIGNAL(NewResBox()),this,SLOT(NewResBox()));
	QObject::connect(CSTree,SIGNAL(NewDumpBox()),this,SLOT(NewDumpBox()));

	QDockWidget *dock = new QDockWidget(tr("Properties and Structures"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea);
	dock->setWidget(CSTree);
	dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	dock->setObjectName("Properties_and_Structures_Dock");

	addDockWidget(Qt::LeftDockWidgetArea,dock);

	GridEditor = new QCSGridEditor(&clGrid);
	QObject::connect(GridEditor,SIGNAL(OpacityChange(int)),StructureVTK,SLOT(SetGridOpacity(int)));
	QObject::connect(GridEditor,SIGNAL(signalDetectEdges(int)),this,SLOT(DetectEdges(int)));
	QObject::connect(GridEditor,SIGNAL(GridChanged()),StructureVTK,SLOT(RenderGrid()));
	QObject::connect(GridEditor,SIGNAL(GridPlaneXChanged(int)),StructureVTK,SLOT(RenderGridX(int)));
	QObject::connect(GridEditor,SIGNAL(GridPlaneYChanged(int)),StructureVTK,SLOT(RenderGridY(int)));
	QObject::connect(GridEditor,SIGNAL(GridPlaneZChanged(int)),StructureVTK,SLOT(RenderGridZ(int)));

	dock = new QDockWidget(tr("Rectilinear Grid"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea);
	dock->setWidget(GridEditor);
	dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	dock->setObjectName("Rectilinear_Grid_Dock");
	addDockWidget(Qt::LeftDockWidgetArea,dock);

	QParaSet= new QParameterSet();
	QObject::connect(QParaSet,SIGNAL(ParameterChanged()),this,SLOT(CheckGeometry()));
	QObject::connect(QParaSet,SIGNAL(ParameterChanged()),this,SLOT(setModified()));
	clParaSet=QParaSet;

	dock = new QDockWidget(tr("Rectilinear Grid - Plane Position"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea);
	dock->setWidget(GridEditor->BuildPlanePosWidget());
	dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	dock->setObjectName("Grid_Plane_Pos");
	addDockWidget(Qt::LeftDockWidgetArea,dock);

	dock = new QDockWidget(tr("Parameter"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea);
	dock->setWidget(QParaSet);
	dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	dock->setObjectName("Parameter_Dock");
	addDockWidget(Qt::LeftDockWidgetArea,dock);

	BuildToolBar();

	bModified=true;
	GridEditor->SetOpacity(30);
	Render();
}

QCSXCAD::~QCSXCAD()
{
}

QString QCSXCAD::GetInfoString()
{
	QString text = QString("%1").arg(_QCSXCAD_LIB_NAME_);
	text += QString("<br>Author: %1<br>EMail: %2").arg(_QCSXCAD_AUTHOR_).arg(_QCSXCAD_AUTHOR_MAIL_);
	text += QString("<br>Version: %1\t Build: %2 %3").arg(_QCSXCAD_VERSION_).arg(__DATE__).arg(__TIME__);
	text += QString("<br>License: %1").arg(_QCSXCAD_LICENSE_);
	return text;
}

QIcon QCSXCAD::GetLibIcon()
{
	return QIcon(":/images/QCSXCAD_Icon.png");
}

void QCSXCAD::aboutQCSXCAD(QWidget* parent)
{
	QDialog infoWidget(parent);
	infoWidget.setWindowTitle("Info");
	QLabel *infoLbl = new QLabel();
	infoLbl->setText(GetInfoString());
	infoLbl->setAlignment(Qt::AlignLeft);

	QGroupBox* DependGroup = new QGroupBox(tr("Used Libraries"),&infoWidget);

	QLabel *qtinfo = new QLabel();
	QPushButton* qtButton = new QPushButton(QIcon(":/images/qt-logo.png"),QString());
	qtButton->setToolTip(tr("About Qt"));
	qtButton->setIconSize(QSize(50,50));
	QObject::connect(qtButton,SIGNAL(clicked()),qApp,SLOT(aboutQt()));
	qtinfo->setText(QString("GUI-Toolkit: Qt by Trolltech (OSS) <br>Version: %1<br>http://www.trolltech.com/<br>License: GNU General Public License (GPL)").arg(QT_VERSION,0,16));
	qtinfo->setAlignment(Qt::AlignLeft);

	QLabel *vtkinfo = new QLabel();
	QPushButton* vtkButton = new QPushButton(QIcon(":/images/vtk-logo.png"),QString());
	vtkButton->setIconSize(QSize(50,50));
	//vtkButton->setToolTip(tr("About Vtk"));
	//QObject::connect(vtkButton,SIGNAL(clicked()),qApp,SLOT(aboutQt()));
	vtkinfo->setText(QString("3D-Toolkit: Visualization Toolkit (VTK)<br>Version: %1<br>http://www.vtk.org/<br>License: BSD-License").arg(VTK_VERSION));
	vtkinfo->setAlignment(Qt::AlignLeft);

	QLabel *CSXCADinfo = new QLabel();
	CSXCADinfo->setText(ContinuousStructure::GetInfoLine().c_str());
	CSXCADinfo->setAlignment(Qt::AlignLeft);

	QGridLayout *Glay = new QGridLayout();
	Glay->addWidget(qtButton,1,1);
	Glay->addWidget(qtinfo,1,2);
	Glay->addWidget(vtkButton,2,1);
	Glay->addWidget(vtkinfo,2,2);
	Glay->addWidget(CSXCADinfo,3,2);
	Glay->setColumnStretch(1,0);
	Glay->setColumnStretch(2,1);

	DependGroup->setLayout(Glay);

	QGridLayout *infoLayout = new QGridLayout();
	QPushButton* iconButt = new QPushButton(QCSXCAD::GetLibIcon(),"");
	iconButt->setFlat(true);
	iconButt->setIconSize(QSize(128,128));
	infoLayout->addWidget(iconButt,1,1);
	infoLayout->addWidget(infoLbl,1,2,1,3);
	infoLayout->addWidget(DependGroup,2,1,1,3);

	QPushButton* OKBut = new QPushButton(tr("Ok"));
	QObject::connect(OKBut,SIGNAL(clicked()),&infoWidget,SLOT(accept()));
	infoLayout->addWidget(OKBut,3,2);

	infoLayout->setColumnStretch(1,1);
	infoLayout->setColumnStretch(3,1);

	infoWidget.setLayout(infoLayout);

//	infoWidget->show();
//	infoWidget->adjustSize();
	infoWidget.exec();
}

//void QCSXCAD::SetFile(QString filename)
//{
//	QFilename=filename;
//
//	QString file;
//	if (QFilename.startsWith("./")) file=relPath+QFilename.mid(2);
//	else file=QFilename;
//
//	//File_Lbl->setText(QString("Geometry File: %1").arg(file));
//
//	emit FileModified(true);
//	setModified();
//	ReadFile(file);
//}
//
//QString QCSXCAD::GetFilename()
//{
//	return QFilename;
//}
//
//QString QCSXCAD::GetGeometry()
//{
//	if (QFilename.startsWith("./")) return relPath+QFilename.mid(2);
//	else return QFilename;
//}

bool QCSXCAD::CheckGeometry()
{
	QString msg = QString::fromStdString(Update());
	if (msg.isEmpty())
	{
		return true;
	}

	QMessageBox::warning(this,tr("Geometry Edit Warning"),tr("Geometry Edit Warning: Update Error occurred!!\n")+msg,QMessageBox::Ok,QMessageBox::NoButton);

	return false;
}

TiXmlNode* QCSXCAD::FindRootNode(TiXmlNode* node)
{
	if (node==NULL) return NULL;
	TiXmlElement* child = node->FirstChildElement("ContinuousStructure");
	if (child)
		return node;
	child=node->FirstChildElement();
	TiXmlNode* found=NULL;
	while (child!=NULL)
	{
		if (child->FirstChildElement("ContinuousStructure"))
			return child;
		found = FindRootNode(child);
		if (found)
			return found;
		child = node->NextSiblingElement();
	}
	return NULL;
}

bool QCSXCAD::ReadNode(TiXmlNode* root)
{
	if (root==NULL) return false;
	clear();
	QString msg(ReadFromXML(root));
	if (msg.isEmpty()==false) QMessageBox::warning(this,tr("Geometry read error"),tr("An geometry read error occured!!\n\n")+msg,QMessageBox::Ok,QMessageBox::NoButton);
	CSTree->UpdateTree();
	CSTree->expandAll();
	setModified();
	CheckGeometry();
	GridEditor->Update();
	BestView();
	StructureVTK->ResetView();
	return true;
}

bool QCSXCAD::ReadFile(QString filename)
{
	if (QFile::exists(filename)==false) return false;

	TiXmlDocument doc(filename.toStdString().c_str());
	if (!doc.LoadFile()) { QMessageBox::warning(this,tr("File- Error!!! File: "),tr("File-Loading failed!!!"),QMessageBox::Ok,QMessageBox::NoButton); }

	TiXmlNode* root = 0;
	TiXmlElement* openEMS = doc.FirstChildElement("openEMS");
	if (openEMS)
	{
		root = ReadOpenEMS(openEMS);
	}
	else
	{
		//try to find a root node somewhere else...
		root = FindRootNode(&doc);
	}
	if (root==NULL)
	{
		QMessageBox::warning(this,tr("Geometry read error"),tr("Can't find root CSX node!!"),QMessageBox::Ok,QMessageBox::NoButton);
		return false;
	}
//	QString msg(ReadFromXML(filename.toLatin1().constData()));
	QString msg(ReadFromXML(root));
	if (msg.isEmpty()==false)
		QMessageBox::warning(this,tr("Geometry read error"),tr("An geometry read error occured!!\n\n")+msg,QMessageBox::Ok,QMessageBox::NoButton);

	CSTree->UpdateTree();
	CSTree->expandAll();
	setModified();
	CheckGeometry();
	GridEditor->Update();
	BestView();
	StructureVTK->ResetView();
	return true;
}

TiXmlNode* QCSXCAD::ReadOpenEMS(TiXmlNode* openEMS)
{
	// read FDTD options
	m_BC.clear();
	TiXmlElement* element = openEMS->FirstChildElement("FDTD");
	if (element)
	{
		TiXmlElement* BC = element->FirstChildElement("BoundaryCond");
		if (BC)
		{
			TiXmlAttribute *attr = BC->FirstAttribute();
			while (attr)
			{
				m_BC[attr->Name()] = attr->Value();
				attr = attr->Next();
			}
		}
	}
	return openEMS;
}

int QCSXCAD::GetCurrentProperty()
{
	return GetIndex(CSTree->GetCurrentProperty());
}

ParameterSet* QCSXCAD::GetParaSet()
{
	return QParaSet;
}

bool QCSXCAD::Write2XML(TiXmlNode* rootNode, bool parameterised)
{
	return ContinuousStructure::Write2XML(rootNode,parameterised);
}

bool QCSXCAD::Write2XML(const char* file, bool parameterised)
{
	return ContinuousStructure::Write2XML(file,parameterised);
}

bool QCSXCAD::Write2XML(QString file, bool parameterised)
{
	return ContinuousStructure::Write2XML(file.toStdString().c_str(),parameterised);
}

bool QCSXCAD::isGeometryValid()
{
	if (ContinuousStructure::isGeometryValid()==false) return false;
	return true;
}

void QCSXCAD::ImportGeometry()
{
	QString filter;
	QString filename=QFileDialog::getOpenFileName(0,tr("Choose geometry file"),NULL,tr("XML-File (*.xml)"),&filter);
	if (filename.isEmpty())
		return;
	ReadFile(filename);
}

void QCSXCAD::Edit()
{
	CSPrimitives* prim = CSTree->GetCurrentPrimitive();
	if (prim!=NULL)
	{
		CSProperties* oldProp=prim->GetProperty();
		QCSPrimEditor* newEdit = new QCSPrimEditor(this,prim);
		if (newEdit->exec()==QDialog::Accepted)
		{
			CSProperties* newProp=prim->GetProperty();
			if (newProp!=oldProp)
				CSTree->SwitchProperty(prim,newProp);
			setModified();
			//CSTree->UpdateTree();
		}
		return;
	}
	CSProperties* prop = CSTree->GetCurrentProperty();
	if (prop!=NULL)
	{
		int index=GetIndex(prop);
		QCSPropEditor* newEdit = new QCSPropEditor(this,prop,m_SimMode);
		if (newEdit->exec()==QDialog::Accepted)
		{
			CSTree->RefreshItem(index);
			setModified();
		}
	}
}

void QCSXCAD::Copy()
{
	CSPrimitives* prim = CSTree->GetCurrentPrimitive();
	if (prim!=NULL)
	{
		CSPrimitives* newPrim=prim->GetCopy();
		if (newPrim==NULL)
			return;
		QCSPrimEditor* newEdit = new QCSPrimEditor(this,newPrim);
		if (newEdit->exec()==QDialog::Accepted)
		{
			setModified();
			CSTree->AddPrimItem(newPrim);
		}
		else
			delete newPrim;
	}
}

void QCSXCAD::SetVisibility2All(bool value)
{
	for (size_t n=0; n<vProperties.size();++n)
	{
		CSProperties* prop = vProperties.at(n);
		prop->SetVisibility(value);
		CSTree->RefreshItem(GetIndex(prop));
		if (value) StructureVTK->SetPropOpacity(prop->GetUniqueID(),prop->GetFillColor().a);
		else StructureVTK->SetPropOpacity(prop->GetUniqueID(),0);
	}
}

void QCSXCAD::HideAll()
{
	SetVisibility2All(false);
}

void QCSXCAD::ShowAll()
{
	SetVisibility2All(true);
}

void QCSXCAD::SetParallelProjection(bool val)
{
	StructureVTK->SetParallelProjection(val);
}

void QCSXCAD::ShowHide()
{
	CSProperties* prop = CSTree->GetCurrentProperty();
	if (prop!=NULL)
	{
		prop->SetVisibility(!prop->GetVisibility());
		CSTree->RefreshItem(GetIndex(prop));
		if (prop->GetVisibility()) StructureVTK->SetPropOpacity(prop->GetUniqueID(),prop->GetFillColor().a);
		else StructureVTK->SetPropOpacity(prop->GetUniqueID(),0);
	}
}

void QCSXCAD::Delete()
{
	CSPrimitives* prim = CSTree->GetCurrentPrimitive();
	if (prim!=NULL)
	{
		if (QMessageBox::question(this,tr("Delete Primitive"),tr("Delete current Primitive (ID: %1)?").arg(prim->GetID()),QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
		{
			CSTree->DeletePrimItem(prim);
			DeletePrimitive(prim);
			setModified();
		}
		return;
	}
	CSProperties* prop = CSTree->GetCurrentProperty();
	if (prop!=NULL)
	{
		size_t qtyPrim=prop->GetQtyPrimitives();
		if (qtyPrim>0)
		{
			if (QMessageBox::question(this,tr("Delete Property"),tr("\"%1\" contains Primitive(s)!!\n Delete anyway?").arg(prop->GetName().c_str()),QMessageBox::Yes,QMessageBox::No)!=QMessageBox::Yes)
				return;
		}
		else if (QMessageBox::question(this,tr("Delete Property"),tr("Delete current Property?"),QMessageBox::Yes,QMessageBox::No)!=QMessageBox::Yes)
			return;
		CSTree->DeletePropItem(prop);
		DeleteProperty(prop);
		setModified();
	}
}

void QCSXCAD::NewBox()
{
	NewPrimitive(new CSPrimBox(clParaSet,CSTree->GetCurrentProperty()));
}

void QCSXCAD::NewMultiBox()
{
	NewPrimitive(new CSPrimMultiBox(clParaSet,CSTree->GetCurrentProperty()));
}

void QCSXCAD::NewSphere()
{
	NewPrimitive(new CSPrimSphere(clParaSet,CSTree->GetCurrentProperty()));
}

void QCSXCAD::NewCylinder()
{
	NewPrimitive(new CSPrimCylinder(clParaSet,CSTree->GetCurrentProperty()));
}

void QCSXCAD::NewPolygon()
{
	NewPrimitive(new CSPrimPolygon(clParaSet,CSTree->GetCurrentProperty()));
}

void QCSXCAD::NewUserDefined()
{
	NewPrimitive(new CSPrimUserDefined(clParaSet,CSTree->GetCurrentProperty()));
}


void QCSXCAD::NewPrimitive(CSPrimitives* newPrim)
{
	if (GetQtyProperties()==0)
	{
		QMessageBox::question(this,tr("New Primitive"),tr("No Property available. You have to add one first!"),QMessageBox::Ok);
		delete newPrim;
		return;
	}
	QCSPrimEditor* newEdit = new QCSPrimEditor(this,newPrim);

	if (newEdit->exec()==QDialog::Accepted)
	{
		setModified();
		CSTree->AddPrimItem(newPrim);
	}
	else
		delete newPrim;
}

void QCSXCAD::NewMaterial()
{
	NewProperty(new CSPropMaterial(clParaSet));
}

void QCSXCAD::NewMetal()
{
	NewProperty(new CSPropMetal(clParaSet));
}

void QCSXCAD::NewExcitation()
{
	NewProperty(new CSPropExcitation(clParaSet,GetQtyPropertyType(CSProperties::EXCITATION)));
}

void QCSXCAD::NewChargeBox()
{
	NewProperty(new CSPropProbeBox(clParaSet));
}

void QCSXCAD::NewResBox()
{
	NewProperty(new CSPropResBox(clParaSet));
}

void QCSXCAD::NewDumpBox()
{
	NewProperty(new CSPropDumpBox(clParaSet));
}

void QCSXCAD::setModified()
{
	bModified=true;
	emit modified(true);
	Render();
}

void QCSXCAD::DetectEdges(int nu)
{
	InsertEdges2Grid(nu);
}

void QCSXCAD::BestView()
{
	StructureVTK->ResetView();
}

void QCSXCAD::setViewDir(int dir)
{
	switch (dir)
	{
	default:
	case 0:
		setYZ();
		return;
	case 1:
		setZX();
		return;
	case 2:
		setXY();
		return;
	}
}

void QCSXCAD::setXY()
{
	m_ViewDir = 2;
	StructureVTK->setXY();
}

void QCSXCAD::setYZ()
{
	m_ViewDir = 0;
	StructureVTK->setYZ();
}

void QCSXCAD::setZX()
{
	m_ViewDir = 1;
	StructureVTK->setZX();
}

void QCSXCAD::SetSimMode(int mode)
{
	m_SimMode=mode;
}

void QCSXCAD::NewProperty(CSProperties* newProp)
{
	QCSPropEditor* newEdit = new QCSPropEditor(this,newProp,m_SimMode);

	if (newEdit->exec()==QDialog::Accepted)
	{
		AddProperty(newProp);
		CSTree->AddPropItem(newProp);
	}
	else delete newProp;
}

void QCSXCAD::New()
{
	if (bModified)
	{
		if (QMessageBox::question(this,tr("New Geometry"),tr("Create empty Geometry??"),QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)
			return;
		else
			clear();
	}
}

//void QCSXCAD::Load()
//{
//	if (bModified)
//	switch (QMessageBox::question(this,tr("Load Geometry"),tr("Save current Geometry??"),QMessageBox::Yes,QMessageBox::No,QMessageBox::Cancel))
//	{
//		case QMessageBox::Yes:
//			Save();
//			break;
//		case QMessageBox::Cancel:
//			return;
//			break;
//	};
//	browseGeometryFile();
//}


//void QCSXCAD::Save()
//{
//	if (QFilename.isEmpty()) {SaveAs(); return;}
//
//	CheckGeometry();
//
//	if (QFilename.startsWith("./")) Write2XML((relPath+QFilename.mid(2)).toLatin1().data());
//	else Write2XML(QFilename.toLatin1().data());
//	bModified=false;
//}

void QCSXCAD::ExportGeometry()
{
	QString qFilename=QFileDialog::getSaveFileName(0,"Choose Geometrie File",NULL,"SimGeometryXML (*.xml)");
	if (qFilename==NULL) return;
	if (!qFilename.endsWith(".xml")) qFilename+=".xml";

	if (Write2XML(qFilename.toLatin1().data())==false) QMessageBox::warning(this,tr("Geometry Export"),tr("Unknown error occured! Geometry Export failed"),1,0);
}

void QCSXCAD::ExportGeometry_Povray()
{
	QString filename = QFileDialog::getSaveFileName( this, tr("Save Povray file"), QString(), tr("Povray files (*.pov)") );
	if (filename.isEmpty())
		return;

	export_pov pov( this );
	pov.save( filename );

	// start povray?
	int ans = QMessageBox::question( 0, "Start Povray", "Should the file directly be rendered?", "Yes", "No", "", 0, 1 );
	if (ans == 1)
		return;

	// start povray
	QStringList args;
	args << filename;
	args << "-W640";
	args << "-H480";
	args << "+A";
	//only valid for povray >3.7.0     args << "+WT4";
	QProcess::startDetached( "povray", args, QFileInfo(filename).absolutePath() );
	return;

	//	// Instead of letting renderer to render the scene, we use
//	// an exportor to save it to a file.
//	vtkPOVExporter *povexp = vtkPOVExporter::New();
//	povexp->SetRenderWindow( ((QVTKWidget*)(StructureVTK->GetVTKWidget()))->GetRenderWindow() );
//	povexp->SetFileName("/tmp/TestPOVExporter.pov");
//	cout << "Writing file TestPOVExporter.pov..." << endl;
//
//	povexp->Write();
//	cout << "Done writing file TestPOVExporter.pov..." << endl;
//
//	povexp->Delete();
}

void QCSXCAD::ExportGeometry_X3D(QString filename)
{
	if (filename.isEmpty())
		filename = QFileDialog::getSaveFileName( this, tr("Save X3D-file"), QString(), tr("X3D files (*.x3d)") );
	if (filename.isEmpty())
		return;

	export_X3D x3d( this );
	x3d.save( filename );
}

void QCSXCAD::ExportGeometry(QString dirname, int type)
{
	if (dirname.isEmpty())
		dirname = QFileDialog::getExistingDirectory(this, tr("Choose directory to save data"));
	if (dirname.isEmpty())
		return;
	int QtyProp = GetQtyProperties();
	for (int i=0;i<QtyProp;++i)
	{
		CSProperties* prop = GetProperty(i);
		if (prop==NULL) continue;

		 unsigned int uID = prop->GetUniqueID();

		 if (prop->GetVisibility()==true)
		 {
			QString filename(dirname);
			filename.append("/");
			filename.append(QString::fromUtf8(prop->GetName().c_str()));
			switch (type)
			{
			case 0:
				StructureVTK->ExportProperty2PolyDataVTK(uID,filename,clGrid.GetDeltaUnit());
				break;
			case 1:
				StructureVTK->ExportProperty2STL(uID,filename,clGrid.GetDeltaUnit());
				break;
			case 2:
				StructureVTK->ExportProperty2PLY(uID,filename,clGrid.GetDeltaUnit());
				break;
			default:
				QMessageBox::warning(this, "Export Dialog","Unkown export type, skipping...");
				return;
				break;
			}
		 }
	 }
}

void QCSXCAD::ExportGeometry_PolyDataVTK(QString dirname)
{
	ExportGeometry(dirname, 0);
}

void QCSXCAD::ExportGeometry_STL(QString dirname)
{
	ExportGeometry(dirname, 1);
}

void QCSXCAD::ExportGeometry_PLY(QString dirname)
{
	ExportGeometry(dirname, 2);
}

void QCSXCAD::ExportView2Image()
{
	if (ViewLevel==VIEW_3D)
		StructureVTK->ExportView2Image();
	else
		QMessageBox::warning(this,tr("PNG export"),tr("Not Yet Implemented for 2D view, use 3D instead."),QMessageBox::Ok,QMessageBox::NoButton);
}

void QCSXCAD::EnableDiscModelRendering(bool val)
{
	m_RenderDiscModels = val;
}

void QCSXCAD::GUIUpdate()
{
	CSTree->UpdateTree();
	GridEditor->Update();
	for (int n=0;n<3;++n)
		viewPlane[n]->setText(GridEditor->GetNormName(n));
}

void QCSXCAD::clear()
{
	ContinuousStructure::clear();
	GUIUpdate();
	setModified();
	bModified=false;
}

void QCSXCAD::BuildToolBar()
{
	QToolBar *mainTB = addToolBar(tr("General"));
	mainTB->setObjectName("General_ToolBar");
	QSize TBIconSize(16,16);
	mainTB->setIconSize(TBIconSize);

	if (QCSX_Settings.GetEdit())
		mainTB->addAction(QIcon(":/images/filenew.png"),tr("New"),this,SLOT(New()));
	if (QCSX_Settings.GetEdit())
		mainTB->addAction(QIcon(":/images/down.png"),tr("Import"),this,SLOT(ImportGeometry()));
	mainTB->addAction(QIcon(":/images/up.png"),tr("Export"),this,SLOT(ExportGeometry()));


	QToolBar *ItemTB = addToolBar(tr("Item View"));
	ItemTB->setIconSize(TBIconSize);
	ItemTB->setObjectName("Item_View_ToolBar");

	ItemTB->addAction(tr("CollapseAll"),CSTree,SLOT(collapseAll()));
	ItemTB->addAction(tr("ExpandAll"),CSTree,SLOT(expandAll()));

	ItemTB->addAction(QIcon(":/images/bulb.png"),tr("ShowAll"),this,SLOT(ShowAll()));
	ItemTB->addAction(QIcon(":/images/bulb_off.png"),tr("HideAll"),this,SLOT(HideAll()));

	QToolBar *newObjct = NULL;
	QAction* newAct = NULL;

	if (QCSX_Settings.GetEdit())
	{
		newObjct = addToolBar(tr("add new Primitive"));
		newObjct->setObjectName("New_Primitive_ToolBar");

		newAct = newObjct->addAction(tr("Box"),this,SLOT(NewBox()));
		newAct->setToolTip(tr("add new Box"));

		newAct = newObjct->addAction(tr("MultiBox"),this,SLOT(NewMultiBox()));
		newAct->setToolTip(tr("add new Multi-Box"));

		newAct = newObjct->addAction(tr("Sphere"),this,SLOT(NewSphere()));
		newAct->setToolTip(tr("add new Sphere"));

		newAct = newObjct->addAction(tr("Cylinder"),this,SLOT(NewCylinder()));
		newAct->setToolTip(tr("add new Cylinder"));

		newAct = newObjct->addAction(tr("Polygon"),this,SLOT(NewPolygon()));
		newAct->setToolTip(tr("add new Polygon"));

		newAct = newObjct->addAction(tr("User Defined"),this,SLOT(NewUserDefined()));
		newAct->setToolTip(tr("add new User Definied Primitive"));

		newObjct = addToolBar(tr("add new Property"));
		newObjct->setObjectName("New_Property_ToolBar");

		newAct = newObjct->addAction(tr("Material"),this,SLOT(NewMaterial()));
		newAct->setToolTip(tr("add new Material-Property"));

		newAct = newObjct->addAction(tr("Metal"),this,SLOT(NewMetal()));
		newAct->setToolTip(tr("add new Metal-Property"));

		newAct = newObjct->addAction(tr("Excitation"),this,SLOT(NewExcitation()));
		newAct->setToolTip(tr("add new Excitation-Property"));

		newAct = newObjct->addAction(tr("ProbeBox"),this,SLOT(NewChargeBox()));
		newAct->setToolTip(tr("add new Probe-Box-Property"));

		newAct = newObjct->addAction(tr("ResBox"),this,SLOT(NewResBox()));
		newAct->setToolTip(tr("add new Res-Box-Property"));

		newAct = newObjct->addAction(tr("DumpBox"),this,SLOT(NewDumpBox()));
		newAct->setToolTip(tr("add new Dump-Box-Property"));
	}

	newObjct = addToolBar(tr("Zoom"));
	newObjct->setIconSize(TBIconSize);
	newObjct->setObjectName("Zoom_ToolBar");

	newAct = newObjct->addAction(QIcon(":/images/viewmagfit.png"),tr("Zoom fit"),this,SLOT(BestView()));
	newAct->setToolTip("Zoom to best fit all objects");

	viewPlane[0] = newObjct->addAction(GridEditor->GetNormName(0),this,SLOT(setYZ()));
	viewPlane[0]->setToolTip(tr("Switch to y-z-plane view (x-normal)"));
	viewPlane[1] = newObjct->addAction(GridEditor->GetNormName(1),this,SLOT(setZX()));
	viewPlane[1]->setToolTip(tr("Switch to z-x-plane view (y-normal)"));
	viewPlane[2] = newObjct->addAction(GridEditor->GetNormName(2),this,SLOT(setXY()));
	viewPlane[2]->setToolTip(tr("Switch to x-y-plane view (z-normal)"));

	addToolBarBreak();

	QActionGroup* ActViewGrp = new QActionGroup(this);
	newAct = newObjct->addAction(tr("2D"),this,SLOT(View2D()));
	newAct->setToolTip(tr("Switch to 2D view mode"));
	ActViewGrp->addAction(newAct);
	newAct->setCheckable(true);
	newAct = newObjct->addAction(tr("3D"),this,SLOT(View3D()));
	newAct->setToolTip(tr("Switch to 3D view mode"));
	ActViewGrp->addAction(newAct);
	newAct->setCheckable(true);
	m_PPview = newObjct->addAction(tr("PP"));
	m_PPview->setToolTip(tr("Toggle parallel projection view mode"));
	QObject::connect(m_PPview,SIGNAL(toggled(bool)),this,SLOT(SetParallelProjection(bool)));
	m_PPview->setCheckable(true);

	if (QCSX_Settings.GetEdit())
		addToolBar(GridEditor->BuildToolbar());
}

void QCSXCAD::Render()
{
	StructureVTK->RenderGrid();
	StructureVTK->RenderGeometry();
	if (m_RenderDiscModels)
		StructureVTK->RenderDiscMaterialModel();

}

void QCSXCAD::View2D()
{
	ViewLevel=VIEW_2D;
	StructureVTK->SaveCamData();
	setViewDir(m_ViewDir);
	StructureVTK->SetParallelProjection(true, false);
	StructureVTK->Set2DInteractionStyle(true);
	m_PPview->setDisabled(true);
}

void QCSXCAD::View3D()
{
	ViewLevel=VIEW_3D;

	m_PPview->setDisabled(false);
	StructureVTK->SetParallelProjection(m_PPview->isChecked(), false);
	StructureVTK->Set2DInteractionStyle(false, false);
	StructureVTK->RestoreCamData(true);
}

void QCSXCAD::keyPressEvent(QKeyEvent * event)
{
	if (event->key()==Qt::Key_Delete) Delete();
	if (event->key()==Qt::Key_Escape)
		CSTree->setCurrentItem(NULL);
	QMainWindow::keyPressEvent(event);
}

