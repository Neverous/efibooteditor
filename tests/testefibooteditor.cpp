// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include <QIcon>
#include <QTest>

#include "efibooteditor.h"
#include "main.h"

class TestEFIBootEditor: public QObject
{
    Q_OBJECT

public:
    TestEFIBootEditor() = default;

private Q_SLOTS:
    void testMainWindow() const;
};

void TestEFIBootEditor::testMainWindow() const
{
    EFIBootEditor gui("");
    gui.show();

    QCOMPARE(QTest::qWaitForWindowExposed(&gui), true);
    QTest::qWait(500);

    QPixmap snapshot = gui.grab();
    QCOMPARE(snapshot.save("screenshot.png"), true);
}

auto main(int argc, char *argv[]) -> int
{
    QApplication app(argc, argv);
    setupApplication();
    setupTranslations();
    setupStyle();

    TestEFIBootEditor test;
    return QTest::qExec(&test, argc, argv);
}

#include "testefibooteditor.moc"
