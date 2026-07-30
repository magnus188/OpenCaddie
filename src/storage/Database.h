#pragma once

#include <QSqlDatabase>
#include <QString>

namespace opencaddie::storage {

class Database {
public:
    Database();
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool open(const QString& path);
    void close();
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QSqlDatabase connection() const;
    [[nodiscard]] QString lastError() const;
    [[nodiscard]] int schemaVersion() const;

private:
    bool applyMigrations();
    bool execute(const QString& sql);
    bool setError(const QString& context);

    QString m_connectionName;
    QString m_lastError;
};

} // namespace opencaddie::storage

