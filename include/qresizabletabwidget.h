// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QStyle>
#include <QStyleOption>
#include <QTabBar>
#include <QTabWidget>

class QResizableTabWidget: public QTabWidget
{
    Q_OBJECT
public:
    explicit QResizableTabWidget(QWidget *parent = nullptr)
        : QTabWidget{parent}
    {
    }

    QResizableTabWidget(const QResizableTabWidget &) = delete;
    QResizableTabWidget &operator=(const QResizableTabWidget &) = delete;

    // Extracted from: qtbase:src/widgets/widgets/qtabwidget.cpp
    // and replaced expanding to all tabs widgets just to current one
    QSize sizeHint() const override
    {
        QSize lc(0, 0), rc(0, 0);
        if(cornerWidget(Qt::TopLeftCorner))
            lc = cornerWidget(Qt::TopLeftCorner)->minimumSizeHint();
        if(cornerWidget(Qt::TopRightCorner))
            rc = cornerWidget(Qt::TopRightCorner)->minimumSizeHint();

        QSize s(currentWidget()->sizeHint());
        QSize t(tabBar()->sizeHint());
        QSize sz = basicSize(tabPosition() == North || tabPosition() == South, lc, rc, s, t);

        QStyleOptionTabWidgetFrame opt;
        initStyleOption(&opt);
        opt.palette = palette();
        opt.state = QStyle::State_None;
        return style()->sizeFromContents(QStyle::CT_TabWidget, &opt, sz, this);
    }

    QSize minimumSizeHint() const override
    {
        QSize lc(0, 0), rc(0, 0);
        if(cornerWidget(Qt::TopLeftCorner))
            lc = cornerWidget(Qt::TopLeftCorner)->minimumSizeHint();
        if(cornerWidget(Qt::TopRightCorner))
            rc = cornerWidget(Qt::TopRightCorner)->minimumSizeHint();

        QSize s(currentWidget()->minimumSizeHint());
        QSize t(tabBar()->minimumSizeHint());
        QSize sz = basicSize(tabPosition() == North || tabPosition() == South, lc, rc, s, t);

        QStyleOptionTabWidgetFrame opt;
        initStyleOption(&opt);
        opt.palette = palette();
        opt.state = QStyle::State_None;
        return style()->sizeFromContents(QStyle::CT_TabWidget, &opt, sz, this);
    }

private:
    static inline QSize basicSize(
        bool horizontal, const QSize &lc, const QSize &rc, const QSize &s, const QSize &t)
    {
        return horizontal
            ? QSize(qMax(s.width(), t.width() + rc.width() + lc.width()),
                s.height() + (qMax(rc.height(), qMax(lc.height(), t.height()))))
            : QSize(s.width() + (qMax(rc.width(), qMax(lc.width(), t.width()))),
                qMax(s.height(), t.height() + rc.height() + lc.height()));
    }
};
