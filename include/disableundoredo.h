// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QEvent>
#include <QKeyEvent>
#include <QObject>

class DisableUndoRedo: public QObject
{
    Q_OBJECT

public:
    explicit DisableUndoRedo(QObject *parent = nullptr)
        : QObject{parent}
    {
    }

    DisableUndoRedo(const DisableUndoRedo &) = delete;
    DisableUndoRedo &operator=(const DisableUndoRedo &) = delete;

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override
    {
        if(ev->type() == QEvent::ShortcutOverride)
        {
            auto keyEvent = dynamic_cast<QKeyEvent *>(ev);
            auto isUndo = keyEvent->matches(QKeySequence::Undo);
            auto isRedo = keyEvent->matches(QKeySequence::Redo);

            return isUndo || isRedo;
        }

        return QObject::eventFilter(obj, ev);
    }
};
