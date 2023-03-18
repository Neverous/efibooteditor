// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QRadioButton>

class QIndicatorWidget: public QRadioButton
{
    Q_OBJECT
public:
    explicit QIndicatorWidget(QWidget *parent = nullptr)
        : QRadioButton{parent}
    {
        // setAttribute(Qt::WA_TransparentForMouseEvents);
        setStyleSheet(
            "::indicator:unchecked {background-color: red; border-radius: 7px;}\n"
            "::indicator:checked {background-color: green; border-radius: 7px;}");
        setLayoutDirection(Qt::RightToLeft);
        setFocusPolicy(Qt::NoFocus);
        setEnabled(false);
    }

    QIndicatorWidget(const QIndicatorWidget &) = delete;
    QIndicatorWidget &operator=(const QIndicatorWidget &) = delete;
};
