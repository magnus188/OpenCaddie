#pragma once

#include <QObject>
#include <QTranslator>

class QQmlEngine;

namespace opencaddie::ui {

class TranslationManager final : public QObject {
    Q_OBJECT

public:
    explicit TranslationManager(QQmlEngine* engine, QObject* parent = nullptr);

public slots:
    void setLanguage(const QString& language);

private:
    QQmlEngine* m_engine;
    QTranslator m_translator;
};

} // namespace opencaddie::ui

