#include <QtTest>
#include "RandomSelector.h"
#include "SelectorRegistry.h"

class TstSelectorRegistry : public QObject {
    Q_OBJECT
private slots:
    void defaultsAreRegistered();
    void createsByName();
    void unknownNameReturnsNull();
    void customSelectorCanBeAdded();
};

void TstSelectorRegistry::defaultsAreRegistered() {
    const auto r = SelectorRegistry::withDefaults();
    QCOMPARE(r.names(), (QStringList{QStringLiteral("ByRating"), QStringLiteral("Random"),
                                     QStringLiteral("WeakestTheme")}));
}

void TstSelectorRegistry::createsByName() {
    const auto r = SelectorRegistry::withDefaults();
    for (const QString& name : r.names()) {
        const auto s = r.create(name, 1);
        QVERIFY(s);
        QCOMPARE(s->name(), name);
    }
}

void TstSelectorRegistry::unknownNameReturnsNull() {
    QVERIFY(!SelectorRegistry::withDefaults().create(QStringLiteral("Nope"), 1));
    QVERIFY(!SelectorRegistry::withDefaults().contains(QStringLiteral("Nope")));
}

void TstSelectorRegistry::customSelectorCanBeAdded() {
    SelectorRegistry r;
    r.add(QStringLiteral("Mine"), [](quint32 seed) {
        return std::make_unique<RandomSelector>(seed);
    });
    QVERIFY(r.contains(QStringLiteral("Mine")));
    QVERIFY(r.create(QStringLiteral("Mine"), 1));
}

QTEST_APPLESS_MAIN(TstSelectorRegistry)
#include "tst_selectorregistry.moc"
