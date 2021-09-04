#include <QApplication>
#include <QClipboard>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QString>
#include <QStyle>
#include <QWidget>

#include <cmath> // For ceil

struct MainWindow : QMainWindow {
    MainWindow(const QApplication &app) {
        const QRect availableGeometry(app.screens().first()->availableGeometry());
        const QSize availableSize(availableGeometry.size());
        int width {availableSize.width()};
        int height {availableSize.height()};

        width /= 2;
        height /= 2;

        const QSize newSize(width, height);
        setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, newSize, availableGeometry));    
        
        QWidget *widget = new QWidget(this);
        QFormLayout *layout = new QFormLayout(widget);

        QLineEdit *playerInit = new QLineEdit("5", widget); // long
        QLineEdit *playerMult = new QLineEdit("1", widget); // 1.0-10.0
        QLineEdit *playerMax = new QLineEdit("180", widget); // 76-449
        QLineEdit *engramStartAtZero = new QLineEdit("true", widget); // bool
        QLineEdit *engramInit = new QLineEdit("8", widget); // long
        QLineEdit *engramMult = new QLineEdit("1", widget); // 1.0-10.0
        QLineEdit *creatureInit = new QLineEdit("10", widget); // long
        QLineEdit *creatureMult = new QLineEdit("1", widget); // 1.0-10.0
        QLineEdit *creatureMax = new QLineEdit("88", widget); // 2-449
        QPushButton *button = new QPushButton("Run", widget);

        connect(button, &QPushButton::clicked, [playerInit,
            playerMult,
            playerMax,
            engramStartAtZero,
            engramInit,
            engramMult,
            creatureInit,
            creatureMult, 
            creatureMax,
            this,
            layout,
            widget] {
                for (int i = 0; i < layout->count(); i++) {
                    QWidget *toCheck {layout->itemAt(i)->widget()};
                    QLabel *toRemove {dynamic_cast<QLabel*>(toCheck)};
                    if (toRemove && toRemove->text().startsWith("[Error]")) {
                        layout->removeWidget(toRemove);
                        delete toCheck;
                        break;
                    }
                }
                bool playerInitOk = false;
                const long playerInitLong {playerInit->text().toLong(&playerInitOk)};
                const QString initialXpMessage {"Initial xp must be a whole number between (inclusive) 2 and 2147483647"};
                if (!playerInitOk || playerInitLong < 2) {
                    error(initialXpMessage, layout, widget);
                    return;
                }

                bool playerMultOk = false;
                const float playerMultFloat {playerMult->text().toFloat(&playerMultOk)};
                const QString multMessage("Multiplier must be a float between (inclusive) 1.0 and 10.0");
                if (!playerMultOk || playerMultFloat < 1.0 || playerMultFloat > 10.0) {
                    error(multMessage, layout, widget);
                    return;
                }

                bool playerMaxOk = false;
                const int playerMaxInt {playerMax->text().toInt(&playerMaxOk)};
                if (!playerMaxOk || playerMaxInt < 76 || playerMaxInt > 449) {
                    error("Player max level must be a whole number between (inclusive) 76 and 449", layout, widget);
                    return;
                }

                bool engramStartAtZeroBool {engramStartAtZero->text() == "true"};

                bool engramInitOk = false;
                const long engramInitLong {engramInit->text().toLong(&engramInitOk)};
                if (!engramInitOk || engramInitLong < 2) {
                    error("Initial engram points must be a whole number between (inclusive) 2 and 2147483647", layout, widget);
                    return;
                }

                bool engramMultOk = false;
                const float engramMultFloat {engramMult->text().toFloat(&engramMultOk)};
                if (!engramMultOk || engramMultFloat < 1.0 || engramMultFloat > 10.0) {
                    error("Engram point multiplier must be a float between (inclusive) 1.0 and 10.0", layout, widget);
                    return;
                }

                bool creatureInitOk = false;
                const long creatureInitLong {creatureInit->text().toLong(&creatureInitOk)};
                if (!creatureInitOk || creatureInitLong < 2) {
                    error(initialXpMessage, layout, widget);
                    return;
                }

                bool creatureMultOk = false;
                const float creatureMultFloat {creatureMult->text().toFloat(&creatureMultOk)};
                if (!creatureMultOk || creatureMultFloat < 1.0 || creatureMultFloat > 10.0) {
                    error(multMessage, layout, widget);
                    return;
                }

                bool creatureMaxOk = false;
                const int creatureMaxInt {creatureMax->text().toInt(&creatureMaxOk)};
                if (!creatureMaxOk || creatureMaxInt < 2 || creatureMaxInt > 449) {
                    error("Creature max level must be a whole number between (inclusive) 2 and 449", layout, widget);
                    return;
                }
                QString *output = new QString;
                // Player level start
                double playerCurrentValue = playerInitLong;
                *output+=QString("LevelExperienceRampOverrides=(ExperiencePointsForLevel[0]=%1, ").arg(playerCurrentValue);

                bool failed = false;
                QString playerOverflowMessage("The next player value would overflow and was discarded, the result works but does not contain all levels");
                for (int i = 1; i < playerMaxInt; i++) {
                    if (testOverflowMult(playerCurrentValue, playerMultFloat)) {
                        error(playerOverflowMessage, layout, widget);
                        failed = true;
                        break;
                    }
                    playerCurrentValue *= playerMultFloat;

                    // To avoid rounding down
                    if (testOverflowInc(playerCurrentValue)) {
                        error(playerOverflowMessage, layout, widget);
                        failed = true;
                        break;
                    }
                    playerCurrentValue = std::ceil(playerCurrentValue);
                    *output+=QString("ExperiencePointsForLevel[%1]=%2, ").arg(QString::number(i), QString::number(static_cast<long>(playerCurrentValue))); // Cast because number() uses E notation by default, alternative would be setting the format and precision
                }

                // Remove trailing comma and space
                output->chop(2);

                *output+=")";
                // Player level end
                *output+="\n";
                // Creature level start
                double creatureCurrentValue = creatureInitLong;
                *output+=QString("LevelExperienceRampOverrides=(ExperiencePointsForLevel[0]=%1, ").arg(creatureCurrentValue);

                QString creatureOverflowMessage("The next creature value would overflow and was discarded, the result works but does not contain all levels");
                for (int i = 1; i < creatureMaxInt; i++) {
                    if (failed) {
                        break;
                    }

                    if (creatureCurrentValue > MAX_VALUE / creatureMultFloat) {
                        error(creatureOverflowMessage, layout, widget);
                        failed = true;
                        break;
                    }
                    creatureCurrentValue *= creatureMultFloat;

                    // To avoid rounding down
                    if (testOverflowInc(creatureCurrentValue)) {
                        error(creatureOverflowMessage, layout, widget);
                        failed = true;
                        break;
                    }
                    creatureCurrentValue = std::ceil(creatureCurrentValue);
                    *output+=QString("ExperiencePointsForLevel[%1]=%2, ").arg(QString::number(i), QString::number(static_cast<long>(creatureCurrentValue)));
                }

                // Remove trailing comma and space
                output->chop(2);

                *output+=")";
                // Creature level end
                *output+="\n";
                
                double engramCurrentValue = engramInitLong;
                *output+=QString("OverridePlayerLevelEngramPoints=%1\n").arg(engramStartAtZero ? 0 : engramCurrentValue);
                
                QString engramOverflowMessage("The next engram value would overflow and was discarded, the result works but does not contain all levels");
                for (int i = 1; i < playerMaxInt; i++) {
                    if (failed) {
                        break;
                    }

                    if (engramCurrentValue > MAX_VALUE / engramMultFloat) {
                        error(engramOverflowMessage, layout, widget);
                        break;
                    }
                    engramCurrentValue *= engramMultFloat;

                    // To avoid rounding down
                    if (testOverflowInc(engramCurrentValue)) {
                        error(engramOverflowMessage, layout, widget);
                        break;
                    }
                    engramCurrentValue = std::ceil(engramCurrentValue);
                    *output+=QString("OverridePlayerLevelEngramPoints=%1\n").arg(static_cast<long>(engramCurrentValue));
                }

                // Remove trailing new line
                output->chop(1);

                QApplication::clipboard()->setText(*output);
                delete output;
        });

        layout->addRow("Initial experience needed (player):", playerInit);
        layout->addRow("Multiply by per level (player):", playerMult);
        layout->addRow("Max level (player):", playerMax);
        layout->addRow("Engram start at 0 (true/false):", engramStartAtZero);
        layout->addRow("Initial engram points:", engramInit);
        layout->addRow("Multiply by per level (engram points):", engramMult);
        layout->addRow("Initial experience needed (creature):", creatureInit);
        layout->addRow("Multiply by per level (creature):", creatureMult);
        layout->addRow("Max level (creature):", creatureMax);
        layout->addWidget(button);

        widget->setLayout(layout);
        setCentralWidget(widget);
    }
    private:
        void error(const QString &message, QFormLayout *layout, QWidget *widget) {
            QLabel *label = new QLabel("[Error] " + message, widget);
            label->setStyleSheet("QLabel { color: red }");
            layout->addWidget(label);
        }

        // Overflow checks, not using climits as ark likely only accepts 2^31-1, but LONG_MAX is platform dependent
        static constexpr long MAX_VALUE = 2147483647;
        bool testOverflowMult(const double x, const float y) {
            return x > MAX_VALUE / y;
        }

        bool testOverflowInc(const double x) {
            return x > MAX_VALUE - 1;
        }
};

int main(int argc, char **argv) {
    const QApplication app(argc, argv);

    MainWindow window(app);
    window.show();

    return app.exec();
}
