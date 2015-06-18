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

#include <QMainWindow>
#include <QHash>
#include <QString>
#include "QCSXCAD_Global.h"
#include "ContinuousStructure.h"

class QVTKStructure;
class QCSPrimEditor;
class QCSPropEditor;
class QCSTreeWidget;
class QCSGridEditor;
class QParameterGui;
class QParameterSet;

class TiXmlNode;

class QCSXCAD_EXPORT QCSXCAD : public QMainWindow, public ContinuousStructure
{
	Q_OBJECT
	friend class export_X3D;
	friend class export_pov;
public:
	QCSXCAD(QWidget *parent=NULL);
	virtual ~QCSXCAD();

	enum ViewMode
	{
		VIEW_2D, VIEW_3D
	};

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

	int GetCurrentProperty();

	ParameterSet* GetParaSet();

	static QString GetInfoString();
	static QIcon GetLibIcon();

	const QHash<QString,QString> &Get_BC() const { return m_BC; }

signals:
	void modified(bool val);

public slots:
	bool CheckGeometry();

	void ImportGeometry();
	void ExportGeometry();
	void ExportGeometry_Povray();
	void ExportGeometry_X3D(QString filename=QString());

	//! Export geometry into a given directory and type
	void ExportGeometry(QString dirname, int type);
	void ExportGeometry_PolyDataVTK(QString dirname=QString());
	void ExportGeometry_STL(QString dirname=QString());
	void ExportGeometry_PLY(QString dirname=QString());

	void ExportView2Image();

	void EnableDiscModelRendering(bool val=true);

	void Render();
	virtual void clear();
	void New();

	void HideAll();
	void ShowAll();

	//! Enable/Disable 3D parallel projection for the 3D vtk viewer
	void SetParallelProjection(bool val);

	void BestView();
	void setViewDir(int dir);
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
	void NewExcitation();
	void NewChargeBox();
	void NewResBox();
	void NewDumpBox();

	void setModified();

	void DetectEdges(int nu);

	void View2D();
	void View3D();

protected:
	//read supported files, return the root to a CSX
	TiXmlNode* ReadOpenEMS(TiXmlNode* openEMS);

	void NewPrimitive(CSPrimitives* newPrim);
	void NewProperty(CSProperties* newProp);

	TiXmlNode* FindRootNode(TiXmlNode* node);

	void SetVisibility2All(bool value);

	bool m_RenderDiscModels;

	QCSTreeWidget *CSTree;
	QCSGridEditor* GridEditor;
	QParameterSet* QParaSet;

	QVTKStructure* StructureVTK;

	QAction* viewPlane[3];
	void BuildToolBar();

	bool bModified;
	ViewMode ViewLevel;
	QAction* m_PPview;
	int m_ViewDir;
	int m_SimMode;

	virtual void keyPressEvent(QKeyEvent * event);

	QHash<QString,QString> m_BC; //!< boundary conditions
};

#endif //_QCSXCAD_H_
