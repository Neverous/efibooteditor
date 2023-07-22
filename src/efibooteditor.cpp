// SPDX-License-Identifier: LGPL-3.0-or-later
#include "efibooteditor.h"

#include "bootentry.h"
#include "form/ui_efibooteditor.h"
#include <QButtonGroup>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMetaMethod>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <cctype>

EFIBootEditor::EFIBootEditor(QWidget *parent)
    : QMainWindow{parent}
    , ui{std::make_unique<Ui::EFIBootEditor>()}
    , confirmation{std::make_unique<QMessageBox>(QMessageBox::Question, qApp->applicationName(), "", QMessageBox::NoButton, this)}
    , error{std::make_unique<QMessageBox>(QMessageBox::Critical, qApp->applicationName(), "", QMessageBox::NoButton, this)}
    , progress{std::make_unique<QProgressDialog>(tr("Working…"), nullptr, 0, 0, this)}
    , disableUndoRedo{std::make_unique<DisableUndoRedo>()}
{
    data.setUndoStack(&undo_stack);
    ui->setupUi(this);
    progress->setWindowModality(Qt::WindowModal);
    ui->boot_entries_list->setModel(&data.boot_entries_list_model);
    ui->driver_entries_list->setModel(&data.driver_entries_list_model);
    ui->sysprep_entries_list->setModel(&data.sysprep_entries_list_model);
    ui->platform_recovery_entries_list->setModel(&data.platform_recovery_entries_list_model);
    ui->entry_form->setBootEntryListModel(data.boot_entries_list_model);
    ui->undo_view->setStack(data.undo_stack);
    ui->undo_view->setHidden(true);
    ui->undo->setEnabled(false);
    ui->redo->setEnabled(false);

    // Connect settings tabs events
    QObject::connect(&data, &EFIBootData::error, this, &EFIBootEditor::showError);
    QObject::connect(&data, &EFIBootData::progress, this, &EFIBootEditor::showProgressBar);
    QObject::connect(&data, &EFIBootData::done, this, &EFIBootEditor::hideProgressBar);

    QObject::connect(&data, &EFIBootData::timeoutChanged, ui->timeout_number, &QSpinBox::setValue);
    QObject::connect(ui->timeout_number, &QSpinBox::valueChanged, &data, &EFIBootData::setTimeout);

    QObject::connect(&data, &EFIBootData::secureBootChanged, ui->secure_boot, &QRadioButton::setChecked);
    QObject::connect(&data, &EFIBootData::vendorKeysChanged, ui->vendor_keys, &QRadioButton::setChecked);
    QObject::connect(&data, &EFIBootData::setupModeChanged, ui->setup_mode, &QRadioButton::setChecked);
    QObject::connect(&data, &EFIBootData::auditModeChanged, ui->audit_mode, &QRadioButton::setChecked);
    QObject::connect(&data, &EFIBootData::deployedModeChanged, ui->deployed_mode, &QRadioButton::setChecked);
    QObject::connect(&data, &EFIBootData::bootOptionSupportChanged, this, &EFIBootEditor::updateBootOptionSupport);

    QObject::connect(&data, &EFIBootData::appleBootArgsChanged, ui->boot_args_text, &QLineEdit::setText);
    QObject::connect(ui->boot_args_text, &QLineEdit::textEdited, &data, &EFIBootData::setAppleBootArgs);

    QObject::connect(&data, &EFIBootData::osIndicationsSupportedChanged, this, &EFIBootEditor::setOsIndicationsSupported);
    QObject::connect(&data, &EFIBootData::osIndicationsChanged, this, &EFIBootEditor::setOsIndications);
    QObject::connect(this, &EFIBootEditor::osIndicationsChanged, &data, &EFIBootData::setOsIndications);

    // Connect undo/redo events
    QObject::connect(&undo_stack, &QUndoStack::cleanChanged, this, [&](bool clean)
        { ui->undo_view->setHidden(clean && !undo_stack.canRedo()); });

    QObject::connect(&undo_stack, &QUndoStack::canUndoChanged, ui->undo, &QAction::setEnabled);
    QObject::connect(&undo_stack, &QUndoStack::canRedoChanged, ui->redo, &QAction::setEnabled);

    QObject::connect(&undo_stack, &QUndoStack::undoTextChanged, this, [&](const QString &text)
        { ui->undo->setText(tr("Undo %1").arg(text)); });

    QObject::connect(&undo_stack, &QUndoStack::redoTextChanged, this, [&](const QString &text)
        { ui->redo->setText(tr("Redo %1").arg(text)); });

    // Disable builtin undo/redo support in some widgets
    for(auto &widget: ui->settings->findChildren<QLineEdit *>())
        widget->installEventFilter(disableUndoRedo.get());

    for(auto &widget: ui->entry_form->findChildren<QLineEdit *>())
        widget->installEventFilter(disableUndoRedo.get());

    for(auto &widget: ui->settings->findChildren<QPlainTextEdit *>())
        widget->installEventFilter(disableUndoRedo.get());

    for(auto &widget: ui->entry_form->findChildren<QPlainTextEdit *>())
        widget->installEventFilter(disableUndoRedo.get());

    for(auto &widget: ui->settings->findChildren<QSpinBox *>())
        widget->installEventFilter(disableUndoRedo.get());

    for(auto &widget: ui->entry_form->findChildren<QSpinBox *>())
        widget->installEventFilter(disableUndoRedo.get());

    ui->entries->setCurrentIndex(0);
    ui->settings->setCurrentIndex(0);

    updateBootOptionSupport(0);
}

EFIBootEditor::~EFIBootEditor()
{
}

void EFIBootEditor::reload()
{
    showConfirmation(
        tr("Are you sure you want to reload the entries?<br/>ALL of your changes will be lost!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        this,
        &EFIBootEditor::reloadBootConfiguration);
}

void EFIBootEditor::reloadBootConfiguration()
{
    disableBootEntryEditor();
    ui->entries->setCurrentIndex(0);
    ui->settings->setCurrentIndex(0);
    ui->undo_view->setHidden(true);
    ui->undo->setEnabled(false);
    ui->redo->setEnabled(false);
    data.clear();
    data.setUndoStack(nullptr);
    data.reload();
    data.setUndoStack(&undo_stack);
}

void EFIBootEditor::reorder()
{
    showConfirmation(
        tr("Are you sure you want to reorder the boot entries?<br/>All indexes will be overwritten!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        this,
        &EFIBootEditor::reorderBootEntries);
}

void EFIBootEditor::undo()
{
    undo_stack.undo();
    refreshBootEntryEditor();
}

void EFIBootEditor::redo()
{
    undo_stack.redo();
    refreshBootEntryEditor();
}

void EFIBootEditor::enableBootEntryEditor(const QModelIndex &index)
{
    if(!index.isValid())
        return disableBootEntryEditor();

    const auto item = index.data().value<const BootEntry *>();
    ui->entry_form->setItem(index, item);
    auto [name, list, model] = currentBootEntryList();
    (void)name;
    (void)list;
    ui->entry_form->setReadOnly((model.options & BootEntryListModel::ReadOnly) || item->is_error);
}

void EFIBootEditor::disableBootEntryEditor()
{
    ui->entry_form->setItem({}, nullptr);
}

void EFIBootEditor::refreshBootEntryEditor()
{
    auto [name, list, model] = currentBootEntryList();
    (void)name;
    (void)model;
    enableBootEntryEditor(list.currentIndex());
}

void EFIBootEditor::switchBootEntryEditor(int index)
{
    auto [name, list, model] = getBootEntryList(index);
    (void)name;
    ui->entry_form->setBootEntryListModel(model);
    list.setCurrentIndex(list.currentIndex());
    enableBootEntryEditor(list.currentIndex());
    ui->entries_actions->setDisabled(model.options & BootEntryListModel::ReadOnly);
}

void EFIBootEditor::save()
{
    showConfirmation(
        tr("Are you sure you want to save?<br/>Your EFI configuration will be overwritten!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        &data,
        &EFIBootData::save);
}

void EFIBootEditor::import()
{
    const QString file_name = QFileDialog::getOpenFileName(this, tr("Open boot configuration dump"), "", tr("JSON documents (*.json)"));
    if(file_name.isEmpty())
        return;

    disableBootEntryEditor();
    ui->entries->setCurrentIndex(0);
    ui->settings->setCurrentIndex(0);
    ui->undo_view->setHidden(true);
    ui->undo->setEnabled(false);
    ui->redo->setEnabled(false);
    data.clear();
    data.setUndoStack(nullptr);
    data.import(file_name);
    data.setUndoStack(&undo_stack);
}

void EFIBootEditor::export_()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save boot configuration dump"), "", tr("JSON documents (*.json)"));
    if(file_name.isEmpty())
        return;

    if(!file_name.endsWith(".json", Qt::CaseInsensitive))
        file_name += ".json";

    data.export_(file_name);
}

void EFIBootEditor::dump()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save raw EFI dump"), "", tr("JSON documents (*.json)"));
    if(file_name.isEmpty())
        return;

    if(!file_name.endsWith(".json", Qt::CaseInsensitive))
        file_name += ".json";

    data.dump(file_name);
}

void EFIBootEditor::showAboutBox()
{
    auto *about = new QMessageBox(QMessageBox::Information,
        tr("About EFI Boot Editor"),
        //: About dialog
        tr("<h1>EFI Boot Editor</h1>"
           "<p>Version <b>%1</b></p>"
           "<p>Boot Editor for (U)EFI based systems.</p>")
            .arg(QCoreApplication::applicationVersion()),
        QMessageBox::Close,
        this);
    about->setIconPixmap(QIcon::fromTheme("preferences-system").pixmap(128, 128));
    //: About dialog details
    about->setInformativeText(tr("<p><a href='%1'>Website</a></p>"
                                 "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"
                                 "<p>License: <a href='https://www.gnu.org/licenses/lgpl.html'>GNU LGPL Version 3</a></p>"
                                 "<p>On Linux uses <a href='https://github.com/rhboot/efivar'>efivar</a> for EFI variables access.</p>"
                                 "<p>Uses Tango Icons as fallback icons.</p>")
                                  .arg(PROJECT_HOMEPAGE_URL));
    QObject::connect(about->button(QMessageBox::Close), &QAbstractButton::clicked, about, &QObject::deleteLater);
    QObject::connect(qApp, &QApplication::aboutToQuit, about, &QObject::deleteLater);
    about->show();
}

void EFIBootEditor::setOsIndicationsSupported(uint64_t value)
{
    // Firmware features
    ui->timestamp_based_revocation->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION);
    ui->file_capsule_support->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED);
    ui->fmp_capsule_support->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED);
    ui->capsule_reporting_support->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED);

    // Firmware actions
    ui->boot_to_firmware_ui->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI);
    ui->start_os_recovery->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY);
    ui->start_platform_recovery->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY);
    ui->collect_current_config->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH);
}

void EFIBootEditor::setOsIndications(uint64_t value)
{
    ui->boot_to_firmware_ui->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI);
    ui->start_os_recovery->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY);
    ui->start_platform_recovery->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY);
    ui->collect_current_config->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH);
}

void EFIBootEditor::setOsIndication(bool)
{
    emit osIndicationsChanged(getOsIndications());
}

void EFIBootEditor::updateBootOptionSupport(uint32_t flags)
{
    // TODO: (flags & EFIBoot::EFI_BOOT_OPTION_SUPPORT_KEY)
    ui->entry_form->showCategory(flags & EFIBoot::EFI_BOOT_OPTION_SUPPORT_APP);
    ui->entries->setTabVisible(ui->entries->indexOf(ui->sysprep_tab), flags & EFIBoot::EFI_BOOT_OPTION_SUPPORT_SYSPREP);
}

void EFIBootEditor::undoViewChanged(const QModelIndex &)
{
    refreshBootEntryEditor();
}

void EFIBootEditor::reorderBootEntries()
{
    auto [name, list, model] = currentBootEntryList();
    if(model.options & BootEntryListModel::ReadOnly)
        return;

    disableBootEntryEditor();

    undo_stack.beginMacro(tr("Reorder %1 entries").arg(name));
    // Skip indexes with errors to not overwrite them accidentally
    QSet<uint16_t> errors;
    for(int r = 0; r < model.rowCount(); ++r)
    {
        auto entry = model.index(r).data().value<const BootEntry *>();
        if(entry->is_error)
            errors.insert(entry->index);
    }

    uint16_t index = 0;
    for(int r = 0; r < model.rowCount(); ++r)
    {
        auto idx = model.index(r);
        if(idx.data().value<const BootEntry *>()->is_error)
            continue;

        while(errors.contains(index))
            index++;

        model.setEntryIndex(idx, index++);
    }

    undo_stack.endMacro();
    enableBootEntryEditor(list.currentIndex());
}

void EFIBootEditor::removeCurrentBootEntry()
{
    auto [name, list, model] = currentBootEntryList();
    (void)name;
    if(model.options & BootEntryListModel::ReadOnly)
        return;

    list.removeCurrentRow();
}

void EFIBootEditor::moveCurrentBootEntryUp()
{
    auto [name, list, model] = currentBootEntryList();
    (void)name;
    if(model.options & BootEntryListModel::ReadOnly)
        return;

    list.moveCurrentRowUp();
}

void EFIBootEditor::moveCurrentBootEntryDown()
{
    auto [name, list, model] = currentBootEntryList();
    (void)name;
    if(model.options & BootEntryListModel::ReadOnly)
        return;

    list.moveCurrentRowDown();
}

void EFIBootEditor::insertBootEntry()
{
    auto [name, list, model] = currentBootEntryList();
    (void)name;
    if(model.options & BootEntryListModel::ReadOnly)
        return;

    list.insertRow();
}

std::tuple<QString, BootEntryListView &, BootEntryListModel &> EFIBootEditor::getBootEntryList(int index)
{
    switch(static_cast<BootEntryType>(index))
    {
    case BootEntryType::BOOT:
        return {tr("Boot"), *ui->boot_entries_list, data.boot_entries_list_model};

    case BootEntryType::DRIVER:
        return {tr("Driver"), *ui->driver_entries_list, data.driver_entries_list_model};

    case BootEntryType::SYSPREP:
        return {tr("System Preparation"), *ui->sysprep_entries_list, data.sysprep_entries_list_model};

    case BootEntryType::PLATFORM_RECOVERY:
        return {tr("Platform Recovery"), *ui->platform_recovery_entries_list, data.platform_recovery_entries_list_model};
    }

    Q_UNREACHABLE();
    return {tr("Boot"), *ui->boot_entries_list, data.boot_entries_list_model};
}

std::tuple<QString, BootEntryListView &, BootEntryListModel &> EFIBootEditor::currentBootEntryList()
{
    return getBootEntryList(ui->entries->currentIndex());
}

uint64_t EFIBootEditor::getOsIndications() const
{
    uint64_t indications = 0;
    if(ui->boot_to_firmware_ui->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI;

    if(ui->start_os_recovery->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY;

    if(ui->start_platform_recovery->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY;

    if(ui->collect_current_config->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH;

    return indications;
}

void EFIBootEditor::closeEvent(QCloseEvent *event)
{
    event->ignore();
    confirmation->setText(tr("Are you sure you want to quit?"));
    confirmation->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmation->exec();
    if(confirmation->clickedButton() == confirmation->button(QMessageBox::Yes))
        event->accept();
}

void EFIBootEditor::showError(const QString &message, const QString &details)
{
    hideProgressBar();
    error->setText(message);
    error->setDetailedText(details);
    error->show();
}

template <class Receiver, typename Slot>
void EFIBootEditor::showConfirmation(const QString &message, const QMessageBox::StandardButtons &buttons, const QMessageBox::StandardButton &confirmation_button, Receiver confirmation_context, Slot confirmation_slot)
{
    confirmation->setText(message);
    confirmation->setStandardButtons(buttons);
    QObject::connect(confirmation->button(confirmation_button), &QAbstractButton::clicked, confirmation_context, confirmation_slot);
    confirmation->show();
}

void EFIBootEditor::showProgressBar(size_t step, size_t total, const QString &details)
{
    progress->setMaximum(static_cast<int>(total));
    progress->setLabelText(details);
    progress->setValue(static_cast<int>(step));
}

void EFIBootEditor::hideProgressBar()
{
    progress->reset();
}
