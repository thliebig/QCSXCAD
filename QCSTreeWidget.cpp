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

#include <QMenu>
#include <QContextMenuEvent>

#include "QCSTreeWidget.h"
#include "QCSXCAD_Global.h"

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
	if (prim==NULL)
		return;
	int propID=clCS->GetIndex(prim->GetProperty());
	if (propID<0)
		return;
	QTreeWidgetItem* parent = topLevelItem(propID);
	if (parent==NULL)
		return;
	QString str = QString(prim->GetTypeName().c_str());

	str+=QString(" - ID: %1").arg(prim->GetID());
	QTreeWidgetItem* newPrimItem = new QTreeWidgetItem(parent,QStringList(str),1);
	newPrimItem->setData(0,1,QVariant(prim->GetID()));
	vPrimItems.push_back(newPrimItem);
}

void QCSTreeWidget::AddPropItem(CSProperties* prop)
{
	QString str;
	if (prop==NULL) return;
	str=QString(prop->GetTypeXMLString().c_str())+"::";
	str+=QString::fromUtf8(prop->GetName().c_str());
		
	QTreeWidgetItem* newItem = new QTreeWidgetItem(this,QStringList(str),0);
	newItem->setFont(0,QFont("Arial",10));
	if (prop->GetVisibility()) newItem->setIcon(1,QIcon(":/images/bulb.png"));
	else newItem->setIcon(1,QIcon(":/images/bulb_off.png"));
}
	

QTreeWidgetItem* QCSTreeWidget::GetTreeItemByPrimID(int primID)
{
	for (int n=0;n<vPrimItems.size();++n)
		if (vPrimItems.at(n)->data(0,1).toInt()==primID)
			return vPrimItems.at(n);
	return NULL;
}

int QCSTreeWidget::GetTreeItemIndexByPrimID(int primID)
{
	for (int n=0;n<vPrimItems.size();++n)
		if (vPrimItems.at(n)->data(0,1).toInt()==primID)
			return n;
	return -1;
}

void QCSTreeWidget::DeletePrimItem(CSPrimitives* prim)
{
	int index=GetTreeItemIndexByPrimID(prim->GetID());
	QTreeWidgetItem* item;
	if ((index>=0) && (index<vPrimItems.size()))
		item=vPrimItems.at(index);
	else return;
	vPrimItems.remove(index);
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
	QString str=QString(prop->GetTypeXMLString().c_str())+"::";
	str+=QString::fromUtf8(prop->GetName().c_str());
	item->setText(0,str);	
	if (prop->GetVisibility()) item->setIcon(1,QIcon(":/images/bulb.png"));
	else item->setIcon(1,QIcon(":/images/bulb_off.png"));
}


void QCSTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	if (QCSX_Settings.GetEdit())
	{
		QMenu* primM = menu.addMenu(QIcon(":/images/edit_add.png"),tr("New Primitive"));
		primM->addAction(tr("Box"),this,SIGNAL(NewBox()));
		primM->addAction(tr("Multi-Box"),this,SIGNAL(NewMultiBox()));
		primM->addAction(tr("Sphere"),this,SIGNAL(NewSphere()));
		primM->addAction(tr("Cylinder"),this,SIGNAL(NewCylinder()));
		primM->addAction(tr("User Defined"),this,SIGNAL(NewUserDefined()));

		QMenu* propM = menu.addMenu(QIcon(":/images/edit_add.png"),tr("New Property"));
		propM->addAction(tr("Material"),this,SIGNAL(NewMaterial()));
		propM->addAction(tr("Metal"),this,SIGNAL(NewMetal()));
		propM->addAction(tr("Excitation"),this,SIGNAL(NewExcitation()));
		propM->addAction(tr("Probe-Box"),this,SIGNAL(NewChargeBox()));
		propM->addAction(tr("Res-Box"),this,SIGNAL(NewResBox()));
		propM->addAction(tr("Dump-Box"),this,SIGNAL(NewDumpBox()));

		menu.addSeparator();

		menu.addAction(QIcon(":/images/edit.png"),tr("Edit"),this,SIGNAL(Edit()));
		menu.addAction(QIcon(":/images/editcopy.png"),tr("Copy"),this,SIGNAL(Copy()));
		menu.addAction(QIcon(":/images/edit_remove.png"),tr("Delete"),this,SIGNAL(Delete()));
	}
	else
	{
		menu.addAction(QIcon(":/images/edit.png"),tr("View"),this,SIGNAL(Edit()));
	}

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

	std::vector<CSPrimitives*> vPrims = clCS->GetAllPrimitives();
	for (size_t i=0;i<vPrims.size();++i)
		AddPrimItem(vPrims.at(i));
}

void QCSTreeWidget::ClearTree()
{
	this->clear();
	vPrimItems.clear();
}

void QCSTreeWidget::SwitchProperty(CSPrimitives* prim, CSProperties* newProp)
{
	int index=GetTreeItemIndexByPrimID(prim->GetID());
	QTreeWidgetItem *item=NULL;
	if ((index>=0) && (index<vPrimItems.size()))
		item=vPrimItems.at(index);
	else
		return;

	QTreeWidgetItem *parent=item->parent();
	if (parent==NULL)
		return;
	QTreeWidgetItem *newParent = topLevelItem(clCS->GetIndex(newProp));
	if (newParent==NULL)
		return;

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
	UNUSED(column);
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
