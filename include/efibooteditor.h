// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <memory>

#include "bootentrylistmodel.h"

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
    std::unique_ptr<Ui::EFIBootEditor> ui;
    BootEntryListModel entries_list_model{};
    std::unique_ptr<QMessageBox> confirmation = nullptr;
    std::unique_ptr<QMessageBox> error = nullptr;

public:
    explicit EFIBootEditor(QWidget *parent = nullptr);
    ~EFIBootEditor() override;

public slots:
    void reload();
    void save();
    void import();
    void export_();
    void dump();
    void resetBootConfiguration();
    void enableBootEntryEditor(const QModelIndex &index);
    void disableBootEntryEditor();
    void saveBootConfiguration();
    void importBootConfiguration(const QString &file_name);
    void exportBootConfiguration(const QString &file_name);
    void dumpRawEFIData(const QString &file_name);
    void showAboutBox();

private:
    void closeEvent(QCloseEvent *event) override;

    void importJSONEFIData(const QJsonObject &input);
    void importRawEFIData(const QJsonObject &input);

    void show_error(const QString &message, const QString &details = "");
    template <class Receiver, typename Slot>
    void show_confirmation(const QString &message, const QMessageBox::StandardButtons &buttons, const QMessageBox::StandardButton &confirmation_button, Receiver confirmation_context, Slot confirmation_slot);
};
