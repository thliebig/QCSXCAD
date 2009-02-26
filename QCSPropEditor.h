#ifndef _QCSPROPEDITOR_H_
#define _QCSPROPEDITOR_H_

#include <QtGui>
#include "ContinuousStructure.h"

class QColorPushButton : public QPushButton
{
	Q_OBJECT
public:
	QColorPushButton(const QString & text, QWidget * parent = 0 );
	~QColorPushButton();

	void SetColor(RGBa c);
	RGBa GetColor();

protected slots:
	void ChangeColor();

protected:
	QColor Color;
};

class QCSPropertyGroupBox;
class QCSPropMaterialGB;

class QCSPropEditor : public QDialog
{
	Q_OBJECT
public:
	QCSPropEditor(ContinuousStructure *CS, CSProperties* prop, int iSimMode, QWidget* parent=NULL);
	~QCSPropEditor() {};

protected slots:
	void Reset();
	void Save();
	void Cancel();

	void ChangeType(int item);

	void Fill2EdgeColor();
	void Edge2FillColor();

	void GetValues();
	void SetValues();

protected:
	QComboBox* TypeCB;

	QGroupBox* BuildGeneral();
	QLayout* BuildButtons();

	QVBoxLayout* MainLayout;
	//QCSPropertyLayout* CSPropEdit;
	ContinuousStructure *clCS;

	QLineEdit* Name;
	QLabel* TypeLbl;

	QColorPushButton* FCButton;
	QColorPushButton* ECButton;

	CSProperties* clProp;
	CSProperties* saveProp;

	QCSPropertyGroupBox* BuildPropGroupBox(CSProperties* clProp);
	QCSPropertyGroupBox* propGB;

	int GetIndexOfType();

	int m_SimMode;
};

//dummy class
class QCSPropertyGroupBox : public QGroupBox
{
	Q_OBJECT
public:
	virtual ~QCSPropertyGroupBox();

protected:
	QCSPropertyGroupBox(QWidget *parent=NULL);

	QString TypeName;
	QString GetTypeName() {return TypeName;};

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

};

class QCSPropUnknownGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropUnknownGB(CSPropUnknown *prop, QWidget *parent=NULL);
	virtual ~QCSPropUnknownGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropUnknown* clProp;

	QLineEdit* Property;
};


class QCSPropMaterialGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropMaterialGB(CSPropMaterial *prop, QWidget *parent=NULL);
	virtual ~QCSPropMaterialGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropMaterial* clProp;

	QLineEdit* EpsLine;
	QLineEdit* MueLine;
	QLineEdit* KappaLine;
};

class QCSPropElectrodeGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropElectrodeGB(CSPropElectrode *prop, QWidget *parent=NULL);
	virtual ~QCSPropElectrodeGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

protected slots:
	void TypeChanged(int index);

signals:
	void modified();

protected:
	CSPropElectrode* clProp;

	QLineEdit* FctLine[3];

	QSpinBox* Number;
	QComboBox* Type;

	QLineEdit* Excitation[3];
};

class QCSPropChargeBoxGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropChargeBoxGB(CSPropChargeBox *prop, QWidget *parent=NULL);
	virtual ~QCSPropChargeBoxGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropChargeBox* clProp;

	QSpinBox* Number;
};

class QCSPropResBoxGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropResBoxGB(CSPropResBox *prop, QWidget *parent=NULL);
	virtual ~QCSPropResBoxGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropResBox* clProp;

	QSpinBox* Factor;
};

class QCSPropDumpBoxGB : public QCSPropertyGroupBox
{
	Q_OBJECT
public:
	QCSPropDumpBoxGB(CSPropDumpBox *prop, int SimMode, QWidget *parent=NULL);
	virtual ~QCSPropDumpBoxGB();

public slots:
	virtual void SetValues();
	virtual void GetValues();

signals:
	void modified();

protected:
	CSPropDumpBox* clProp;

	QCheckBox* GlobSet;

	QCheckBox* DumpPhi;
	QCheckBox* DumpDivE;
	QCheckBox* DumpDivD;
	QCheckBox* DumpDivP;
	QCheckBox* DumpFieldW;
	QCheckBox* DumpChargeW;

	QCheckBox* DumpEField;
	QCheckBox* DumpDField;
	QCheckBox* DumpPField;

	QCheckBox* DumpSubGrid;
	QSpinBox* SGLevel;
	QCheckBox* SimpleGrid;

	QLineEdit* DumpPath;
};


#endif //_QCSPROPEDITOR_H_
