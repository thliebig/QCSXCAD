#ifndef _QCSGRIDEDITOR_H_
#define _QCSGRIDEDITOR_H_

#include <QtGui>

class CSRectGrid;

class QCSGridEditor : public QWidget
{
	Q_OBJECT
public:
	QCSGridEditor(CSRectGrid* grid, QWidget* parent = 0);
	virtual ~QCSGridEditor();

	QToolBar* BuildToolbar();

signals:
	void OpacityChange(int);
	void signalDetectEdges(int);
	void GridChanged();

public slots:
	void Update();
	void SetOpacity(int val);
	void EditX();
	void EditY();
	void EditZ();
	void Edit(int direct);

protected slots:
	void BuildHomogenDisc();
	void BuildInHomogenDisc();
	void IncreaseResolution();
	void DetectEdges();
	void SetDeltaUnit(QString val);

protected:
	CSRectGrid* clGrid;
	QVector<QLabel*> SimBox;
	QVector<QLabel*> NodeQty;
	//QComboBox* UnitLength;
	QLineEdit* UnitLength;
	QSlider* OpacitySlider;

	double* GetDoubleArrayFromString(int *count, QString qsValue);
};

#endif //_QCSGRIDEDITOR_H_
