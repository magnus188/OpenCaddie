# Security policy

## Supported versions

Security updates are provided for the latest tagged release and `main`.

## Reporting

Do not open a public issue for a suspected vulnerability. Use GitHub private
vulnerability reporting for `magnus188/OpenCaddie` once available, or contact
the maintainer through the private address on their GitHub profile. Include the
affected revision, device mode, impact, and a minimal reproduction. Please do
not include real Wi-Fi credentials, access tokens, or personal location data.

## Security properties

- OpenCaddie stores no Wi-Fi credentials; NetworkManager owns them.
- Course ZIPs are size-bounded, path-checked, hash-verified, and installed
  atomically.
- Round data is local by default and excluded from logs.
- Remote course endpoints require HTTPS, except loopback development.
- Future sync, BLE, and OTA features must use scoped credentials, authenticated
  encryption, signed artifacts, and rollback-safe state transitions.

