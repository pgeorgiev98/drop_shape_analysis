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
    void visualiseTheoreticalModel();
    void selectExperimentalModel();
    void setExperimentalModel(const QString &filePath);
    void updateErrorSeries();

private:
    QHBoxLayout *m_chartsLayout;
    QLineEdit *m_inputprecision, *m_inputb, *m_inputc;
    QtCharts::QChartView *m_modelChart, *m_errorChart;
    QComboBox *m_dropType;
    QtCharts::QLineSeries *m_theoreticalSeries;
    QtCharts::QLineSeries *m_experimentalSeries;
    QtCharts::QLineSeries *m_errorSeries;
    QVector<QPointF> m_currentTheoreticalModel, m_currentExperimentalModel;

    enum class DropType {PENDANT, SPINNING};
    static QVector<QPointF> generateTheoreticalModel(double b, double c, DropType type, double precision);
    static QVector<QPointF> generateError(const QVector<QPointF> &theoretical, const QVector<QPointF> &experimental);

};
#endif // MAINWINDOW_H
