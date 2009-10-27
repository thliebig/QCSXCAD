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

class QCSXCAD_EXPORT QCSXCAD : public QMainWindow, public ContinuousStructure
{
	Q_OBJECT
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

signals:
	void modified(bool val);

public slots:
	bool CheckGeometry();

	void ImportGeometry();
	void ExportGeometry();

	virtual void clear();
	void New();

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

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void wheelEvent(QWheelEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	
	double* GetMouseXY(QMouseEvent* event, bool bRound=true);
	double lastMouseXY[2];

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
