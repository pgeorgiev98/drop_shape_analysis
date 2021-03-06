#include "mainwindow.h"
#include "dropgenerator.h"
#include "gradientdescent.h"

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
#include <QGroupBox>
#include <QDoubleValidator>
#include <QSettings>
#include <QList>
#include <QTimer>
#include <QDialog>
#include <QProgressBar>
#include <QThread>
#include <QDebug>

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_chartsLayout(new QHBoxLayout)
    , m_inputprecision(new QLineEdit)
    , m_inputb(new QLineEdit)
    , m_inputc(new QLineEdit)
    , m_modelErrorLabel(new QLabel)
    , m_iterationsCountLabel(new QLabel)
    , m_modelChart(new QChartView)
    , m_errorChart(new QChartView)
    , m_dropType(new QComboBox)
    , m_theoreticalSeries(new QLineSeries)
    , m_experimentalSeries(new QLineSeries)
    , m_errorSeries(new QLineSeries)
    , m_cutoffMoment(0)
{
    const double minDouble = std::numeric_limits<double>::lowest();
    const double maxDouble = std::numeric_limits<double>::max();
    m_dropType->addItems({"Pendant", "Rotating"});
    m_modelChart->setMinimumSize(500, 500);
    m_errorChart->setMinimumSize(500, 500);
    m_inputb->setValidator(new QDoubleValidator(minDouble, maxDouble, 1000));
    m_inputc->setValidator(new QDoubleValidator(minDouble, maxDouble, 1000));
    QWidget *w = new QWidget;
    w->setLayout(m_chartsLayout);
    setCentralWidget(w);
    m_inputprecision->setPlaceholderText("Default is 0.1");
    m_inputprecision->setValidator(new QDoubleValidator(minDouble, maxDouble, 1000));
    m_inputb->setAlignment(Qt::AlignRight);
    m_inputc->setAlignment(Qt::AlignRight);
    m_inputprecision->setAlignment(Qt::AlignRight);
    m_modelErrorLabel->setAlignment(Qt::AlignHCenter);
    m_iterationsCountLabel->setAlignment(Qt::AlignHCenter);

    QPushButton *visualiseTheoreticalModelButton = new QPushButton("Visualise theoretical model");
    QPushButton *loadExperimentalModelButton = new QPushButton("Load experimental model");
    QPushButton *loadModelFromImageButton = new QPushButton("Load model from image");
    QPushButton *generateClosestModelButton = new QPushButton("Generate closest theoretical model");

    QGroupBox *calculationSettingsBox = new QGroupBox("Calculation settings");
    QGroupBox *theoreticalModelBox = new QGroupBox("Theoretical model");

    {
        QGridLayout *l = new QGridLayout;
        calculationSettingsBox->setLayout(l);

        int row = 0;
        l->addWidget(new QLabel("Drop type: "), row, 0);
        l->addWidget(m_dropType, row, 1);
        ++row;

        l->addWidget(new QLabel("Precision: "), row, 0, Qt::AlignRight);
        l->addWidget(m_inputprecision, row, 1);
        ++row;
    }

    {
        QGridLayout *l = new QGridLayout;
        theoreticalModelBox->setLayout(l);

        int row = 0;
        l->addWidget(new QLabel("b: "), row, 0, Qt::AlignRight);
        l->addWidget(m_inputb, row, 1);
        ++row;

        l->addWidget(new QLabel("c: "), row, 0, Qt::AlignRight);
        l->addWidget(m_inputc, row, 1);
        ++row;

        l->addWidget(visualiseTheoreticalModelButton, row++, 0, 1, 2);
    }

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(16);
    int row = 0;

    grid->setRowStretch(row++, 1);

    grid->addWidget(calculationSettingsBox, row++, 0, 1, 2);
    grid->addWidget(theoreticalModelBox, row++, 0, 1, 2);

    grid->addWidget(loadExperimentalModelButton, row++, 0, 1, 2);
    grid->addWidget(loadModelFromImageButton, row++, 0, 1, 2);
    grid->addWidget(generateClosestModelButton, row++, 0, 1, 2);

    grid->setRowStretch(row++, 1);

    grid->addWidget(m_iterationsCountLabel, row++, 0, 1, 2);
    grid->addWidget(m_modelErrorLabel, row++, 0, 1, 2);

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

    connect(m_inputprecision, &QLineEdit::returnPressed, this, &MainWindow::visualiseTheoreticalModel);
    connect(m_inputb, &QLineEdit::returnPressed, this, &MainWindow::visualiseTheoreticalModel);
    connect(m_inputc, &QLineEdit::returnPressed, this, &MainWindow::visualiseTheoreticalModel);

    connect(visualiseTheoreticalModelButton, &QPushButton::clicked, this, &MainWindow::visualiseTheoreticalModel);
    connect(loadExperimentalModelButton, &QPushButton::clicked, this, &MainWindow::selectExperimentalModel);
    connect(loadModelFromImageButton, &QPushButton::clicked, this, &MainWindow::selectImage);
    connect(generateClosestModelButton, &QPushButton::clicked, this, &MainWindow::visualiseClosestTheoreticalModel);
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
    QList<QAbstractAxis*> axesX= c->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axesY= c->axes(Qt::Vertical);

    double maxX = 0.0;
    double maxY = 0.0;
    for (QAbstractSeries *series : c->series()) {
        QXYSeries *s = static_cast<QXYSeries *>(series);
        const auto &points = s->points();
        for (auto p : points) {
            if (p.x() > maxX)
                maxX = p.x();
            if (p.y() > maxY)
                maxY = p.y();
        }
    }

    if (maxX == 0.0)
        maxX = 1.0;
    if (maxY == 0.0)
        maxY = 1.0;

    for(auto axisX : axesX)
        axisX->setRange(0, maxX);
    for(auto axisY : axesY)
        axisY->setRange(0, maxY);
}

void MainWindow::visualiseTheoreticalModel()
{
    double b, c;
    b = m_inputb->text().replace(',', '.').toDouble();
    c = m_inputc->text().replace(',', '.').toDouble();
    bool precisionValid;
    double h = m_inputprecision->text().replace(',', '.').toDouble(&precisionValid);
    if (!precisionValid)
        h = 0.1;

    auto dropType = m_dropType->currentIndex() == 0 ? TheoreticalModelParameters::PENDANT : TheoreticalModelParameters::SPINNING;

    TheoreticalModelParameters params(dropType, b, c, h, m_cutoffMoment);
    auto drop = DropGenerator::generateTheoreticalModel(params);
    if(drop.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Maximum number of iterations reached");
        m_iterationsCountLabel->setText(QString());
        return;
    }
    m_iterationsCountLabel->setText(QString("Iterations: %1").arg(drop.size() - 1));
    setSeries(m_theoreticalSeries, drop);
    m_currentTheoreticalModel = drop;
    updateErrorSeries();
}

void MainWindow::visualiseClosestTheoreticalModel()
{
    if (m_currentExperimentalModel.isEmpty()) {
        QMessageBox::information(this, "Select experimental model", "An experimental model must be loaded first");
        return;
    }

    const auto dropType = m_dropType->currentIndex() == 0 ? TheoreticalModelParameters::PENDANT : TheoreticalModelParameters::SPINNING;
    double precision;
    {
        bool precisionValid;
        precision = m_inputprecision->text().replace(',', '.').toDouble(&precisionValid);
        if (!precisionValid)
            precision = 0.1;
    }

    QDialog waitingDialog(this);
    QVBoxLayout *l = new QVBoxLayout;
    QProgressBar *progressBar = new QProgressBar;
    progressBar->setRange(0, 1000);
    l->addWidget(new QLabel("Minimizing error..."), 0, Qt::AlignHCenter);
    l->addWidget(progressBar);
    waitingDialog.setLayout(l);

    QThread *workerThread = new QThread(this);
    GradientDescent *worker = new GradientDescent;
    worker->moveToThread(workerThread);

    connect(worker, &GradientDescent::progressChanged, progressBar, [progressBar](double progress) {
        progressBar->setValue(int(1000 * progress));
    }, Qt::QueuedConnection);

    QTimer singleShotTimer;
    singleShotTimer.setSingleShot(true);
    QObject::connect(
                &singleShotTimer,
                &QTimer::timeout,
                worker,
                [worker, this, dropType, precision]() {
        worker->doWork(m_currentExperimentalModel, dropType, precision, m_cutoffMoment);
    }, Qt::QueuedConnection);
    workerThread->start();
    singleShotTimer.start(0);

    TheoreticalModelParameters bestParameters;
    connect(worker, &GradientDescent::finished, this, &MainWindow::setBestTheoreticalModel, Qt::QueuedConnection);
    connect(worker, &GradientDescent::finished, &waitingDialog, &QDialog::accept, Qt::QueuedConnection);

    waitingDialog.exec();

    workerThread->quit();
    workerThread->wait();
    worker->deleteLater();
    workerThread->deleteLater();
}

void MainWindow::selectExperimentalModel()
{
    QSettings settings;
    static const char *dataDirKey = "experimental-data-dir";
    QString dir = settings.value(dataDirKey, QString()).toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Select experimental model", dir);
    if (!fileName.isEmpty()) {
        settings.setValue(dataDirKey, QFileInfo(fileName).dir().path());
        setExperimentalModel(fileName);
    }
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
    m_currentExperimentalModel = points;
    updateCutoffMoment();
    updateErrorSeries();
}

void MainWindow::setBestTheoreticalModel(TheoreticalModelParameters parameters)
{
    m_dropType->setCurrentIndex(parameters.dropType == TheoreticalModelParameters::PENDANT ? 0 : 1);
    m_inputb->setText(QString::number(parameters.b));
    m_inputc->setText(QString::number(parameters.c));
    m_inputprecision->setText(QString::number(parameters.precision));
    visualiseTheoreticalModel();
}

void MainWindow::updateErrorSeries()
{
    auto theoretical = m_currentTheoreticalModel;
    auto experimental = m_currentExperimentalModel;

    auto error = DropGenerator::generateError(theoretical, experimental);
    setSeries(m_errorSeries, error);
    if (!error.isEmpty()) {
        double errorValue = DropGenerator::calculateError(error);
        m_modelErrorLabel->setText(QString("Squared error: %1").arg(errorValue));
    } else {
        m_modelErrorLabel->setText(QString());
    }
}

void MainWindow::selectImage()
{
    QSettings settings;
    static const char *imageDirKey = "image-dir";
    QString dir = settings.value(imageDirKey, QString()).toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Select image", dir);
    if (!fileName.isEmpty()) {
        settings.setValue(imageDirKey, QFileInfo(fileName).dir().path());
        auto drop = DropGenerator::generateModelFromImage(fileName);
        setSeries(m_experimentalSeries, drop);
        m_currentExperimentalModel = drop;
        updateCutoffMoment();
        updateErrorSeries();
    }
}

void MainWindow::updateCutoffMoment()
{
    m_cutoffMoment = 0;
    for (const QPointF &p : m_currentExperimentalModel) {
        if (p.x() > 1.15) { // Magic number
            m_cutoffMoment = 1;
            break;
        }
    }
}
