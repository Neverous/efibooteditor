// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QLabel>

class QLabelWrapped: public QLabel
{
    Q_OBJECT
public:
    explicit QLabelWrapped(QWidget *parent = nullptr)
        : QLabel{parent}
    {
        setWordWrap(true);
    }

    QLabelWrapped(const QLabelWrapped &) = delete;
    QLabelWrapped &operator=(const QLabelWrapped &) = delete;

    QSize sizeHint() const override
    {
        auto w = width();
        auto h = heightForWidth(w);
        return {w, h};
    }
};
