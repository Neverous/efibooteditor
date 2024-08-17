// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "hotkeydelegate.h"

void HotKeyBootOptionDelegate::refreshBootOptions(const BootEntryListModel &model)
{
    painter.clear();
    renderer.clear();
    event_handler.clear();

    for(const auto &entry: model.getEntries())
    {
        painter.addItem(entry.getTitle(), entry.index);
        renderer.addItem(entry.getTitle(), entry.index);
        event_handler.addItem(entry.getTitle(), entry.index);
    }
}

void HotKeyBootOptionDelegate::setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex & /*index*/, int role) const
{
    if(role == Qt::EditRole)
    {
        for(int index = 0; index < event_handler.count(); ++index)
            widget.addItem(event_handler.itemText(index), event_handler.itemData(index));
    }

    for(int index = 0; index < widget.count(); ++index)
    {
        if(widget.itemData(index) == item)
        {
            widget.setCurrentIndex(index);
            break;
        }
    }

    if(role == Qt::EditRole)
    {
        // Commit data after select
        connect(&widget, &QComboBox::activated, this, [&](int /*index*/)
            {
            // by pretending enter was sent
            QKeyEvent enter{QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier};
            QApplication::sendEvent(&widget, &enter); });
    }
}

bool HotKeyBootOptionDelegate::setupItemFromWidget(const Widget &widget, Item &item, const QModelIndex & /*index*/) const
{
    item = widget.currentData().value<Item>();
    return true;
}

bool HotKeyBootOptionDelegate::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::FocusIn)
    {
        auto *editor = dynamic_cast<Widget *>(object);
        const auto *focus = dynamic_cast<QFocusEvent *>(event);
        if(editor && focus && focus->reason() != Qt::PopupFocusReason)
            editor->showPopup();
    }

    return QWidgetItemDelegate<Widget, Item>::eventFilter(object, event);
}

void HotKeyKeysDelegate::setMaximumSequenceLength(qsizetype count)
{
    maximumSequenceLength = count;
}

void HotKeyKeysDelegate::setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex & /*index*/, int role) const
{
    widget.setClearButtonEnabled(role == Qt::EditRole);
    widget.setMaximumSequenceLength(maximumSequenceLength);
    widget.setKeySequence(*item);

    if(role == Qt::EditRole)
    {
        // Commit data after edit is finished
        connect(&widget, &EFIKeySequenceEdit::editingFinished, this, [&]()
            {
            // by pretending enter was sent
            QKeyEvent enter{QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier};
            QApplication::sendEvent(&widget, &enter); });
    }
}

bool HotKeyKeysDelegate::setupItemFromWidget(const Widget &widget, Item &item, const QModelIndex & /*index*/) const
{
    item = &widget.keySequence();
    return true;
}
