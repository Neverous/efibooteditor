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

    class Renderer
    {
        const QWidgetItemDelegate<Widget, Item> &delegate;
        Widget widget;

        class Guard: private QObject
        {
            friend class Renderer;

            Renderer &renderer;

            Guard(const Guard &) = delete;
            Guard &operator=(const Guard &) = delete;

        public:
            ~Guard();
            operator Widget *() { return &renderer.widget; }
            Widget *operator->() { return &renderer.widget; }

        private:
            Guard(Renderer &_renderer, QWidget *parent, const Item &item, const QRect &geometry);
        };

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

    public:
        Renderer(const QWidgetItemDelegate<Widget, Item> &_delegate)
            : delegate(_delegate)
            , widget()
        {
        }
        Guard getWidget(QWidget *parent, const Item &item, const QRect &geometry) { return {*this, parent, item, geometry}; }
    };

    mutable Renderer renderer;
    mutable Renderer painter;
    mutable Renderer event_handler;

public:
    explicit QWidgetItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *_painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual void setupWidgetFromItem(Widget &, const Item &) const {}
    virtual bool handleWidgetDelegateEventResult(const QEvent *, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex &, const Widget &, bool result) const { return result; }

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

template <class Widget, class Item>
QWidgetItemDelegate<Widget, Item>::Renderer::Guard::~Guard()
{
    renderer.widget.setParent(nullptr);
}

template <class Widget, class Item>
QWidgetItemDelegate<Widget, Item>::Renderer::Guard::Guard(QWidgetItemDelegate<Widget, Item>::Renderer &_renderer, QWidget *parent, const Item &item, const QRect &geometry)
    : renderer{_renderer}
{
    renderer.widget.setParent(parent);
    renderer.delegate.setupWidgetFromItem(renderer.widget, item);
    renderer.widget.setGeometry(geometry);
    // force layout
    renderer.widget.grab();
}

template <class Widget, class Item>
QWidgetItemDelegate<Widget, Item>::QWidgetItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , renderer(*this)
    , painter(*this)
    , event_handler(*this)
{
}

template <class Widget, class Item>
void QWidgetItemDelegate<Widget, Item>::paint(QPainter *_painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.data().canConvert<Item>())
        return QStyledItemDelegate::paint(_painter, option, index);

    auto item = index.data().value<Item>();
    auto widget = this->painter.getWidget(const_cast<QWidget *>(option.widget), item, option.rect);

    _painter->save();
    if(option.state & QStyle::State_Selected)
        _painter->fillRect(option.rect, option.palette.highlight());

    _painter->translate(option.rect.topLeft());
    widget->render(_painter, QPoint{}, QRegion{}, QWidget::RenderFlag::DrawChildren);
    _painter->restore();
    return;
}

template <class Widget, class Item>
QSize QWidgetItemDelegate<Widget, Item>::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.data().canConvert<Item>())
        return QStyledItemDelegate::sizeHint(option, index);

    auto item = index.data().value<Item>();
    auto widget = renderer.getWidget(const_cast<QWidget *>(option.widget), item, option.rect);

    QSize hint;
#ifdef QLISTVIEW_SIZEHINT_BUG
    // BUG: option.rect is not updated on list resize ;( ignore it and use parent widget size instead
    // https://bugreports.qt.io/browse/QTBUG-11227
    auto fixed_rect = option.widget->geometry();
    fixed_rect.adjust(0, 0, -2, -2);
    fixed_rect.setHeight(widget->heightForWidth(fixed_rect.width()));
    hint = fixed_rect.size();
#else
    hint = widget->sizeHint();
#endif
    return hint;
}

template <class Widget, class Item>
bool QWidgetItemDelegate<Widget, Item>::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(index.data().canConvert<Item>())
    {
        bool result = false;
        auto item = index.data().value<Item>();
        auto widget = event_handler.getWidget(const_cast<QWidget *>(option.widget), item, option.rect);

        switch(event->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        {
            QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
            auto pos = widget->mapFromParent(mouse_event->pos());
            mouse_event->setLocalPos(pos);

            QWidget *child = widget->childAt(pos);
            if(child)
            {
                auto child_pos = child->mapFromParent(pos);
                mouse_event->setLocalPos(child_pos);
                result = QApplication::sendEvent(child, event);
            }
            break;
        }

        default:
            break;
        }

        if(!event->isAccepted())
            result = QApplication::sendEvent(widget, event);

        if(event->isAccepted())
            handleWidgetDelegateEventResult(event, model, option, index, *widget, result);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
