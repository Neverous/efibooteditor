// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include <QSignalSpy>
#include <QTest>

#include "efibootdata.h"

class TestEFIBootData: public QObject
{
    Q_OBJECT

public:
    TestEFIBootData();

    void setRequireEntries(bool value) { this->require_entries = value; }

private Q_SLOTS:
    void testReload() const;

    void showError(const QString &message, const QString &details) const;
    void showProgress(size_t step, size_t total, const QString &details) const;

private:
    bool efi_supported = true;
    bool require_entries = true;
};

TestEFIBootData::TestEFIBootData()
    : QObject()
{
    auto efi_error_message = EFIBoot::init();
    if(efi_error_message)
    {
        this->efi_supported = false;
        showError(tr("EFI support required"), QStringFromStdTString(*efi_error_message));
    }
}

void TestEFIBootData::showError(const QString &message, const QString &details) const
{
    qWarning() << tr("ERROR: %0! %1").arg(message, details) << Qt::endl;
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
    QSignalSpy spy(&data, &EFIBootData::error);
    (void)connect(&data, &EFIBootData::progress, this, &TestEFIBootData::showProgress);
    (void)connect(&data, &EFIBootData::error, this, &TestEFIBootData::showError);

    data.reload(require_entries);

    QCOMPARE(spy.count(), 0);
}

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication app(argc, argv);
    TestEFIBootData test;
    if(QCoreApplication::arguments().contains("--allow-no-entries"))
        test.setRequireEntries(false);

    return QTest::qExec(&test, argc, argv);
}

#include "testefibootdata.moc"
