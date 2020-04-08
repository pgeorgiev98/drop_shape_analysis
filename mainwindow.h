#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>
#include <QDoubleSpinBox>

class QHBoxLayout;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

public slots:
	//void addFile(const QString &file);
	void plot(QVector<QVector<QPointF>> things);
    void onInputButtonClicked();

protected:
	//void paintEvent(QPaintEvent *) override;

private:
    QHBoxLayout *m_chartsLayout;
    QDoubleSpinBox *m_inputb, *m_inputd;
    QtCharts::QChartView *m_chart;
};
#endif // MAINWINDOW_H
