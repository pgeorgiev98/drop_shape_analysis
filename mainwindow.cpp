#include "mainwindow.h"

#include <QTextStream>
#include <QPainter>
#include <QMessageBox>
#include <QLineSeries>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QtMath>
#include <limits>
#include <QComboBox>
#include <QFileDialog>
#include <QDoubleValidator>

#include <QDebug>

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
    , m_chartsLayout(new QHBoxLayout)
    , m_inputprecision(new QLineEdit)
    , m_inputb(new QLineEdit)
    , m_inputc(new QLineEdit)
    , m_modelChart(new QChartView)
    , m_errorChart(new QChartView)
    , m_dropType(new QComboBox)
	, m_theoreticalSeries(new QLineSeries)
	, m_experimentalSeries(new QLineSeries)
    , m_errorSeries(new QLineSeries)
{
    m_dropType->addItems({"Pendant", "Rotating"});
    m_modelChart->setMinimumSize(500, 500);
    m_errorChart->setMinimumSize(500, 500);
    m_inputb->setValidator(new QDoubleValidator(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 1000));
    m_inputc->setValidator(new QDoubleValidator(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 1000));
	QWidget *w = new QWidget;
    w->setLayout(m_chartsLayout);
    setCentralWidget(w);
    m_inputprecision->setPlaceholderText("Default is 0.1");
    m_inputprecision->setValidator(new QDoubleValidator(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 1000));
    m_inputb->setAlignment(Qt::AlignRight);
    m_inputc->setAlignment(Qt::AlignRight);
    m_inputprecision->setAlignment(Qt::AlignRight);

    QPushButton *generateTheoreticalModelButton = new QPushButton("Generate theoretical model");
    QPushButton *loadExperimentalModelButton = new QPushButton("Load experimental model");

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(16);
    int row = 0;

    grid->setRowStretch(row++, 1);

    grid->addWidget(new QLabel("Drop type: "), row, 0);
    grid->addWidget(m_dropType, row, 1);
    ++row;

    grid->addWidget(new QLabel("precision: "), row, 0, Qt::AlignRight);
    grid->addWidget(m_inputprecision, row, 1);
    ++row;

    grid->addWidget(new QLabel("b: "), row, 0, Qt::AlignRight);
    grid->addWidget(m_inputb, row, 1);
    ++row;

    grid->addWidget(new QLabel("c: "), row, 0, Qt::AlignRight);
    grid->addWidget(m_inputc, row, 1);
    ++row;

    grid->addWidget(generateTheoreticalModelButton, row++, 0, 1, 2);
    grid->addWidget(loadExperimentalModelButton, row++, 0, 1, 2);

    grid->setRowStretch(row++, 1);

    m_chartsLayout->addLayout(grid);
    m_chartsLayout->addWidget(m_modelChart, 1);
    m_chartsLayout->addWidget(m_errorChart, 1);

    m_theoreticalSeries->setName("Theoretical model");
    m_experimentalSeries->setName("Experimental model");
    m_modelChart->chart()->addSeries(m_theoreticalSeries);
    m_modelChart->chart()->addSeries(m_experimentalSeries);
    m_modelChart->chart()->createDefaultAxes();
    m_modelChart->setRenderHint(QPainter::Antialiasing);

    m_errorSeries->setName("Error");
    m_errorSeries->setPen(QPen(Qt::red));
    m_errorChart->chart()->addSeries(m_errorSeries);
    m_errorChart->chart()->createDefaultAxes();
    m_errorChart->setRenderHint(QPainter::Antialiasing);

    connect(m_inputb, &QLineEdit::returnPressed, this, &MainWindow::generateTheoreticalModel);
    connect(m_inputc, &QLineEdit::returnPressed, this, &MainWindow::generateTheoreticalModel);

    connect(generateTheoreticalModelButton, &QPushButton::clicked, this, &MainWindow::generateTheoreticalModel);
    connect(loadExperimentalModelButton, &QPushButton::clicked, this, &MainWindow::selectExperimentalModel);
}

static bool expectChar(QTextStream &in, char c)
{
	char ch;
	in >> ch;
	return c == ch;
}

void MainWindow::setSeries(QLineSeries *series, const QVector<QPointF> &points)
{
    QChart *c = series->chart();
    c->removeSeries(series);
    series->clear();
	for (auto p : points)
		series->append(p.x(), p.y());
    c->addSeries(series);
    c->createDefaultAxes();
}

void MainWindow::generateTheoreticalModel()
{
    qDebug() << "Generating model...";
    double b, c;
    b = m_inputb->text().replace(',', '.').toDouble();
    c = m_inputc->text().replace(',', '.').toDouble();
    QVector<QPointF> drop;
    double x0 = 0, z0 = 0, phi0 = 0;
    double x1, z1, phi1;
    bool precisionValid;
    double h = m_inputprecision->text().replace(',', '.').toDouble(&precisionValid);
    if (!precisionValid)
        h = 0.1;
    if(m_dropType->currentIndex() == 0)
    {
        for (;;)
        {
            drop.append(QPointF(x0, z0));
            if (x0 > 1.0)
                break;
            x1 = x0 + h * cos(phi0);
            z1 = z0 + h * sin(phi0);
            if(x0 == 0)
            {
                phi1 = phi0 + h * b;
            }
            else
            {
                phi1 = phi0 + h * (2*b + c*z0 - sin(phi0)/x0);
            }
            x0 = x1;
            z0 = z1;
            phi0 = phi1;
        }
    }
    else
    {
        drop.append(QPointF(x0, z0));
        for (;;)
        {
            x1 = x0 + h * cos(phi0);
            z1 = z0 + h * sin(phi0);
            if(x0 == 0)
            {
                phi1 = phi0 + h * b;
            }
            else
            {
                phi1 = phi0 + h * (2*b + c*x0*x0 - sin(phi0)/x0);
            }
            x0 = x1;
            z0 = z1;
            phi0 = phi1;
            drop.append(QPointF(x0, z0));
            if (x0 <= std::numeric_limits<double>::epsilon())
                break;
        }
    }
    qDebug() << "Done";

    setSeries(m_theoreticalSeries, drop);
    updateErrorSeries();
}

void MainWindow::selectExperimentalModel()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select experimental model");
    if (!fileName.isEmpty())
        setExperimentalModel(fileName);
}

void MainWindow::setExperimentalModel(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", QString("Failed to open file %1: %2").arg(filePath).arg(f.errorString()));
        return;
    }
    QTextStream in(&f);
    QVector<QPointF> points;
    try {
        if (!expectChar(in, '{'))
            throw "";
        while (!in.atEnd()) {
            if (!expectChar(in, '{'))
                throw "";
            double x, y;
            in >> x;
            if (!expectChar(in, ','))
                throw "";
            in >> y;
            if (!expectChar(in, '}'))
                throw "";
            points.append(QPointF(x, y));
            if (!expectChar(in, ','))
                break;
        }
    } catch(...) {
        QMessageBox::critical(this, "Error", "File format error");
        return;
    }

    setSeries(m_experimentalSeries, points);
    updateErrorSeries();
}

void MainWindow::updateErrorSeries()
{
    auto theoretical = m_theoreticalSeries->points();
    auto experimental = m_experimentalSeries->points();

    if (theoretical.size() < 2 || experimental.isEmpty()) {
        setSeries(m_errorSeries, {});
        return;
    }

    QVector<QPointF> error;

    auto t0 = theoretical.begin(), t1 = theoretical.begin() + 1;
    auto e = experimental.begin();

    while (t1 != theoretical.end() && e != experimental.end()) {
        while (e->x() > t1->x()) {
            ++t0;
            ++t1;
        }
        if (e->x() < t0->x())
            continue;
        Q_ASSERT(e->x() <= t1->x() && e->x() >= t0->x());
        double y = t0->y() + (e->x() - t0->x()) * (t1->y() - t0->y()) / (t1->x() - t0->x());
        error.append({e->x(), qAbs(y - e->y())});
        ++e;
    }

    setSeries(m_errorSeries, error);
}
