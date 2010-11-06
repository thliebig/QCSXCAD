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

#include "QCSXCAD.h"
#include "QVTKStructure.h"
#include "QCSPrimEditor.h"
#include "QCSPropEditor.h"
#include "QCSTreeWidget.h"
#include "QCSGridEditor.h"
#include "QParameterGui.h"
#include "vtkConfigure.h"
#include "tinyxml.h"
#ifdef __GYM2XML__
#include "Gym2XML.h"
#endif
#include <iostream>

#include <QVTKWidget.h>
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
//	QFilename.clear();
//	relPath.clear();

//	QPushButton* GeometryBrowse = new QPushButton(tr("Browse Geometry"));
//	//GeometryLine->setFixedWidth(LineWidth);
//	QObject::connect(GeometryBrowse,SIGNAL(clicked()),this,SLOT(browseGeometryFile()));


//	QGridLayout* Layout = new QGridLayout();
//	File_Lbl = new QLabel("None");
//	Layout->addWidget(File_Lbl,1,1,1,2);
//	Layout->addWidget(GeometryBrowse,2,1);
//	//Layout->addWidget(qTree,3,1);
//	Layout->setColumnStretch(3,1);
//	Layout->setRowStretch(4,1);

	ViewLevel=0; //0=2D; 1=3D view

	StructureVTK = new QVTKStructure();
	StructureVTK->SetGeometry(this);

	StackWidget = new QStackedWidget();

	DrawWidget = new QGeometryPlot(this);
	StackWidget->addWidget(DrawWidget);
	StackWidget->addWidget(StructureVTK->GetVTKWidget());

	setCentralWidget(StackWidget);
	DrawWidget->SetStatusBar(statusBar());
//	centralWidget()->setLayout(Layout);

	CSTree = new QCSTreeWidget(this);
	QObject::connect(CSTree,SIGNAL(itemSelectionChanged()),DrawWidget,SLOT(update()));

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
	QObject::connect(CSTree,SIGNAL(NewElectrode()),this,SLOT(NewElectrode()));
	QObject::connect(CSTree,SIGNAL(NewChargeBox()),this,SLOT(NewChargeBox()));
	QObject::connect(CSTree,SIGNAL(NewResBox()),this,SLOT(NewResBox()));
	QObject::connect(CSTree,SIGNAL(NewDumpBox()),this,SLOT(NewDumpBox()));

	QDockWidget *dock = new QDockWidget(tr("Properties and Structures"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea);
	dock->setWidget(CSTree);
	dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

	addDockWidget(Qt::LeftDockWidgetArea,dock);

	GridEditor = new QCSGridEditor(&clGrid);
	QObject::connect(GridEditor,SIGNAL(OpacityChange(int)),DrawWidget,SLOT(setGridOpacity(int)));
	QObject::connect(GridEditor,SIGNAL(OpacityChange(int)),StructureVTK,SLOT(SetGridOpacity(int)));
	QObject::connect(GridEditor,SIGNAL(signalDetectEdges(int)),this,SLOT(DetectEdges(int)));
	QObject::connect(GridEditor,SIGNAL(GridChanged()),StructureVTK,SLOT(RenderGrid()));

	dock = new QDockWidget(tr("Rectilinear Grid"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea);
	dock->setWidget(GridEditor);
	dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::LeftDockWidgetArea,dock);

	QParaSet= new QParameterSet();
	QObject::connect(QParaSet,SIGNAL(ParameterChanged()),this,SLOT(CheckGeometry()));
	QObject::connect(QParaSet,SIGNAL(ParameterChanged()),this,SLOT(setModified()));
	clParaSet=QParaSet;

	dock = new QDockWidget(tr("Parameter"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea);
	dock->setWidget(QParaSet);
	dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::LeftDockWidgetArea,dock);

	BuildToolBar();

	bModified=true;
	GridEditor->SetOpacity(30);
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
	QString msg = QString(Update());
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
		TiXmlAttribute *attr = BC->FirstAttribute();
		while (attr)
		{
			m_BC[attr->Name()] = attr->Value();
			attr = attr->Next();
		}
	}
	return openEMS;
}

int QCSXCAD::GetCurrentPrimitive()
{
	return GetIndex(CSTree->GetCurrentPrimitive());
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
//	QString filter;
//	QString filename=QFileDialog::getOpenFileName(0,tr("Choose geometry file"),NULL,"*.xml *.gym",&filter);
//	cerr << filter.toStdString();
//	if (filename!=NULL) ReadFile(filename);
//	CSTree->UpdateTree();

	QFileDialog FD(this,tr("Choose geometry file"));
	QStringList NameFiler;
	QString xmlFile("XML-File (*.xml)");
	QString gymFile("Gym-File (*.gym)");
	NameFiler << xmlFile << gymFile;
	FD.setFilters(NameFiler);
	FD.setFileMode(QFileDialog::ExistingFile);

	if (FD.exec()==QDialog::Accepted)
	{
		QStringList selectedFiles = FD.selectedFiles();
		if (selectedFiles.isEmpty()) return;
		if (FD.selectedNameFilter()==xmlFile)
		{
			ReadFile(selectedFiles.at(0));
		}
		else if  (FD.selectedNameFilter()==gymFile)
		{
#ifdef __GYM2XML__
			if (QMessageBox::warning(this,tr("Import gym-file"),tr("Import of gym-files is highly experimental. This may cause a crash of this application!\nContinue?"),QMessageBox::Ok,QMessageBox::Cancel)==QMessageBox::Ok)
			{
				Gym2XML converter(this);
				converter.ReadGymFile(selectedFiles.at(0).toStdString().c_str());
				GUIUpdate();
			}
#else
			QMessageBox::warning(this,tr("Import gym-file"),tr("Import of gym-files is not supported in this version!!"),QMessageBox::Ok);
#endif
		}
		else return;
	}
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
			if (newProp!=oldProp) CSTree->SwitchProperty(prim,newProp);
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
//		CSProperties* oldProp=prim->GetProperty();
		CSPrimitives* newPrim=prim->GetCopy();
		if (newPrim==NULL) return;
		QCSPrimEditor* newEdit = new QCSPrimEditor(this,newPrim);
		if (newEdit->exec()==QDialog::Accepted)
		{
//			CSProperties* newProp=newPrim->GetProperty();
//			if (newProp!=oldProp) CSTree->SwitchProperty(newPrim,newProp);
			AddPrimitive(newPrim);
			setModified();
			CSTree->AddPrimItem(newPrim);
		}
		return;
	}
//	CSProperties* prop = CSTree->GetCurrentProperty();
//	if (prop!=NULL)
//	{
//		CSProperties* newProp = prop->
//		int index=GetIndex(prop);
//		QCSPropEditor* newEdit = new QCSPropEditor(this,prop);
//		if (newEdit->exec()==QDialog::Accepted)
//		{
//			CSTree->RefreshItem(index);
//			setModified();
//		}
//	}
}

void QCSXCAD::SetVisibility2All(bool value)
{
	for (size_t n=0; n<vProperties.size();++n)
	{
		CSProperties* prop = vProperties.at(n);
		prop->SetVisibility(value);
		CSTree->RefreshItem(GetIndex(prop));
		if (ViewLevel==0) DrawWidget->update();
		if (ViewLevel==1)
		{
			if (value) StructureVTK->SetPropOpacity(prop->GetUniqueID(),prop->GetFillColor().a);
			else StructureVTK->SetPropOpacity(prop->GetUniqueID(),0);
		}
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


void QCSXCAD::ShowHide()
{
	CSProperties* prop = CSTree->GetCurrentProperty();
	if (prop!=NULL)
	{
		prop->SetVisibility(!prop->GetVisibility());
		CSTree->RefreshItem(GetIndex(prop));
		if (ViewLevel==0) DrawWidget->update();
		if (ViewLevel==1)
		{
			if (prop->GetVisibility()) StructureVTK->SetPropOpacity(prop->GetUniqueID(),prop->GetFillColor().a);
			else StructureVTK->SetPropOpacity(prop->GetUniqueID(),0);
		}
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
			if (QMessageBox::question(this,tr("Delete Property"),tr("\"%1\" contains Primitive(s)!!\n Delete anyway?").arg(prop->GetName().c_str()),QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
			{
				for (size_t i=0;i<qtyPrim;++i)
				{
					CSTree->DeletePrimItem(prop->GetPrimitive(0));
					DeletePrimitive(prop->GetPrimitive(0));
				}
			}
			else return;
		}
		else if (QMessageBox::question(this,tr("Delete Property"),tr("Delete current Property?"),QMessageBox::Yes,QMessageBox::No)!=QMessageBox::Yes) return;
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
		AddPrimitive(newPrim);
		setModified();
		//CSTree->UpdateTree();
		CSTree->AddPrimItem(newPrim);
	}
	else delete newPrim;
}

void QCSXCAD::NewMaterial()
{
	NewProperty(new CSPropMaterial(clParaSet));
}

void QCSXCAD::NewMetal()
{
	NewProperty(new CSPropMetal(clParaSet));
}

void QCSXCAD::NewElectrode()
{
	NewProperty(new CSPropElectrode(clParaSet,GetQtyPropertyType(CSProperties::ELECTRODE)));
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
	DrawWidget->update();
	if (StackWidget->currentIndex()==1) StructureVTK->RenderGeometry();
}

void QCSXCAD::DetectEdges(int nu)
{
	InsertEdges2Grid(nu);
}

void QCSXCAD::BestView()
{
	if (ViewLevel==0)
	{
		double area[6];
		double* SimArea=clGrid.GetSimArea();
		double* ObjArea=GetObjectArea();
		for (int i=0;i<3;++i)
		{
			area[2*i]=SimArea[2*i];
	//		if (area[2*i]>clGrid.GetLine(i,0)) area[2*i]=clGrid.GetLine(i,0);
			if (ObjArea[2*i]<area[2*i]) area[2*i]=ObjArea[2*i];
			area[2*i+1]=SimArea[2*i+1];
	//		if (area[2*i+1]<clGrid.GetLine(i,clGrid.GetQtyLines(i)-1)) area[2*i+1]=clGrid.GetLine(i,clGrid.GetQtyLines(i)-1);
			if (ObjArea[2*1+1]>area[2*i+1]) area[2*i+1]=ObjArea[2*i+1];
		}
		DrawWidget->setDrawArea(area);
	}
	else StructureVTK->ResetView();
}

void QCSXCAD::setXY()
{
	if (ViewLevel==1) StructureVTK->setXY();
	else DrawWidget->setXY();
}

void QCSXCAD::setYZ()
{
	if (ViewLevel==1) StructureVTK->setYZ();
	else DrawWidget->setYZ();
}

void QCSXCAD::setZX()
{
	if (ViewLevel==1) StructureVTK->setZX();
	else DrawWidget->setZX();
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

void QCSXCAD::ExportGeometry_X3D()
{
	QString filename = QFileDialog::getSaveFileName( this, tr("Save X3D-file"), QString(), tr("X3D files (*.x3d)") );
	if (filename.isEmpty())
		return;

	export_X3D x3d( this );
	x3d.save( filename );
}

void QCSXCAD::ExportView2Image()
{
	if (ViewLevel==1)
		StructureVTK->ExportView2Image();
	else
		QMessageBox::warning(this,tr("PNG export"),tr("Not Yet Implemented for 2D view, use 3D instead."),QMessageBox::Ok,QMessageBox::NoButton);
}

void QCSXCAD::GUIUpdate()
{
	CSTree->UpdateTree();
	GridEditor->Update();
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

	mainTB->addAction(QIcon(":/images/filenew.png"),tr("New"),this,SLOT(New()));
	mainTB->addAction(QIcon(":/images/down.png"),tr("Import"),this,SLOT(ImportGeometry()));
	mainTB->addAction(QIcon(":/images/up.png"),tr("Export"),this,SLOT(ExportGeometry()));


	QToolBar *ItemTB = addToolBar(tr("Item View"));

	ItemTB->addAction(tr("CollapseAll"),CSTree,SLOT(collapseAll()));
	ItemTB->addAction(tr("ExpandAll"),CSTree,SLOT(expandAll()));

	ItemTB->addAction(QIcon(":/images/bulb.png"),tr("ShowAll"),this,SLOT(ShowAll()));
	ItemTB->addAction(QIcon(":/images/bulb_off.png"),tr("HideAll"),this,SLOT(HideAll()));

	QToolBar *newObjct = addToolBar(tr("add new Primitive"));

	QAction* newAct = NULL;
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

	newAct = newObjct->addAction(tr("Material"),this,SLOT(NewMaterial()));
	newAct->setToolTip(tr("add new Material-Property"));

	newAct = newObjct->addAction(tr("Metal"),this,SLOT(NewMetal()));
	newAct->setToolTip(tr("add new Metal-Property"));

	newAct = newObjct->addAction(tr("Electrode"),this,SLOT(NewElectrode()));
	newAct->setToolTip(tr("add new Electrode-Property"));

	newAct = newObjct->addAction(tr("ProbeBox"),this,SLOT(NewChargeBox()));
	newAct->setToolTip(tr("add new Probe-Box-Property"));

	newAct = newObjct->addAction(tr("ResBox"),this,SLOT(NewResBox()));
	newAct->setToolTip(tr("add new Res-Box-Property"));

	newAct = newObjct->addAction(tr("DumpBox"),this,SLOT(NewDumpBox()));
	newAct->setToolTip(tr("add new Dump-Box-Property"));

	newObjct = addToolBar(tr("Zoom"));

	newAct = newObjct->addAction(QIcon(":/images/viewmagfit.png"),tr("Zoom fit"),this,SLOT(BestView()));

	//QActionGroup* ActGrp = new QActionGroup(this);
	newAct = newObjct->addAction(tr("XY"),this,SLOT(setXY()));
//	connect(newAct,SIGNAL(triggered()),this,SLOT(setXY()));
	//ActGrp->addAction(newAct);
	//newAct->setCheckable(true);
	//newAct->setChecked(true);
	newAct = newObjct->addAction(tr("YZ"),this,SLOT(setYZ()));
//	connect(newAct,SIGNAL(triggered()),this,SLOT(setYZ()));
	//newAct->setCheckable(true);
	//ActGrp->addAction(newAct);
	newAct = newObjct->addAction(tr("ZX"),this,SLOT(setZX()));
//	connect(newAct,SIGNAL(triggered()),this,SLOT(setZX()));
	//newAct->setCheckable(true);
	//ActGrp->addAction(newAct);

	addToolBarBreak();

	QActionGroup* ActViewGrp = new QActionGroup(this);
	newAct = newObjct->addAction(tr("2D"),this,SLOT(View2D()));
	ActViewGrp->addAction(newAct);
	newAct->setCheckable(true);
	newAct->setChecked(true);
	newAct = newObjct->addAction(tr("3D"),this,SLOT(View3D()));
	newAct->setCheckable(true);
	ActViewGrp->addAction(newAct);


	addToolBar(GridEditor->BuildToolbar());
}

void QCSXCAD::View2D()
{
	ViewLevel=0;
	StackWidget->setCurrentIndex(ViewLevel);
	StructureVTK->SetUpdateMode(false);
}

void QCSXCAD::View3D()
{
	ViewLevel=1;

	StackWidget->setCurrentIndex(ViewLevel);
	StructureVTK->SetUpdateMode(true);
	StructureVTK->RenderGrid();
	StructureVTK->RenderGeometry();
}

void QCSXCAD::keyPressEvent(QKeyEvent * event)
{
	if (event->key()==Qt::Key_Delete) Delete();
	if (event->key()==Qt::Key_Escape)
	{
		CSTree->setCurrentItem(NULL);
		DrawWidget->Reset();
	}
	QMainWindow::keyPressEvent(event);
}

QGeometryPlot::QGeometryPlot(QCSXCAD* CS) : QWidget()
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	direct=2;
	clCS=CS;
	spacer=10;
	GridOpacity=0;
	for (unsigned int i=0;i<6;++i) DrawArea[i]=(2.0*(double)(i%2)-1.0)*1000.0;
	setCursor(Qt::CrossCursor);
	setMouseTracking(true);
	statBar=NULL;
	bArrow=false;
}

QGeometryPlot::~QGeometryPlot()
{
}

void QGeometryPlot::setXY()
{
	direct=2;
	Reset();
}

void QGeometryPlot::setYZ()
{
	direct=0;
	Reset();
}

void QGeometryPlot::setZX()
{
	direct=1;
	Reset();
}

void QGeometryPlot::setDrawArea(double* area)
{
	if ((area[0]==area[1])  && (area[2]==area[3])  && (area[4]==area[5])) return;
	for (unsigned int i=0;i<6;++i) DrawArea[i]=area[i];
	update();
}

void QGeometryPlot::setGridOpacity(int val)
{
	GridOpacity=val;
	update();
}

void QGeometryPlot::Reset()
{
	bArrow = false;
	update();
}

void QGeometryPlot::paintEvent(QPaintEvent * /* event */)
{
	CSPrimitives* prim=NULL;
	CSProperties* prop=NULL;
	QPainter painter(this);

	int x=(direct+1)%3;
	int y=(direct+2)%3;

	double widthX = (DrawArea[2*x+1]-DrawArea[2*x]);
	double widthY = (DrawArea[2*y+1]-DrawArea[2*y]);

	double factorX=widthX/(double)(width()-2*spacer);
	double factorY=widthY/(double)(height()-2*spacer);
	factor=factorX;
	if (factor<factorY) factor=factorY;
//	factor*=1.05;
	offsetX=DrawArea[2*x]+(widthX-(double)(width())*factor)/2;
	offsetY=DrawArea[2*y]+(widthY-(double)(height())*factor)/2;;
	double help;

	if (GridOpacity>0)
	{
		QPen pen(QColor(0,0,0,GridOpacity));
		painter.setPen(pen);

		CSRectGrid* grid = clCS->GetGrid();
		int iX=0;
		int iY1=height()-(int)((grid->GetLine(y,0)-offsetY)/factor);
		int iY2=height()-(int)((grid->GetLine(y,grid->GetQtyLines(y)-1)-offsetY)/factor);
		for (size_t n=0;n<grid->GetQtyLines(x);++n)
		{
			iX=(int)((grid->GetLine(x,n)-offsetX)/factor);
			painter.drawLine(iX,iY1,iX,iY2);
		}
		int iY=0;
		int iX1=(int)((grid->GetLine(x,0)-offsetX)/factor);
		int iX2=(int)((grid->GetLine(x,grid->GetQtyLines(x)-1)-offsetX)/factor);
		for (size_t n=0;n<grid->GetQtyLines(y);++n)
		{
			iY=height()-(int)((grid->GetLine(y,n)-offsetY)/factor);
			painter.drawLine(iX1,iY,iX2,iY);
		}
	}
	int currPrim=clCS->GetCurrentPrimitive();
	int currProp=clCS->GetCurrentProperty();

	for (size_t i=0;i<clCS->GetQtyPrimitives();++i)
	{
		prim=clCS->GetPrimitive(i);
		if (prim==NULL) break;
		prop=prim->GetProperty();
		if (prop==NULL) break;
		if (prop->GetVisibility())
		{
			RGBa color=prop->GetFillColor();
			//QBrush brush(QColor(color.R,color.G,color.B,0));
			//painter.setBrush(brush);
			color=prop->GetEdgeColor();
			QPen pen(QColor(color.R,color.G,color.B,255));
			if (currPrim>=0) {if ((size_t)currPrim==i) pen.setWidth(2);}
			else
			{
				if (currProp>=0) { if (prop->GetID()==(unsigned int)currProp) pen.setWidth(2);}
			}
			painter.setPen(pen);

			switch (prim->GetType())
			{
				case CSPrimitives::BOX:
				{
					CSPrimBox* box=prim->ToBox();
					double x1=(box->GetCoord(2*x)-offsetX)/factor;
					double y1=(box->GetCoord(2*y)-offsetY)/factor;
					double x2=(box->GetCoord(2*x+1)-offsetX)/factor;
					double y2=(box->GetCoord(2*y+1)-offsetY)/factor;
					if (x1>x2) {help=x2;x2=x1;x1=help;}
					if (y1>y2) {help=y2;y2=y1;y1=help;}
					painter.drawRect(QRect((int)x1,height()-(int)y2,(int)(x2-x1),(int)(y2-y1)));
					break;
				}
				case CSPrimitives::MULTIBOX:
				{
					CSPrimMultiBox* MultiBox=prim->ToMultiBox();
					for (unsigned int n=0;n<MultiBox->GetQtyBoxes();++n)
					{
						double x1=(MultiBox->GetCoord(6*n+2*x)-offsetX)/factor;
						double y1=(MultiBox->GetCoord(6*n+2*y)-offsetY)/factor;
						double x2=(MultiBox->GetCoord(6*n+2*x+1)-offsetX)/factor;
						double y2=(MultiBox->GetCoord(6*n+2*y+1)-offsetY)/factor;
						if (x1>x2) {help=x2;x2=x1;x1=help;}
						if (y1>y2) {help=y2;y2=y1;y1=help;}
						painter.drawRect(QRect((int)x1,height()-(int)y2,(int)(x2-x1),(int)(y2-y1)));
					}
					break;
				}
				case CSPrimitives::SPHERE:
				{
					CSPrimSphere* sphere=prim->ToSphere();
					double x1=(sphere->GetCoord(x)-offsetX)/factor;
					double y1=(sphere->GetCoord(y)-offsetY)/factor;
					double r=sphere->GetRadius()/factor;
					painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
					break;
				}
				case CSPrimitives::CYLINDER:
				{  //erstmal nur kartesische Cylinder
					CSPrimCylinder* cylinder=prim->ToCylinder();
					double r0[3]={cylinder->GetCoord(0),cylinder->GetCoord(2),cylinder->GetCoord(4)};
					double r1[3]={cylinder->GetCoord(1),cylinder->GetCoord(3),cylinder->GetCoord(5)};
					double x1=(r0[x]-offsetX)/factor;
					double y1=(r0[y]-offsetY)/factor;
					double x2=(r1[x]-offsetX)/factor;
					double y2=(r1[y]-offsetY)/factor;
					double r=cylinder->GetRadius()/factor;
					if (r0[1]==r1[1] && r0[2]==r1[2]) //x-ausdehnung
					{
						if (direct==0)
						{
							painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
						}
						else if (direct==2)
						{
							painter.drawRect(QRect((int)x1,height()-(int)(y2+r),(int)(x2-x1),(int)(y2-y1+2*r)));
						}
						else if (direct==1)
						{
							painter.drawRect(QRect((int)(x1-r),height()-(int)y2,(int)(x2-x1+2*r),(int)(y2-y1)));
						}
					}
					else if (r0[0]==r1[0] && r0[2]==r1[2]) //y-ausdehnung
					{
						if (direct==1)
						{
							painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
						}
						else if (direct==0)
						{
							painter.drawRect(QRect((int)x1,height()-(int)(y2+r),(int)(x2-x1),(int)(y2-y1+2*r)));
						}
						else if (direct==2)
						{
							painter.drawRect(QRect((int)(x1-r),height()-(int)y2,(int)(x2-x1+2*r),(int)(y2-y1)));
						}
					}
					else if (r0[1]==r1[1] && r0[0]==r1[0]) //z-ausdehnung
					{
						if (direct==2)
						{
							painter.drawEllipse(QRect((int)(x1-r),height()-(int)(y1+r),(int)(r*2),(int)(2*r)));
						}
						else if (direct==1)
						{
							painter.drawRect(QRect((int)x1,height()-(int)(y2+r),(int)(x2-x1),(int)(y2-y1+2*r)));
						}
						else if (direct==0)
						{
							painter.drawRect(QRect((int)(x1-r),height()-(int)y2,(int)(x2-x1+2*r),(int)(y2-y1)));
						}
					}
					break;
				}
				case CSPrimitives::POLYGON:
				{
					CSPrimPolygon* poly=prim->ToPolygon();
					double elev = poly->GetElevation();
					int normDir = 0;
					if (poly->GetNormDir(0) != 0) normDir = 0;
					else if (poly->GetNormDir(1) != 0) normDir = 1;
					else if (poly->GetNormDir(2) != 0) normDir = 2;
					int nrPts = poly->GetQtyCoords();
					QPointF points[nrPts+1];
					for (int n=0;n<nrPts;++n)
					{
						if (normDir == direct)
						{
							points[n].setX((poly->GetCoord(2*n)-offsetX)/factor);
							points[n].setY(height()-(poly->GetCoord(2*n+1)-offsetY)/factor);
						}
						else if ( normDir == ((direct+2)%3) )
						{
							points[n].setX((poly->GetCoord(2*n+1)-offsetX)/factor);
							points[n].setY(height()-(elev-offsetY)/factor);
						}
						else if ( normDir == ((direct+1)%3) )
						{
							points[n].setX((elev-offsetX)/factor);
							points[n].setY(height()-(poly->GetCoord(2*n)-offsetY)/factor);
						}
						else
							break;
					}
					points[nrPts] = points[0];
					painter.drawPolyline(points,nrPts+1);
					break;
				}
				case CSPrimitives::CURVE:
				case CSPrimitives::WIRE:
				{
					CSPrimCurve* curve = NULL;
					if (prim->GetType()==CSPrimitives::CURVE)
						curve = prim->ToCurve();
					else
						curve = prim->ToWire();

					unsigned int nrPts = (unsigned int)curve->GetNumberOfPoints();
					QPointF points[nrPts];
					double xyz[3];
					int nP = (direct+1)%3;
					int nPP = (direct+2)%3;
					for (unsigned int n=0;n<nrPts;++n)
					{
						curve->GetPoint(n,xyz);
						points[n].setX((xyz[nP]-offsetX)/factor);
						points[n].setY(height()-(xyz[nPP]-offsetY)/factor);
					}
					if (prim->GetType()==CSPrimitives::CURVE)
						painter.drawPolyline(points,nrPts);
					else
					{
//						cerr << "QCSXCAD: Warning: 2D painting of wire currently with thin pen only..." << endl;
						painter.drawPolyline(points,nrPts);
					}
					break;
				}
			};
		}
	}
	
	//calculate and draw an arrow here
	if (bArrow)
	{
		QPen pen(QColor(255,0,0,255));
		pen.setWidth(2);
		painter.setPen(pen);
		int x1 = (int)((dArrow[0]-offsetX)/factor);
		int y1 = height()-(int)((dArrow[1]-offsetY)/factor);
		int x2 = (int)((dArrow[2]-offsetX)/factor);
		int y2 = height()-(int)((dArrow[3]-offsetY)/factor);
		
		painter.drawLine(x1,y1,x2,y2);
		painter.drawText(x1,y1,QString("(%1,%2)").arg(dArrow[0]).arg(dArrow[1]));
		painter.drawText(x2,y2,QString("(%1,%2)").arg(dArrow[2]).arg(dArrow[3]));
		
		double angle = atan2(y2-y1,x2-x1);
		double PI = 3.141;
		painter.drawLine(x2,y2,x2-10*cos(angle-PI/4),y2-10*sin(angle-PI/4));
		painter.drawLine(x2,y2,x2-10*cos(angle+PI/4),y2-10*sin(angle+PI/4));
	}

	QRectF target(10, height()-110.0, 100.0, 100.0);
	//QRectF source(0.0, 0.0, 70.0, 40.0);
	QImage image;
	if (direct==2) image=QImage(":/images/ArrowsXY.png");
	if (direct==1) image=QImage(":/images/ArrowsXZ.png");
	if (direct==0) image=QImage(":/images/ArrowsYZ.png");
	QRect source=image.rect();

	painter.drawImage(target, image, source);
	
//	QPen pen;
//	pen.setWidth(5);
//	painter.setPen(pen);
//	QPointF points[5];
//	if (direct=2)
//	{
//		pen.setColor(QColor(0,0,255,255));
//		points[0]=QPointF(10.0, height()-20.0);
//		points[1]=QPointF(60.0, height()-20.0);
//		points[2]=QPointF(50.0, height()-30.0);
//		points[3]=QPointF(50.0, height()-10.0);
//		points[4]=QPointF(60.0, height()-20.0);
//		painter.drawPolyline(points, 5);
//	}

	// *****
	// * indicate the boundary conditions

	// calculate offset in
	QRect v = painter.viewport();
	QRect w = painter.window();
	int offset1 = 10 * (double)w.width() / (double)v.width();
	int offset2 = 10 * (double)w.height() / (double)v.height();

	CSRectGrid* grid = clCS->GetGrid();
	int iY1 = height()-(int)((grid->GetLine(y,0)-offsetY)/factor);
	int iY2 = height()-(int)((grid->GetLine(y,grid->GetQtyLines(y)-1)-offsetY)/factor);
	int iX1 = (int)((grid->GetLine(x,0)-offsetX)/factor);
	int iX2 = (int)((grid->GetLine(x,grid->GetQtyLines(x)-1)-offsetX)/factor);
	int iY = (iY1 + iY2) / 2.0;
	int iX = (iX1 + iX2) / 2.0;

	QString xmin, xmax, ymin, ymax;
	QHash<QString,QString> BC = clCS->Get_BC();
	if (direct == 0) {
		// y-z plane
		xmin = BC.value("ymin");
		xmax = BC.value("ymax");
		ymin = BC.value("zmin");
		ymax = BC.value("zmax");
	}
	else if (direct == 1) {
		// x-z plane
		xmin = BC.value("zmin");
		xmax = BC.value("zmax");
		ymin = BC.value("xmin");
		ymax = BC.value("xmax");
	}
	else if (direct == 2) {
		// x-y plane
		xmin = BC.value("xmin");
		xmax = BC.value("xmax");
		ymin = BC.value("ymin");
		ymax = BC.value("ymax");
	}

	painter.setPen(Qt::black);
	painter.save();
	painter.translate( iX1-offset1, iY );
	painter.rotate(-90);
	painter.drawText( 0, 0, 0, 0, Qt::AlignCenter|Qt::TextDontClip, xmin );
	painter.restore();
	painter.save();
	painter.translate( iX2+offset1, iY );
	painter.rotate(-90);
	painter.drawText( 0, 0, 0, 0, Qt::AlignCenter|Qt::TextDontClip, xmax );
	painter.restore();
	painter.drawText( iX, iY1+offset2, 0, 0, Qt::AlignCenter|Qt::TextDontClip, ymin );
	painter.drawText( iX, iY2-offset2, 0, 0, Qt::AlignCenter|Qt::TextDontClip, ymax );
}

void QGeometryPlot::wheelEvent(QWheelEvent * event)
{
	double zoomFac=(double)(event->delta());
	zoomFac/=120;
	zoomFac/=-20;

	for (int i=0;i<3;++i)
	{
		double width=(DrawArea[2*i+1]-DrawArea[2*i])*zoomFac;
		DrawArea[2*i]=DrawArea[2*i]-width;
		DrawArea[2*i+1]=DrawArea[2*i+1]+width;
	}
	if (event->buttons()==Qt::LeftButton)
	{
		GetMouseXY(event->pos());
		dArrow[2]=lastMouseXY[0];
		dArrow[3]=lastMouseXY[1];
	}
	update();
}


void QGeometryPlot::mousePressEvent(QMouseEvent * event)
{
	Pos=event->pos();
	if (event->button()==Qt::RightButton)
	{
		setCursor(Qt::ClosedHandCursor);
	}
	if (event->button()==Qt::LeftButton)
	{
		bArrow=true;
		GetMouseXY(event->pos());
		dArrow[0]=lastMouseXY[0];
		dArrow[1]=lastMouseXY[1];
	}
}



void QGeometryPlot::mouseReleaseEvent(QMouseEvent * event)
{
	setCursor(Qt::CrossCursor);
	if (event->button()==Qt::LeftButton)
	{
		GetMouseXY(event->pos());
		dArrow[2]=lastMouseXY[0];
		dArrow[3]=lastMouseXY[1];
		if (dArrow[0]==dArrow[2] && dArrow[1]==dArrow[3]) bArrow=false;
		else bArrow=true;
	}
}

void QGeometryPlot::mouseMoveEvent(QMouseEvent * event)
{
	if (statBar!=NULL)
	{
		GetMouseXY(event->pos());
		if (direct==0) statBar->showMessage(QString("Y: %1  Z: %2").arg(lastMouseXY[0]).arg(lastMouseXY[1]));
		if (direct==1) statBar->showMessage(QString("Z: %1  X: %2").arg(lastMouseXY[0]).arg(lastMouseXY[1]));
		if (direct==2) statBar->showMessage(QString("X: %1  Y: %2").arg(lastMouseXY[0]).arg(lastMouseXY[1]));
	}

	if ((event->buttons()==Qt::LeftButton) && (bArrow)) 
	{
		GetMouseXY(event->pos());
		dArrow[2]=lastMouseXY[0];
		dArrow[3]=lastMouseXY[1];
		update(); 
		return;
	}
	if (event->buttons()!=Qt::RightButton) return;

	double shiftX=-1*(Pos.x()-event->x())*factor;
	double shiftY=(Pos.y()-event->y())*factor;


	int x=(direct+1)%3;
	int y=(direct+2)%3;

	DrawArea[2*x]=DrawArea[2*x]-shiftX;
	DrawArea[2*x+1]=DrawArea[2*x+1]-shiftX;

	DrawArea[2*y]=DrawArea[2*y]-shiftY;
	DrawArea[2*y+1]=DrawArea[2*y+1]-shiftY;

	Pos=event->pos();
	update();
}


double* QGeometryPlot::GetMouseXY(const QPoint qp, bool bRound)
{
	lastMouseXY[0]=qp.x()*factor+offsetX;
	lastMouseXY[1]=(height()-qp.y())*factor+offsetY;
	if (bRound==false) return lastMouseXY;
	double logfac = pow(10,round(log10(factor)));
	lastMouseXY[0] = round(lastMouseXY[0]/logfac)*logfac;	
	lastMouseXY[1] = round(lastMouseXY[1]/logfac)*logfac;
	return lastMouseXY;
}




