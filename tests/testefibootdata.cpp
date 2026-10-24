// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include <QSignalSpy>
#include <QTest>

#include "efibootdata.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#define QCOMPARE_NE(a, b) QVERIFY((a) != (b))
#define QCOMPARE_GT(a, b) QVERIFY((a) > (b))
#endif

class TestEFIBootData: public QObject
{
    Q_OBJECT

public:
    TestEFIBootData();

    void setRequireEFIEntries(bool value) { this->require_efi_entries = value; }

private Q_SLOTS:
    void testReload() const;

    // imports
    void testImportRaw() const;
    void testImportNative() const;
    void verifyImport(const EFIBootData &data) const;

    void showError(const QString &message, const QString &details) const;
    void showProgress(size_t step, size_t total, const QString &details) const;

private:
    bool efi_supported = true;
    bool require_efi_entries = true;
};

TestEFIBootData::TestEFIBootData()
    : QObject()
{
    auto efi_error_message = EFIBoot::init();
    if(efi_error_message)
    {
        this->efi_supported = false;
        showError("EFI support required", QStringFromStdTString(*efi_error_message));
    }
}

void TestEFIBootData::showError(const QString &message, const QString &details) const
{
    qWarning() << QString("ERROR: %0! %1").arg(message, details) << Qt::endl;
}

void TestEFIBootData::showProgress(size_t step, size_t total, const QString &details) const
{
    if(step >= total)
        total = step + 1;

    qDebug() << QString("[%0%] (%1/%2) %3").arg(100 * step / total).arg(step).arg(total).arg(details);
}

void TestEFIBootData::testReload() const
{
    if(!efi_supported)
    {
        QSKIP("EFI not supported.");
    }

    EFIBootData data;
    QSignalSpy errors(&data, &EFIBootData::error);
    (void)connect(&data, &EFIBootData::progress, this, &TestEFIBootData::showProgress);
    (void)connect(&data, &EFIBootData::error, this, &TestEFIBootData::showError);

    data.reload(require_efi_entries);

    QCOMPARE(errors.count(), 0);
}

void TestEFIBootData::testImportRaw() const
{
    EFIBootData data;
    QSignalSpy errors(&data, &EFIBootData::error);
    (void)connect(&data, &EFIBootData::progress, this, &TestEFIBootData::showProgress);
    (void)connect(&data, &EFIBootData::error, this, &TestEFIBootData::showError);

    data.import_("files/raw.json");

    QCOMPARE(errors.count(), 0);

    verifyImport(data);
}

void TestEFIBootData::testImportNative() const
{
    EFIBootData data;
    QSignalSpy errors(&data, &EFIBootData::error);
    (void)connect(&data, &EFIBootData::progress, this, &TestEFIBootData::showProgress);
    (void)connect(&data, &EFIBootData::error, this, &TestEFIBootData::showError);

    data.import_("files/native.json");

    QCOMPARE(errors.count(), 0);

    verifyImport(data);
}

void TestEFIBootData::verifyImport(const EFIBootData &data) const
{
    QCOMPARE(data.boot_entries_list_model.rowCount(), 7);

    for(const auto &entry: data.boot_entries_list_model.getEntries())
    {
        QCOMPARE_NE(entry.attributes, EFIBoot::Load_option_attribute::EMPTY);
        QCOMPARE_NE(entry.description, "");
        QCOMPARE(entry.efi_attributes, 7);
        QCOMPARE_GT(entry.device_path.size(), 0);
    }

    QCOMPARE(data.driver_entries_list_model.rowCount(), 0);
    QCOMPARE(data.sysprep_entries_list_model.rowCount(), 0);
    QCOMPARE(data.platform_recovery_entries_list_model.rowCount(), 1);
    QCOMPARE(data.hot_keys_list_model.rowCount(), 5);

    for(const auto &entry: data.hot_keys_list_model.getEntries())
    {
        QCOMPARE_NE(entry.index, -1);
        QCOMPARE(entry.efi_attributes, 7);
        QCOMPARE(entry.keys.isEmpty(), false);
    }

    QCOMPARE(data.apple_boot_args, "");
    QCOMPARE(data.supported_indications, 65);
    QCOMPARE(data.indications, 0);
    QCOMPARE(data.boot_option_support, 787);
    QCOMPARE(data.timeout, 10);
    QCOMPARE(data.secure_boot, false);
    QCOMPARE(data.vendor_keys, true);
    QCOMPARE(data.setup_mode, true);
    QCOMPARE(data.audit_mode, false);
    QCOMPARE(data.deployed_mode, false);
}

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication app(argc, argv);
    TestEFIBootData test;
    if(qgetenv("TEST_ALLOW_NO_EFI_ENTRIES") == "true")
        test.setRequireEFIEntries(false);

    return QTest::qExec(&test, argc, argv);
}

#include "testefibootdata.moc"
