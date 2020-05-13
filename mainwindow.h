#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dropgenerator.h"

#include <QMainWindow>
#include <QChartView>
#include <QLineEdit>
#include <QComboBox>
#include <QLineSeries>

class QHBoxLayout;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void setSeries(QtCharts::QLineSeries *series, const QVector<QPointF> &points);
    void visualiseTheoreticalModel();
    void visualiseClosestTheoreticalModel();
    void selectExperimentalModel();
    void setExperimentalModel(const QString &filePath);
    void setBestTheoreticalModel(DropGenerator::TheoreticalModelParameters parameters);
    void updateErrorSeries();
    void selectImage();
    void updateCutoffMoment();

private:
    QHBoxLayout *m_chartsLayout;
    QLineEdit *m_inputprecision, *m_inputb, *m_inputc;
    QLabel *m_modelErrorLabel, *m_iterationsCountLabel;
    QtCharts::QChartView *m_modelChart, *m_errorChart;
    QComboBox *m_dropType;
    QtCharts::QLineSeries *m_theoreticalSeries;
    QtCharts::QLineSeries *m_experimentalSeries;
    QtCharts::QLineSeries *m_errorSeries;
    QVector<QPointF> m_currentTheoreticalModel, m_currentExperimentalModel;
    int m_cutoffMoment;
};
#endif // MAINWINDOW_H
