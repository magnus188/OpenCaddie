# OpenGolfMap device contract

OpenCaddie consumes OpenGolfMap manifest schema v2. Search returns selectable OSM
candidates; exact import identifies a candidate by OSM type and ID. A course ZIP
contains the existing rendered assets plus:

- `render-model.json` for semantic local rendering and user colors
- `navigation.json` with course location and per-hole WGS84 play line, green,
  tees, projection origin/rotation, and view box
- quality, version/update time, ODbL attribution, byte sizes, and SHA-256 hashes

The device rejects corrupt, incomplete, incompatible, or incorrectly attributed
packages without replacing an installed version. `includeNavigation` and
`includeRenderModel` default to true for bundle requests.

