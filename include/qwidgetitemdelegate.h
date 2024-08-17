// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QApplication>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItem>
#include <QStyledItemDelegate>

template <class Widget_, class Item_ = QStandardItem>
class QWidgetItemDelegate: public QStyledItemDelegate
{
public:
    using Widget = Widget_;
    using Item = Item_;

protected:
    mutable Widget renderer{};
    mutable Widget painter{};
    Widget event_handler{};

public:
    explicit QWidgetItemDelegate(QObject *parent = nullptr);
    QWidgetItemDelegate(const QWidgetItemDelegate &) = delete;
    QWidgetItemDelegate &operator=(const QWidgetItemDelegate &) = delete;

protected:
    void paint(QPainter *_painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    virtual void setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex &index, int role) const = 0;
    virtual bool setupItemFromWidget(const Widget & /*widget*/, Item & /*item*/, const QModelIndex & /*index*/) const { return false; }

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

template <class Widget, class Item>
QWidgetItemDelegate<Widget, Item>::QWidgetItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

template <class Widget, class Item>
void QWidgetItemDelegate<Widget, Item>::paint(QPainter *_painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid() || !index.data().canConvert<Item>())
        return QStyledItemDelegate::paint(_painter, option, index);

    auto item = index.data().value<Item>();
    setupWidgetFromItem(painter, item, index, Qt::DisplayRole);
    painter.setParent(const_cast<QWidget *>(option.widget));
    painter.setGeometry(option.rect);
    _painter->save();
    if(option.state & QStyle::State_Selected)
        _painter->fillRect(option.rect, option.palette.highlight());

    _painter->translate(option.rect.topLeft());
    painter.render(_painter, QPoint{}, QRegion{}, QWidget::RenderFlag::DrawChildren);
    _painter->restore();
    painter.setParent(nullptr);
    return;
}

template <class Widget, class Item>
QSize QWidgetItemDelegate<Widget, Item>::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid() || !index.data().canConvert<Item>())
        return QStyledItemDelegate::sizeHint(option, index);

    auto item = index.data().value<Item>();
    setupWidgetFromItem(renderer, item, index, Qt::SizeHintRole);
    renderer.grab(); // force layout

    // Take rect into consideration for word wrapping etc.
    auto rect = option.rect;
    if(!rect.isValid())
        return renderer.sizeHint();

    rect.setHeight(renderer.heightForWidth(option.rect.width()));
    return rect.size();
}

template <class Widget, class Item>
QWidget *QWidgetItemDelegate<Widget, Item>::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid() || !index.data().canConvert<Item>())
        return QStyledItemDelegate::createEditor(parent, option, index);

    Widget *editor = new Widget{parent};
    return editor;
}

template <class Widget, class Item>
void QWidgetItemDelegate<Widget, Item>::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(!index.isValid() || !index.data().canConvert<Item>())
        return QStyledItemDelegate::setEditorData(editor, index);

    const auto item = index.data().value<Item>();
    setupWidgetFromItem(*dynamic_cast<Widget *>(editor), item, index, Qt::EditRole);
}

template <class Widget, class Item>
void QWidgetItemDelegate<Widget, Item>::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(!index.isValid() || !index.data().canConvert<Item>() || !model)
        return QStyledItemDelegate::setModelData(editor, model, index);

    Item item;
    if(!setupItemFromWidget(*dynamic_cast<Widget *>(editor), item, index))
        return;

    QVariant data;
    data.setValue(item);

    model->setData(index, data);
}

template <class Widget, class Item>
bool QWidgetItemDelegate<Widget, Item>::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Handle events on delegates with disabled editor
    if(index.isValid() && index.data().canConvert<Item>() && !(index.flags() & Qt::ItemIsEditable))
    {
        auto item = index.data().value<Item>();
        setupWidgetFromItem(event_handler, item, index, Qt::EditRole);
        event_handler.setParent(const_cast<QWidget *>(option.widget));
        event_handler.setGeometry(option.rect);
        event_handler.grab(); // force layout

        if(const auto passthrough_events = {QEvent::MouseButtonPress, QEvent::MouseButtonRelease, QEvent::MouseButtonDblClick, QEvent::MouseMove}; std::find(std::begin(passthrough_events), std::end(passthrough_events), event->type()) != std::end(passthrough_events))
        {
            auto mouse_event = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            auto position = event_handler.mapFromParent(mouse_event->position());
            QWidget *child = event_handler.childAt(position.toPoint());
            if(child)
            {
                auto child_position = child->mapFromParent(position);
                // All I need is to set localPosition...
                QMouseEvent child_event{mouse_event->type(), child_position, mouse_event->scenePosition(), mouse_event->globalPosition(), mouse_event->button(), mouse_event->buttons(), mouse_event->modifiers(), Qt::MouseEventSource::MouseEventSynthesizedByApplication, mouse_event->pointingDevice()};
                QApplication::sendEvent(child, &child_event);
                event_handler.setParent(nullptr);
                return false; // indicate event as not handled to allow for selection etc.
            }
#else
            auto pos = event_handler.mapFromParent(mouse_event->pos());
            mouse_event->setLocalPos(pos);

            QWidget *child = event_handler.childAt(pos);
            if(child)
            {
                auto child_pos = child->mapFromParent(pos);
                mouse_event->setLocalPos(child_pos);
                QApplication::sendEvent(child, event);
                event_handler.setParent(nullptr);
                return false; // indicate event as not handled to allow for selection etc.
            }
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            // All I need is to set localPosition...
            QMouseEvent widget_event{mouse_event->type(), position, mouse_event->scenePosition(), mouse_event->globalPosition(), mouse_event->button(), mouse_event->buttons(), mouse_event->modifiers(), Qt::MouseEventSource::MouseEventSynthesizedByApplication, mouse_event->pointingDevice()};
            QApplication::sendEvent(&event_handler, &widget_event);
#else
            QApplication::sendEvent(&event_handler, event);
#endif
            event_handler.setParent(nullptr);
            return false; // indicate event as not handled to allow for selection etc.
        }

        QApplication::sendEvent(&event_handler, event);
        event_handler.setParent(nullptr);
        return false; // indicate event as not handled to allow for selection etc.
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
