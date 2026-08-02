# V2 extension path

These are stable boundaries, not enabled V1 features.

## BLE multiplayer

A host device owns the round. Peers exchange LE Secure Connections events with
UUID event IDs, monotonically ordered actor sequence numbers, acknowledgements,
periodic snapshots, and reconnect replay. Score mutations are idempotent; no
device merges raw SQLite files.

## Shot logging

One tap selects/confirms a club and marks a GPS point. The next marker completes
the previous shot. Events retain accuracy and original coordinates and support
undo/edit. Derived distance, lateral direction, fairway result, and per-club
left/centre/right percentages are projections that can be rebuilt.

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
