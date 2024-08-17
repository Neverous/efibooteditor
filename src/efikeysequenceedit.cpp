// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "efikeysequenceedit.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QTimerEvent>

EFIKeySequenceEdit::EFIKeySequenceEdit(QWidget *parent)
    : QWidget{parent}
    , lineEdit{std::make_unique<QLineEdit>(this)}
    , layout{std::make_unique<QVBoxLayout>(this)}
{
    connect(lineEdit.get(), &QLineEdit::textChanged, this, [this](const QString &text)
        {
            // Clear the sequence if the user clicked on the clear icon
            if(text.isEmpty())
                this->clear(); });

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(lineEdit.get());

    lineEdit->setFocusProxy(this);
    lineEdit->installEventFilter(this);
    resetState();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_MacShowFocusRect, true);
    setAttribute(Qt::WA_InputMethodEnabled, false);
}

EFIKeySequenceEdit::EFIKeySequenceEdit(const EFIKeySequence &keySequence, QWidget *parent)
    : EFIKeySequenceEdit{parent}
{
    setKeySequence(keySequence);
}

void EFIKeySequenceEdit::setKeySequence(const EFIKeySequence &keySequence)
{
    resetState();

    if(_keySequence == keySequence)
        return;

    _keySequence = keySequence;
    lineEdit->setText(_keySequence.toString());
    Q_EMIT keySequenceChanged(_keySequence);
}

void EFIKeySequenceEdit::setMaximumSequenceLength(qsizetype count)
{
    _maximumSequenceLength = count;
}

void EFIKeySequenceEdit::keyPressEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat())
        return;

    if(event->key() == Qt::Key_Enter)
        return;

    int key = event->key();
    if(startKey == -1)
    {
        clear();
        startKey = key;
    }

    lineEdit->setPlaceholderText({});

    // Clear selected
    const auto selectedText = lineEdit->selectedText();
    if(!selectedText.isEmpty() && selectedText == lineEdit->text())
    {
        clear();
        if(key == Qt::Key_Backspace)
            return;
    }

    if(!_keySequence.addKey(key, event->text(), _maximumSequenceLength))
        return;

    lineEdit->setText(_keySequence.toString() + "...");
    event->accept();
}

void EFIKeySequenceEdit::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == startKey)
        finishEditing();

    event->accept();
}

void EFIKeySequenceEdit::focusOutEvent(QFocusEvent *event)
{
    if(startKey != -1 && event->reason() != Qt::PopupFocusReason)
        finishEditing();

    return QWidget::focusOutEvent(event);
}

void EFIKeySequenceEdit::resetState()
{
    startKey = -1;
    lineEdit->setText(_keySequence.toString());
    lineEdit->setPlaceholderText(tr("Press hot key"));
}

void EFIKeySequenceEdit::finishEditing()
{
    resetState();
    Q_EMIT keySequenceChanged(_keySequence);
    Q_EMIT editingFinished();
}
