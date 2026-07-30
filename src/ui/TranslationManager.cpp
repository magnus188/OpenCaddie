#include "ui/TranslationManager.h"

#include <QCoreApplication>
#include <QQmlEngine>

namespace opencaddie::ui {

TranslationManager::TranslationManager(QQmlEngine* engine, QObject* parent)
    : QObject(parent), m_engine(engine) {}

void TranslationManager::setLanguage(const QString& language) {
    QCoreApplication::removeTranslator(&m_translator);
    if (language == QStringLiteral("nb") &&
        m_translator.load(QStringLiteral(":/i18n/opencaddie_nb.qm"))) {
        QCoreApplication::installTranslator(&m_translator);
    }
    if (m_engine) m_engine->retranslate();
}

} // namespace opencaddie::ui

