/* -*- c++ -*- */
/*
 * Copyright 2008-2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef CONSTELLATION_DISPLAY_PLOT_C
#define CONSTELLATION_DISPLAY_PLOT_C

#include <gnuradio/qtgui/ConstellationDisplayPlot.h>

#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <QColor>
#include <cmath>

class ConstellationDisplayZoomer : public QwtPlotZoomer
{
public:
    ConstellationDisplayZoomer(QWidget* canvas) : QwtPlotZoomer(canvas)
    {
        setTrackerMode(QwtPicker::AlwaysOn);
    }

    ~ConstellationDisplayZoomer() override {}

    virtual void updateTrackerText() { updateDisplay(); }

protected:
    using QwtPlotZoomer::trackerText;
    QwtText trackerText(const QPoint& p) const override
    {
        QPointF dp = QwtPlotZoomer::invTransform(p);
        QwtText t(QString("(%1, %2)").arg(dp.x(), 0, 'f', 4).arg(dp.y(), 0, 'f', 4));
        return t;
    }
};

ConstellationDisplayPlot::ConstellationDisplayPlot(int nplots, QWidget* parent)
    : DisplayPlot(nplots, parent)
{
    resize(parent->width(), parent->height());

    d_numPoints = 1024;
    d_pen_size = 5;

    d_zoomer = new ConstellationDisplayZoomer(canvas());

    d_zoomer->setMousePattern(
        QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    d_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    const QColor c(Qt::darkRed);
    d_zoomer->setRubberBandPen(c);
    d_zoomer->setTrackerPen(c);

    d_magnifier->setAxisEnabled(QwtPlot::xBottom, true);
    d_magnifier->setAxisEnabled(QwtPlot::yLeft, true);

    setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
    set_xaxis(-2.0, 2.0);
    setAxisTitle(QwtPlot::xBottom, "In-phase");

    setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
    set_yaxis(-2.0, 2.0);
    setAxisTitle(QwtPlot::yLeft, "Quadrature");
    updateAxes();

    QList<QColor> colors;
    colors << QColor(Qt::blue) << QColor(Qt::red) << QColor(Qt::green)
           << QColor(Qt::black) << QColor(Qt::cyan) << QColor(Qt::magenta)
           << QColor(Qt::yellow) << QColor(Qt::gray) << QColor(Qt::darkRed)
           << QColor(Qt::darkGreen) << QColor(Qt::darkBlue) << QColor(Qt::darkGray);

    // Setup dataPoints and plot vectors
    // Automatically deleted when parent is deleted
    for (unsigned int i = 0; i < d_nplots; ++i) {
        d_real_data.emplace_back(d_numPoints);
        d_imag_data.emplace_back(d_numPoints);

        d_plot_curve.push_back(new QwtPlotCurve(QString("Data %1").arg(i)));
        d_plot_curve[i]->attach(this);
        d_plot_curve[i]->setPen(QPen(colors[i]));

        QwtSymbol* symbol = new QwtSymbol(
            QwtSymbol::NoSymbol, QBrush(colors[i]), QPen(colors[i]), QSize(7, 7));

        d_plot_curve[i]->setRawSamples(
            d_real_data[i].data(), d_imag_data[i].data(), d_numPoints);
        d_plot_curve[i]->setSymbol(symbol);

        setLineStyle(i, Qt::NoPen);
        setLineMarker(i, QwtSymbol::Ellipse);
    }
}

ConstellationDisplayPlot::~ConstellationDisplayPlot()
{
    // d_plot_curves deleted when parent deleted
    // d_zoomer and d_panner deleted when parent deleted
}

void ConstellationDisplayPlot::set_xaxis(double min, double max) { setXaxis(min, max); }

void ConstellationDisplayPlot::set_yaxis(double min, double max) { setYaxis(min, max); }

void ConstellationDisplayPlot::set_axis(double xmin,
                                        double xmax,
                                        double ymin,
                                        double ymax)
{
    set_xaxis(xmin, xmax);
    set_yaxis(ymin, ymax);
}

void ConstellationDisplayPlot::set_pen_size(int size)
{
    if (size > 0 && size < 30) {
        d_pen_size = size;
        for (unsigned int i = 0; i < d_nplots; ++i) {
            d_plot_curve[i]->setPen(
                QPen(Qt::blue, d_pen_size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }
    }
}

void ConstellationDisplayPlot::replot() { QwtPlot::replot(); }


void ConstellationDisplayPlot::plotNewData(
    const std::vector<const double*> realDataPoints,
    const std::vector<const double*> imagDataPoints,
    const int64_t numDataPoints,
    const double timeInterval)
{
    if (!d_stop) {
        if ((numDataPoints > 0)) {
            if (numDataPoints != d_numPoints) {
                d_numPoints = numDataPoints;

                for (unsigned int i = 0; i < d_nplots; ++i) {
                    d_real_data[i].resize(d_numPoints);
                    d_imag_data[i].resize(d_numPoints);

                    d_plot_curve[i]->setRawSamples(
                        d_real_data[i].data(), d_imag_data[i].data(), d_numPoints);
                }
            }

            for (unsigned int i = 0; i < d_nplots; ++i) {
                memcpy(d_real_data[i].data(),
                       realDataPoints[i],
                       numDataPoints * sizeof(double));
                memcpy(d_imag_data[i].data(),
                       imagDataPoints[i],
                       numDataPoints * sizeof(double));
            }

            if (d_autoscale_state) {
                double bottom = 1e20, top = -1e20;
                for (unsigned int n = 0; n < d_nplots; ++n) {
                    for (int64_t point = 0; point < numDataPoints; point++) {
                        double b =
                            std::min(realDataPoints[n][point], imagDataPoints[n][point]);
                        double t =
                            std::max(realDataPoints[n][point], imagDataPoints[n][point]);
                        if (b < bottom) {
                            bottom = b;
                        }
                        if (t > top) {
                            top = t;
                        }
                    }
                }
                _autoScale(bottom, top);
            }

            replot();
        }
    }
}

void ConstellationDisplayPlot::plotNewData(const double* realDataPoints,
                                           const double* imagDataPoints,
                                           const int64_t numDataPoints,
                                           const double timeInterval)
{
    std::vector<const double*> vecRealDataPoints;
    std::vector<const double*> vecImagDataPoints;
    vecRealDataPoints.push_back(realDataPoints);
    vecImagDataPoints.push_back(imagDataPoints);
    plotNewData(vecRealDataPoints, vecImagDataPoints, numDataPoints, timeInterval);
}

void ConstellationDisplayPlot::_autoScale(double bottom, double top)
{
    // Auto scale the x- and y-axis with a margin of 20%
    double b = bottom - fabs(bottom) * 0.20;
    double t = top + fabs(top) * 0.20;
    set_axis(b, t, b, t);
}

void ConstellationDisplayPlot::setAutoScale(bool state) { d_autoscale_state = state; }

#endif /* CONSTELLATION_DISPLAY_PLOT_C */
