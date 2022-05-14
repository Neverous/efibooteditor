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
    typedef Widget_ Widget;
    typedef Item_ Item;

private:
    friend class Guard;

    mutable Widget renderer;
    mutable Widget painter;
    Widget event_handler;

public:
    explicit QWidgetItemDelegate(QObject *parent = nullptr);
    QWidgetItemDelegate(const QWidgetItemDelegate &) = delete;
    QWidgetItemDelegate &operator=(const QWidgetItemDelegate &) = delete;

    void paint(QPainter *_painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual void setupWidgetFromItem(Widget &, const Item &) const { }
    virtual bool handleWidgetDelegateEventResult(const QEvent *, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex &, const Widget &, bool result) const { return result; }

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

template <class Widget, class Item>
QWidgetItemDelegate<Widget, Item>::QWidgetItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , renderer()
    , painter()
    , event_handler()
{
}

template <class Widget, class Item>
void QWidgetItemDelegate<Widget, Item>::paint(QPainter *_painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.data().canConvert<Item>())
        return QStyledItemDelegate::paint(_painter, option, index);

    auto item = index.data().value<Item>();
    setupWidgetFromItem(painter, item);
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
    if(!index.data().canConvert<Item>())
        return QStyledItemDelegate::sizeHint(option, index);

    auto item = index.data().value<Item>();
    setupWidgetFromItem(renderer, item);
    renderer.grab(); // force layout

    auto rect = option.rect;
    rect.setHeight(renderer.heightForWidth(option.rect.width()));
    return rect.size();
}

template <class Widget, class Item>
bool QWidgetItemDelegate<Widget, Item>::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(index.data().canConvert<Item>())
    {
        bool result = false;
        auto item = index.data().value<Item>();
        setupWidgetFromItem(event_handler, item);
        event_handler.setParent(const_cast<QWidget *>(option.widget));
        event_handler.setGeometry(option.rect);
        event_handler.grab(); // force layout

        const auto passthrough_events = {QEvent::MouseButtonPress, QEvent::MouseButtonRelease, QEvent::MouseButtonDblClick, QEvent::MouseMove};
        if(std::find(std::begin(passthrough_events), std::end(passthrough_events), event->type()) != std::end(passthrough_events))
        {
            QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            auto position = event_handler.mapFromParent(mouse_event->position());
            QWidget *child = event_handler.childAt(position.toPoint());
            if(child)
            {
                auto child_position = child->mapFromParent(position);
                // All I need is to set localPosition...
                QMouseEvent child_event{mouse_event->type(), child_position, mouse_event->scenePosition(), mouse_event->globalPosition(), mouse_event->button(), mouse_event->buttons(), mouse_event->modifiers(), Qt::MouseEventSource::MouseEventSynthesizedByApplication, mouse_event->pointingDevice()};
                result = QApplication::sendEvent(child, &child_event);
            }
#else
            auto pos = event_handler.mapFromParent(mouse_event->pos());
            mouse_event->setLocalPos(pos);

            QWidget *child = event_handler.childAt(pos);
            if(child)
            {
                auto child_pos = child->mapFromParent(pos);
                mouse_event->setLocalPos(child_pos);
                result = QApplication::sendEvent(child, event);
            }
#endif
        }

        if(!event->isAccepted())
            result = QApplication::sendEvent(&event_handler, event);

        if(event->isAccepted())
            result = handleWidgetDelegateEventResult(event, model, option, index, event_handler, result);

        event_handler.setParent(nullptr);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
