# Hardware

## Prototype

- Raspberry Pi 5
- Waveshare 4.3-inch 800×480 DSI capacitive display
- Any gpsd-compatible USB or UART GNSS receiver
- Regulated battery supply with adequate peak current

## Production target

- Wireless 16 GB Compute Module Zero on a custom carrier
- u-blox MAX-M10S-class UART GNSS
- DSI display, Bluetooth 5, battery gauge and brightness control
- Storage partitioning sized for signed RAUC A/B system images

The current display is approximately 300 nits and must pass an outdoor
readability gate before production. Carrier design must expose a safe power
button/hold-up strategy so SQLite commits complete before power removal.

## Raspberry Pi OS

Use Raspberry Pi OS Lite 64-bit, KMS, NetworkManager, gpsd, and a dedicated
`opencaddie` user in the `video`, `render`, `input`, and `dialout` groups. Apply
`deploy/raspberry-pi/config.txt.fragment`, install the service and tmpfiles
definitions, then verify display/touch rotation and UART assignment on the exact
carrier revision.

Six-hour round endurance, thermal behavior, GNSS cold starts, suspend/restart,
and steady-state RSS below 256 MB are release acceptance tests.

