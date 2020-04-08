#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>

class QHBoxLayout;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

public slots:
	//void addFile(const QString &file);
	void plot(QVector<QVector<QPointF>> things);

protected:
	//void paintEvent(QPaintEvent *) override;

private:
	QtCharts::QChartView *m_chart1;
	QtCharts::QChartView *m_chart2;
	QHBoxLayout *m_layout;
};
#endif // MAINWINDOW_H
