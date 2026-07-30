#include "storage/Database.h"
#include "storage/Repositories.h"

#include <QCoreApplication>
#include <QTemporaryDir>

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    QTemporaryDir directory;
    if (!directory.isValid()) {
        std::cerr << "Could not create temporary directory\n";
        return EXIT_FAILURE;
    }

    opencaddie::storage::Database database;
    if (!database.open(directory.filePath(QStringLiteral("test.sqlite")))) {
        std::cerr << database.lastError().toStdString() << '\n';
        return EXIT_FAILURE;
    }
    if (database.schemaVersion() != 1) {
        std::cerr << "Migration version mismatch\n";
        return EXIT_FAILURE;
    }

    opencaddie::storage::ClubRepository clubs(database.connection());
    if (!clubs.ensureDefaultProfile()) return EXIT_FAILURE;
    const auto profile = clubs.defaultProfileId();
    const auto clubId = clubs.create(profile, QStringLiteral("7 iron"), 145.0);
    if (clubId.isEmpty() || clubs.list(profile).size() != 1) return EXIT_FAILURE;

    opencaddie::storage::RoundRepository rounds(database.connection());
    const auto round = rounds.start({QStringLiteral("fixture"),
                                     QStringLiteral("Fixture Golf Club"),
                                     QStringLiteral("v1"),
                                     profile,
                                     18,
                                     opencaddie::domain::ScoringMode::Stableford,
                                     12,
                                     QStringLiteral("Yellow")});
    if (!round) return EXIT_FAILURE;
    if (!rounds.saveScore(*round, {1, 4, 7},
                          {.hole = 1, .strokes = 5, .putts = 2})) {
        return EXIT_FAILURE;
    }
    if (!rounds.active() || rounds.scores(*round).front().strokes != 5) {
        return EXIT_FAILURE;
    }

    // Re-open the database to exercise WAL recovery and committed score durability.
    database.close();
    if (!database.open(directory.filePath(QStringLiteral("test.sqlite")))) {
        return EXIT_FAILURE;
    }
    opencaddie::storage::RoundRepository reopened(database.connection());
    const auto resumed = reopened.active();
    if (!resumed || reopened.scores(*resumed).front().strokes != 5) {
        std::cerr << "Committed score did not survive reopen\n";
        return EXIT_FAILURE;
    }

    std::cout << "All storage tests passed\n";
    return EXIT_SUCCESS;
}
