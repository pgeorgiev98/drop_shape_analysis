#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>
#include <QLineEdit>
#include <QComboBox>
#include <QLineSeries>

class QHBoxLayout;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

public slots:
	void setSeries(QtCharts::QLineSeries *series, const QVector<QPointF> &points);
    void generateTheoreticalModel();
	void selectExperimentalModel();
	void setExperimentalModel(const QString &filePath);

private:
    QHBoxLayout *m_chartsLayout;
    QLineEdit *m_inputb, *m_inputc;
    QtCharts::QChartView *m_chart;
    QComboBox *m_dropType;
	QtCharts::QLineSeries *m_theoreticalSeries;
	QtCharts::QLineSeries *m_experimentalSeries;
};
#endif // MAINWINDOW_H
