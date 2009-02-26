#include "QCSTreeWidget.h"

QCSTreeWidget::QCSTreeWidget(ContinuousStructure* CS, QWidget * parent) : QTreeWidget(parent)
{
	clCS=CS;
	setColumnCount(2);
	setColumnWidth(0,200);
//	setColumnWidth(1,16);
	setHeaderLabels(QStringList(tr("Properties / Primitives"))<<tr("Vis"));
//	setItemHidden(headerItem(),true);
//	qTree->setDragEnabled(true);
//	qTree->setAcceptDrops(true);
//	qTree->setDropIndicatorShown(true);
	QObject::connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(Edit(QTreeWidgetItem*,int)));
	QObject::connect(this,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(Clicked(QTreeWidgetItem*,int)));
}

QCSTreeWidget::~QCSTreeWidget()
{
}

CSProperties* QCSTreeWidget::GetCurrentProperty()
{
	QTreeWidgetItem* curr=currentItem();
	if (curr==NULL) return NULL;
	if (curr->type()==PRIMTYPE) curr=curr->parent();
	if (curr==NULL) return NULL;
	return clCS->GetProperty(indexOfTopLevelItem(curr));
}

CSPrimitives* QCSTreeWidget::GetCurrentPrimitive()
{
	QTreeWidgetItem* curr=currentItem();
	if (curr==NULL) return NULL;
	if (curr->type()!=PRIMTYPE) return NULL;
	return clCS->GetPrimitiveByID(curr->data(0,1).toInt());
}

void QCSTreeWidget::AddPrimItem(CSPrimitives* prim)
{
	if (prim==NULL) return;
	int propID=clCS->GetIndex(prim->GetProperty());
	if (propID<0) return;
	QTreeWidgetItem* parent = topLevelItem(propID);
	if (parent==NULL) return;
	QString str;
	switch (prim->GetType())
	{
		case CSPrimitives::BOX:
			str="Box";
			break;
		case CSPrimitives::MULTIBOX:
			str="Multi Box";
			break;
		case CSPrimitives::SPHERE:
			str="Sphere";
			break;
		case CSPrimitives::CYLINDER:
			str="Cylinder";
			break;
		case CSPrimitives::USERDEFINED:
			str="User Defined";
			break;
		default:
			str="Unkown";
			break;					
	};
	str+=QString(" - ID: %1").arg(prim->GetID());
	QTreeWidgetItem* newPrimItem = new QTreeWidgetItem(parent,QStringList(str),1);
	newPrimItem->setData(0,1,QVariant(prim->GetID()));
	vPrimItems.push_back(newPrimItem);
}

void QCSTreeWidget::AddPropItem(CSProperties* prop)
{
	QString str;
	if (prop==NULL) return;
	str=QString(prop->GetTypeString())+"::";
	str+=QString(prop->GetName());
		
	QTreeWidgetItem* newItem = new QTreeWidgetItem(this,QStringList(str),0);
	newItem->setFont(0,QFont("Arial",10));
	if (prop->GetVisibility()) newItem->setIcon(1,QIcon(":/images/bulb.png"));
	else newItem->setIcon(1,QIcon(":/images/bulb_off.png"));
}
	
void QCSTreeWidget::DeletePrimItem(CSPrimitives* prim)
{
	int index=clCS->GetIndex(prim);
	QTreeWidgetItem *item=NULL;
	if ((index>=0) && (index<vPrimItems.size())) item=vPrimItems.at(index);
	else return;
	//QTreeWidgetItem *parent=item->parent();
	vPrimItems.remove(index);
	//if (parent==NULL) return;
	delete item;
}

void QCSTreeWidget::DeletePropItem(CSProperties* prop)
{
	int index=clCS->GetIndex(prop);
	
	QTreeWidgetItem* parent = topLevelItem(index);
	if (parent==NULL) return;
	
	delete parent;
}

void QCSTreeWidget::RefreshItem(int index)
{
	CSProperties* prop=clCS->GetProperty(index);
	if (prop==NULL) return;
	QTreeWidgetItem* item = topLevelItem(index);
	if (item==NULL) return;
	QString str=QString(prop->GetTypeString())+"::";
	str+=QString(prop->GetName());
	item->setText(0,str);	
	if (prop->GetVisibility()) item->setIcon(1,QIcon(":/images/bulb.png"));
	else item->setIcon(1,QIcon(":/images/bulb_off.png"));
}


void QCSTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	QMenu* primM = menu.addMenu(QIcon(":/images/edit_add.png"),tr("New Primitive"));
	primM->addAction(tr("Box"),this,SIGNAL(NewBox()));
	primM->addAction(tr("Multi-Box"),this,SIGNAL(NewMultiBox()));
	primM->addAction(tr("Sphere"),this,SIGNAL(NewSphere()));
	primM->addAction(tr("Cylinder"),this,SIGNAL(NewCylinder()));
	primM->addAction(tr("User Defined"),this,SIGNAL(NewUserDefined()));
	
	QMenu* propM = menu.addMenu(QIcon(":/images/edit_add.png"),tr("New Property"));
	propM->addAction(tr("Material"),this,SIGNAL(NewMaterial()));
	propM->addAction(tr("Metal"),this,SIGNAL(NewMetal()));
	propM->addAction(tr("Electrode"),this,SIGNAL(NewElectrode()));
	propM->addAction(tr("Probe-Box"),this,SIGNAL(NewChargeBox()));
	propM->addAction(tr("Res-Box"),this,SIGNAL(NewResBox()));
	propM->addAction(tr("Dump-Box"),this,SIGNAL(NewDumpBox()));
	
	menu.addSeparator();
	
	menu.addAction(QIcon(":/images/edit.png"),tr("Edit"),this,SIGNAL(Edit()));
	menu.addAction(QIcon(":/images/editcopy.png"),tr("Copy"),this,SIGNAL(Copy()));
	menu.addAction(QIcon(":/images/edit_remove.png"),tr("Delete"),this,SIGNAL(Delete()));

	menu.exec(event->globalPos());
}

void QCSTreeWidget::UpdateTree()
{
	ClearTree();
	for (size_t i=0;i<clCS->GetQtyProperties();++i)
	{
		QString str;
		CSProperties* prop=clCS->GetProperty(i);
		if (prop==NULL) break;
		AddPropItem(prop);
	}
	for (size_t i=0;i<clCS->GetQtyPrimitives();++i)
	{
		QString str;
		CSPrimitives* prim=clCS->GetPrimitive(i);
		if (prim==NULL) break;
		AddPrimItem(prim);
	}
}

void QCSTreeWidget::ClearTree()
{
	this->clear();
	vPrimItems.clear();
}

void QCSTreeWidget::SwitchProperty(CSPrimitives* prim, CSProperties* newProp)
{
	int index=clCS->GetIndex(prim);
	QTreeWidgetItem *item=NULL;
	if ((index>=0) && (index<vPrimItems.size())) item=vPrimItems.at(index);	
	else return;

	QTreeWidgetItem *parent=item->parent();
	if (parent==NULL) return;
	QTreeWidgetItem *newParent = topLevelItem(clCS->GetIndex(newProp));
	if (newParent==NULL) return;

	parent->takeChild(parent->indexOfChild(item));
	newParent->addChild(item);
	
}

void QCSTreeWidget::collapseAll()
{
	for (int i=0;i<topLevelItemCount();++i) collapseItem(topLevelItem(i));
}

void QCSTreeWidget::expandAll()
{
	for (int i=0;i<topLevelItemCount();++i) expandItem(topLevelItem(i));
}

void QCSTreeWidget::Edit(QTreeWidgetItem * item, int column)
{
	if (item==NULL) return;
	if (item->type()!=PRIMTYPE) return;
	emit Edit();
}

void QCSTreeWidget::Clicked(QTreeWidgetItem * item, int column)
{
	if (item==NULL) return;
	if (item->type()!=PROPTYPE) return;
	if (column==1) {emit ShowHide();}
}
