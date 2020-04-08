#include "mainwindow.h"

#include <QTextStream>
#include <QPainter>
#include <QMessageBox>
#include <QLineSeries>
#include <QHBoxLayout>

#include <QDebug>

using namespace QtCharts;

static QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta};

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_layout(new QHBoxLayout)
{
	setFixedSize(500, 500);
	QWidget *w = new QWidget;
	w->setLayout(m_layout);
	setCentralWidget(w);
}

static bool expectChar(QTextStream &in, char c)
{
	char ch;
	in >> ch;
	return c == ch;
}

/*
void MainWindow::addFile(const QString &file)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this, "Error", QString("Failed to open file %1: %2").arg(file).arg(f.errorString()));
		return;
	}
	QTextStream in(&f);
	Drop drop;
	try {
		while (!in.atEnd()) {
			if (!expectChar(in, '{'))
				throw;
			double x, y;
			in >> x;
			if (!expectChar(in, ','))
				throw;
			in >> y;
			if (!expectChar(in, '}'))
				throw;
			drop.m_points.append(QPointF(x, y));
			if (y > m_height)
				m_height = y;
			if (!expectChar(in, ','))
				break;
		}
	} catch(...) {
		QMessageBox::critical(this, "Error", "File format error");
		return;
	}

	m_drops.append(drop);
	update();
}
*/

void MainWindow::plot(QVector<QVector<QPointF>> things)
{
	QChartView *view = new QChartView;
	m_layout->addWidget(view);
	for (auto points : things) {
		QLineSeries *series = new QLineSeries(this);

		for (auto p : points)
			series->append(p.x(), p.y());

		view->chart()->addSeries(series);
	}
	//view->chart()->legend()->hide();
	view->chart()->createDefaultAxes();
	view->setRenderHint(QPainter::Antialiasing);
}

/*
void MainWindow::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	double width = m_maxX - m_minX;
	double height = m_maxY - m_minY;
	p.translate(0, this->height());
	p.scale(1, -1);
	double scaleX = this->height() / height;
	double scaleY = this->width() / width;
	for (int d = 0; d < m_drops.size(); ++d) {
		p.setPen(QPen(colors[d], 3));
		const Drop &drop = m_drops[d];
		for (int i = 1; i < drop.m_points.size(); ++i) {
			QPointF p1 = drop.m_points[i - 1] - QPointF(m_minX, m_minY);
			QPointF p2 = drop.m_points[i] - QPointF(m_minX, m_minY);
			p.drawLine(QLineF(p1 * scaleX, p2 * scaleY));
			qDebug() << QLineF(p1 * scaleX, p2 * scaleY);
		}
	}
}
*/
