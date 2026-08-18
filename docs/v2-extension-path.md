# V2 extension path

These are stable boundaries, not enabled V1 features.

## BLE multiplayer

A host device owns the round. Peers exchange LE Secure Connections events with
UUID event IDs, monotonically ordered actor sequence numbers, acknowledgements,
periodic snapshots, and reconnect replay. Score mutations are idempotent; no
device merges raw SQLite files.

## Extended shot logging

V1 already provides optional landing-point tracking with an image-based club
picker, latest-shot undo/type correction, editable score synchronization, and
local route display. V2 may add editing older strokes, historical-round map
playback, lateral/fairway projections, and per-club dispersion. Those additions
should remain rebuildable projections over the existing canonical shot events
rather than changing the optional V1 workflow.

## Self-hosted sync

The planned stack is PostgreSQL, a TypeScript/Fastify API, React administration,
and reverse-proxy TLS in Docker Compose. Devices pair once, receive scoped
revocable tokens, and drain an idempotent outbox while staying offline-first.

## External providers

Provider interfaces accept approved official APIs only. Garmin Golf access is
partner-dependent and write-back remains disabled unless Garmin documentation
explicitly authorizes it. TrackMan CSV is the preferred first file importer.
Toptracer network access and GolfBox/NGF handicap, score submission, and
tee-time actions remain disabled pending vendor credentials and agreements.
FIT, CSV, and JSON exchange remain the portable path. See the dated
[integration feasibility review](integrations.md).

## OTA

The CM0 carrier plan uses signed RAUC bundles with U-Boot A/B slots, boot-health
confirmation, automatic rollback, release channels, and documented recovery.
