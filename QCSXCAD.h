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

#ifndef _QCSXCAD_H_
#define _QCSXCAD_H_

#include <QtGui>
#include "QCSXCAD_Global.h"
#include "ContinuousStructure.h"
using namespace std;

class QVTKStructure;
class QCSPrimEditor;
class QCSPropEditor;
class QCSTreeWidget;
class QCSGridEditor;
class QParameterGui;
class QParameterSet;

class QGeometryPlot;

class TiXmlNode;

class QCSXCAD_EXPORT QCSXCAD : public QMainWindow, public ContinuousStructure
{
	Q_OBJECT
	friend class export_X3D;
	friend class export_pov;
public:
	QCSXCAD(QWidget *parent=NULL);
	virtual ~QCSXCAD();

//	void SetFile(QString filename);
//	void SetPath(QString path) {relPath=path;};
//	QString GetFilename();

	//StaticComplexGeometry* GetDiscGeometry() {return SCGeometry;};
	QString GetGeometry();

	virtual bool isGeometryValid();

	virtual bool Write2XML(TiXmlNode* rootNode, bool parameterised=true);
	virtual bool Write2XML(const char* file, bool parameterised=true);
	virtual bool Write2XML(QString file, bool parameterised=true);

	virtual bool ReadFile(QString filename);
	bool ReadNode(TiXmlNode* root);

	int GetCurrentPrimitive();
	int GetCurrentProperty();

	ParameterSet* GetParaSet();

	static QString GetInfoString();
	static QIcon GetLibIcon();

	const QHash<QString,QString> &GetFDTD_BC() const { return m_FDTD_BC; }

signals:
	void modified(bool val);

public slots:
	bool CheckGeometry();

	void ImportGeometry();
	void ExportGeometry();
	void ExportGeometry_Povray();
	void ExportGeometry_X3D();

	void ExportView2Image();

	virtual void clear();
	void New();

	void HideAll();
	void ShowAll();

	void BestView();
	void setXY();
	void setYZ();
	void setZX();

	void SetSimMode(int mode);

	void GUIUpdate();

	static void aboutQCSXCAD(QWidget* parent=0);

protected slots:
	void Edit();
	void Copy();
	void ShowHide();

	void Delete();

	void NewBox();
	void NewMultiBox();
	void NewSphere();
	void NewCylinder();
	void NewPolygon();
	void NewUserDefined();

	void NewMaterial();
	void NewMetal();
	void NewElectrode();
	void NewChargeBox();
	void NewResBox();
	void NewDumpBox();

	void setModified();

	void DetectEdges(int nu);

	void View2D();
	void View3D();

protected:

	void NewPrimitive(CSPrimitives* newPrim);
	void NewProperty(CSProperties* newProp);

	TiXmlNode* FindRootNode(TiXmlNode* node);

	void SetVisibility2All(bool value);

	QCSTreeWidget *CSTree;
	QCSGridEditor* GridEditor;
	QGeometryPlot* DrawWidget;
	QParameterSet* QParaSet;

	QVTKStructure* StructureVTK;
	QStackedWidget* StackWidget;

//	QString QFilename;
//	QString relPath;

	void BuildToolBar();

	bool bModified;
	int ViewLevel;
	int m_SimMode;

	virtual void keyPressEvent(QKeyEvent * event);

	QHash<QString,QString> m_FDTD_BC; //!< boundary conditions from FDTD element in xml file
};

class QGeometryPlot : public QWidget
{
	Q_OBJECT
public:
	QGeometryPlot(QCSXCAD* CS);
	~QGeometryPlot();

	void SetStatusBar(QStatusBar* bar) {statBar=bar;};

public slots:
	void setXY();
	void setYZ();
	void setZX();
	void setDrawArea(double* area);
	void setGridOpacity(int val);
	
	void Reset();

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void wheelEvent(QWheelEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	
	double* GetMouseXY(const QPoint qp, bool bRound=true);
	double lastMouseXY[2];
	
	bool bArrow;
	double dArrow[4];

	QCSXCAD* clCS;
	int direct;
	int spacer;
	int GridOpacity;
	double DrawArea[6];

	double factor;
	double offsetX,offsetY;

	QStatusBar* statBar;

	QPoint Pos;
};

#endif //_QCSXCAD_H_
