#ifndef _QPARAMETERGUI_H_
#define _QPARAMETERGUI_H_

#include <QtGui>
#include "ParameterObjects.h"

class QParameter : public QGroupBox
{
	Q_OBJECT
public:
	QParameter(Parameter* para, QWidget* parent=NULL);
	virtual ~QParameter();
	
	Parameter* GetParameter() {return clPara;};
	
signals:
 	void ParameterChanged();
 	void Delete(Parameter* para);
	
public slots:
	virtual bool Edit();
	virtual void Update();
	void DeleteParameter();
	void SweepState(int state);

protected slots:
	virtual void Changed();
		
protected:
	Parameter* clPara;
	QLabel* Name;
	QLineEdit* Value;
	QGridLayout *lay;
	QCheckBox* SweepCB;
};

class QLinearParameter : public QParameter
{
	Q_OBJECT
public:
	QLinearParameter(LinearParameter* para, QWidget* parent=NULL);
	virtual ~QLinearParameter();
 		
signals:

public slots:
	virtual bool Edit();
	virtual void Update();

protected slots:
	virtual void Changed();
 		
protected:
	QSlider* slider;
};

class QParameterSet : public QWidget, public ParameterSet
{
	Q_OBJECT
public:
	QParameterSet(QWidget* parent=NULL);
	virtual ~QParameterSet();

	virtual size_t LinkParameter(Parameter* newPara);

signals:
 	void ParameterChanged();
 	
public slots:
 	void NewParameter();
	virtual void SetModified(bool mod=true);
	
	virtual void clear();
 
protected slots:
	virtual size_t DeleteParameter(Parameter* para); 		
	
protected:
	void AddParaWid(Parameter* newPara);
	QVBoxLayout* ParaLay;
	QVector<QParameter*> vecQPara;
};

#endif //_QPARAMETERGUI_H_
