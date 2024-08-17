// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "efikeysequence.h"
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

// Based on QKeySequenceEdit
class EFIKeySequenceEdit: public QWidget
{
    Q_OBJECT

    Q_PROPERTY(EFIKeySequence keySequence READ keySequence WRITE setKeySequence NOTIFY keySequenceChanged USER true)
    Q_PROPERTY(bool clearButtonEnabled READ isClearButtonEnabled WRITE setClearButtonEnabled)
    Q_PROPERTY(qsizetype maximumSequenceLength READ maximumSequenceLength WRITE setMaximumSequenceLength)

private:
    std::unique_ptr<QLineEdit> lineEdit;
    std::unique_ptr<QVBoxLayout> layout;
    EFIKeySequence _keySequence{};
    qsizetype _maximumSequenceLength{3};
    int startKey{-1};

public:
    explicit EFIKeySequenceEdit(QWidget *parent = nullptr);
    explicit EFIKeySequenceEdit(const EFIKeySequence &keySequence, QWidget *parent = nullptr);
    EFIKeySequenceEdit(const EFIKeySequenceEdit &) = delete;
    EFIKeySequenceEdit &operator=(const EFIKeySequenceEdit &) = delete;

    const EFIKeySequence &keySequence() const { return _keySequence; }
    qsizetype maximumSequenceLength() const { return _maximumSequenceLength; }

    void setClearButtonEnabled(bool enable) { lineEdit->setClearButtonEnabled(enable); }
    bool isClearButtonEnabled() const { return lineEdit->isClearButtonEnabled(); }

public Q_SLOTS:
    void setKeySequence(const EFIKeySequence &keySequence);
    void clear() { setKeySequence({}); }
    void setMaximumSequenceLength(qsizetype count);

Q_SIGNALS:
    void editingFinished();
    void keySequenceChanged(const EFIKeySequence &keySequence);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void resetState();
    void finishEditing();
};
