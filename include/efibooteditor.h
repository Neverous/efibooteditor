// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QProgressDialog>
#include <QUndoStack>
#include <memory>

#include "bootentrylistview.h"
#include "disableundoredo.h"
#include "efibootdata.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class EFIBootEditor;
}
QT_END_NAMESPACE

class EFIBootEditor: public QMainWindow
{
    Q_OBJECT

private:
    enum BootEntryType
    {
        BOOT = 0,
        DRIVER = 1,
        SYSPREP = 2,
        PLATFORM_RECOVERY = 3,
    };

    std::unique_ptr<Ui::EFIBootEditor> ui;
    EFIBootData data{this};

    std::unique_ptr<QMessageBox> confirmation;
    std::unique_ptr<QMessageBox> error;
    std::unique_ptr<QProgressDialog> progress;
    std::unique_ptr<DisableUndoRedo> disable_undo_redo;

    QUndoStack undo_stack{this};

public:
    explicit EFIBootEditor(const std::optional<tstring> &efi_error_message, QWidget *parent = nullptr);
    EFIBootEditor(const EFIBootEditor &) = delete;
    EFIBootEditor &operator=(const EFIBootEditor &) = delete;
    ~EFIBootEditor() override;

    void reloadBootConfiguration();

public slots:
    void reload();
    void save();
    void import();
    void export_();
    void dump();
    void reorder();

    void undo();
    void redo();

    void removeCurrentBootEntry();
    void moveCurrentBootEntryUp();
    void moveCurrentBootEntryDown();
    void insertBootEntry();

    void enableBootEntryEditor(const QModelIndex &index);
    void switchBootEntryEditor(int index);
    void showAboutBox();

    void setOsIndicationsSupported(uint64_t value);
    void setOsIndications(uint64_t value);

    void setOsIndication(bool checked);

    void updateBootOptionSupport(uint32_t flags);

    void undoViewChanged(const QModelIndex &index);

signals:
    void osIndicationsChanged(uint64_t value);

private:
    void disableBootEntryEditor();
    void refreshBootEntryEditor();
    void reorderBootEntries();

    std::tuple<QString, BootEntryListView &, BootEntryListModel &> getBootEntryList(int index);
    std::tuple<QString, BootEntryListView &, BootEntryListModel &> currentBootEntryList();

    uint64_t getOsIndications() const;

    void closeEvent(QCloseEvent *event) override;

    template <class Receiver, typename Slot>
    void showConfirmation(const QString &message, const QMessageBox::StandardButtons &buttons, const QMessageBox::StandardButton &confirmation_button, Receiver confirmation_context, Slot confirmation_slot);

    void showError(const QString &message, const QString &details);
    void showProgressBar(size_t step, size_t total, const QString &details);
    void hideProgressBar();
};
