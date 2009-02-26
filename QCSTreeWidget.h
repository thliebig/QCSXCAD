#ifndef _QCSTREEWIDGET_H_
#define _QCSTREEWIDGET_H_

#include <QtGui>
#include "ContinuousStructure.h"

#define PROPTYPE 0
#define PRIMTYPE 1

class QCSTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:
	QCSTreeWidget(ContinuousStructure* CS, QWidget * parent = 0);
	virtual ~QCSTreeWidget();
	
	CSProperties* GetCurrentProperty();
	CSPrimitives* GetCurrentPrimitive();
	
	void AddPrimItem(CSPrimitives* prim);
	void AddPropItem(CSProperties* prop);
	
	void DeletePrimItem(CSPrimitives* prim);
	void DeletePropItem(CSProperties* prop);
	
	void RefreshItem(int index);

	void UpdateTree();
	void ClearTree();

signals:
	void Edit();
	void Copy();
	void ShowHide();

	void NewBox();
	void NewMultiBox();
	void NewSphere();
	void NewCylinder();	
	void NewUserDefined();

	void NewMaterial();
	void NewMetal();
	void NewElectrode();
	void NewChargeBox();
	void NewResBox();
	void NewDumpBox();

	void Delete();
	
public slots:
	void SwitchProperty(CSPrimitives* prim, CSProperties* newProp);
	
	void collapseAll();
	void expandAll();

protected slots:
	void Edit(QTreeWidgetItem * item, int column);
	void Clicked(QTreeWidgetItem * item, int column);
	
protected:
	void contextMenuEvent(QContextMenuEvent *event);

//	void BuildContextMenu();
	ContinuousStructure* clCS;
	
	QVector<QTreeWidgetItem*> vPrimItems;
	
};

#endif //_QCSTREEWIDGET_H_
