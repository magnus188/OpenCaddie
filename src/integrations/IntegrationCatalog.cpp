#include "integrations/IntegrationCatalog.h"

#include "integrations/ExternalGolfDataProvider.h"

#include <QVariantMap>

namespace opencaddie::integrations {

namespace {

QVariantMap entry(const QString &id, const QString &name, const QString &availability,
                  const QStringList &documentedPotentialOperations = {},
                  const QStringList &requestedOperations = {},
                  const QStringList &plannedOperations = {}) {
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), name},
        {QStringLiteral("availability"), availability},
        {QStringLiteral("implementedOperations"), QStringList{}},
        {QStringLiteral("documentedPotentialOperations"),
         documentedPotentialOperations},
        {QStringLiteral("requestedOperations"), requestedOperations},
        {QStringLiteral("plannedOperations"), plannedOperations},
        {QStringLiteral("authorized"), false},
    };
}

} // namespace

QVariantList integrationCatalog() {
    // This is research/status metadata only. It must never be used to authorize
    // an operation; authorization comes from verified service-side token scopes
    // plus explicit user consent.
    return {
        entry(QStringLiteral("garmin_golf"), QStringLiteral("Garmin Golf"),
              QStringLiteral("partner_required"),
              {QStringLiteral("rounds.read"), QStringLiteral("scorecards.read"),
               QStringLiteral("shots.read"), QStringLiteral("launch_metrics.read")}),
        entry(QStringLiteral("garmin_connect"), QStringLiteral("Garmin Connect"),
              QStringLiteral("partner_required"),
              {QStringLiteral("activities.read"), QStringLiteral("files.fit.read"),
               QStringLiteral("files.gpx.read"), QStringLiteral("files.tcx.read")}),
        entry(QStringLiteral("trackman"), QStringLiteral("TrackMan"),
              QStringLiteral("planned_file_import"),
              {QStringLiteral("files.csv.export")}, {}, {QStringLiteral("csv.import")}),
        entry(QStringLiteral("toptracer"), QStringLiteral("Toptracer"),
              QStringLiteral("partner_required"),
              {QStringLiteral("sessions.read"), QStringLiteral("shots.read"),
               QStringLiteral("launch_metrics.read"), QStringLiteral("weather.read")}),
        entry(QStringLiteral("golfbox_no"), QStringLiteral("GolfBox Norge"),
              QStringLiteral("partner_required"), {},
              {QStringLiteral("handicap.read"), QStringLiteral("whs_scores.submit"),
               QStringLiteral("tee_times.search"), QStringLiteral("bookings.create"),
               QStringLiteral("bookings.cancel")}),
    };
}

} // namespace opencaddie::integrations
