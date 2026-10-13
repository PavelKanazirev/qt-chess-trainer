#include <QtTest>
#include "Version.h"

class TstVersion : public QObject {
    Q_OBJECT
private slots:
    void versionIsSet() { QVERIFY(!trainerVersion().isEmpty()); }
};

QTEST_APPLESS_MAIN(TstVersion)
#include "tst_version.moc"
