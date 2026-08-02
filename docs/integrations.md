# External golf data integrations

Status reviewed: 2026-07-31.

OpenCaddie only integrates through documented exports or an approved vendor
API. A visible web or mobile application is not an API contract, and OpenCaddie
will not scrape pages, automate a player's login, or store a vendor password.

## Feasibility summary

| Provider | Data that appears feasible | Access status | OpenCaddie decision |
| --- | --- | --- | --- |
| Garmin Golf | Hole scorecards, fairways, putts, launch-monitor metrics, and per-shot GPS locations/distances | Garmin Golf Premium API is licensed and limited to select approved partners | Keep cloud sync disabled until approval. Apply for read access. Do not claim or implement scorecard write-back without a documented write endpoint. |
| Garmin Connect | Consented activity detail as FIT, GPX, or TCX | Enterprise/business OAuth 2.0 program with approval and evaluation environment | Treat as a supplementary activity/file source. Do not assume that every Garmin Golf field is present in an activity file. |
| TrackMan | Shot Analysis metrics through TrackMan CSV exports | Manual export is officially documented; no public consumer cloud API was found | A local, user-initiated CSV importer is the lowest-risk first integration and is planned, not implemented. Preserve the original file checksum and source fields. Cloud automation needs a TrackMan agreement. |
| Toptracer | Sessions, completed games, shots, shot traces, launch metrics, and recorded weather | A documented game-data OpenAPI publishes Keycloak and legacy API-key security schemes; no self-service production onboarding was found | Keep network sync partner-gated. Ask Toptracer for an OpenCaddie integration account, scopes, retention rules, and sandbox. |
| GolfBox Norge / NGF | Potentially handicap, score submission, and tee-time workflows | No public player OAuth/API documentation was found. Current standard GolfBox integrations documented by NGF cover economy, members, and club/course data. NGF is planning a controlled Unionsdatabase and two-way integrations. | Keep HCP read, score submission, and booking disabled until NGF/GolfBox provides an approved supplier agreement, scopes, sandbox, and write rules. |

“Feasible” describes data represented by an official API or export. It does not
mean that OpenCaddie currently has credentials or permission to access it.

## Garmin

The Garmin Golf Premium API explicitly advertises scorecard data (including
par, strokes, fairways, and putts), launch-monitor ball/club metrics, and GPS
location/distance for every shot. Garmin describes it as a select-partner
product, requires an application, and states that a license fee applies.

The public overview documents access to data, but it does not document a Garmin
Golf scorecard write API. OpenCaddie's capability catalog therefore exposes
Garmin as `partner_required` and read-only. Round upload remains off.

Garmin Connect's separate Activity API uses OAuth 2.0 after business approval
and can provide complete activity files in FIT, GPX, or TCX. It can be useful
for user-owned file exchange, but is not treated as a substitute contract for
the richer Garmin Golf Premium model.

Recommended next action: apply with a concise data-flow diagram, requested
read scopes, expected user count, retention policy, deletion flow, and explicit
questions about scorecard write-back and FIT golf-field coverage.

## TrackMan

TrackMan documents exporting selected shots from TrackMan Performance Studio
Shot History or a live Shot Analysis session as TrackMan CSV. That makes an
explicit file picker and deterministic CSV import realistic without collecting
TrackMan credentials.

An importer should:

- detect the export schema/version and units instead of guessing;
- map measurements into canonical SI units while retaining original values;
- attach provider, external row/session ID, source timestamp, device, and file
  SHA-256 to every imported record;
- show a preview and warnings before writing;
- be idempotent when the same export is selected twice; and
- never silently combine practice shots with an on-course round.

The canonical DTO and metric schema provide a foundation, and on-course shot
writes are idempotent. Practice-session persistence, complete source provenance,
the CSV parser, and the import UI are not implemented yet.

## Toptracer

Toptracer's game-data OpenAPI exposes player sessions, completed games, session
shots, traces, activity results, and a weather response. Its published security
schemes include Keycloak and legacy credentials. No self-service production
onboarding is publicly documented. Public documentation of the schema is not
the same as permission to use production player data.

Recommended next action: send Toptracer a partner request asking for:

- OAuth/service-account onboarding and least-privilege read scopes;
- player consent and account-linking requirements;
- sandbox users and stable schema/versioning commitments;
- rate, retention, attribution, and deletion requirements; and
- whether round/game records may be exported into a user-owned local device.

No Toptracer write-back should be implemented without an explicit endpoint and
contract.

## GolfBox Norge and tee times

NGF says that GolfBox currently remains the core system for members, handicap,
and tournaments. NGF's current public integration page lists three standard
integrations—economy, member, and club/course—and describes them as data flows
from GolfBox to third-party club systems. Its GolfBox review says a planned
national Unionsdatabase should make controlled two-way integrations possible.
It also states that the current one-way scenario gives external systems no
write rights except the economy integration.

This means OpenCaddie should not ship a player-login workaround. For HCP,
scorecards, or tee times, request approved supplier onboarding from NGF and
GolfBox and ask for:

- player OAuth/account linking and handicap read scopes;
- WHS score validation, attestation/marker rules, corrections, duplicate
  handling, and write acknowledgements;
- course/tee/rating identifiers and home-club rules;
- tee-time search, pricing, membership entitlements, booking, payment,
  cancellation, and participant APIs;
- sandbox data, audit requirements, commercial terms, and launch approval.

Tee-time support is more than slot reservation: GolfBox's own player guidance
shows that selected club membership can affect booking windows and green-fee
pricing. Booking therefore remains separately capability-gated even if handicap
access is granted.

## Backend boundary and security

The repository now contains:

- a canonical external round, practice-session, weather, shot, launch-metric,
  WGS84 position, and provenance DTO scaffold in `src/integrations`;
- research/status metadata that clearly separates documented potential,
  requested operations, planned work, implemented operations, and authorization;
- schema-v2 shot records with stable identity, original provider/external IDs,
  accuracy, coordinates, distance, and source timestamp, plus a schema-v3
  canonical/source-unit metric table for extensible launch-monitor measurements;
- informational integration account status/reported capabilities in SQLite,
  deliberately without credentials and never used as authorization; and
- statistics queries that only use the owner participant and only report
  longest drive from a recorded/imported tee shot.

No vendor importer or network client is implemented yet.

Future cloud OAuth credentials belong in the self-hosted sync service's secret
store. The device should receive only a scoped, revocable OpenCaddie device
token. A permitted operation must be implemented locally, present in verified
service-side token scopes, and covered by explicit user consent; mutable device
metadata is never sufficient authorization. Imported source files stay local
unless the user explicitly enables sync.

Exact shot GPS, timestamps, external account identifiers, handicap, and score
history are sensitive personal data even though they are not passwords. The
current local SQLite database is not encrypted at rest, so physical device loss
can expose it. Before cloud integrations ship, OpenCaddie needs explicit
retention and deletion controls, export/account-unlink behavior, consent
boundaries, log redaction, encrypted server-side secrets, and device-loss
guidance.

## Primary sources

- [Garmin Golf Premium API overview](https://developer.garmin.com/golf-api/overview/)
- [Garmin Connect Activity API](https://developer.garmin.com/gc-developer-program/activity-api/)
- [Garmin Connect Developer Program FAQ](https://developer.garmin.com/gc-developer-program/program-faq/)
- [TrackMan TPS CSV export guide](https://support.trackmangolf.com/hc/en-us/articles/12985883274139-Shot-Analysis-How-To-Export-A-CSV-File-From-TPS)
- [Toptracer game-data API documentation](https://game-data.toptracer.com/docs)
- [Toptracer OpenAPI document](https://game-data.toptracer.com/openapi.json)
- [NGF digitalisation project and current GolfBox integrations](https://www.golfforbundet.no/klubb/organisasjon/digitaliseringsprosjektet/)
- [NGF GolfBox review and Unionsdatabase direction](https://www.golfforbundet.no/klubb/organisasjon/utredning-golfbox)
- [GolfBox guidance on membership selection for tee-time pricing](https://golfbox.zendesk.com/hc/no/articles/4889103962642-Velg-riktig-klubb-n%C3%A5r-du-bestiller-starttid)
