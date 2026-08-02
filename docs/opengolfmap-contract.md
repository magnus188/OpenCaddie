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

## Service origin and round maps

OpenGolfMap is self-hosted. OpenCaddie accepts an HTTPS service origin from the
connectivity settings, `--opengolfmap-url`, or
`OPENCADDIE_OPENGOLFMAP_URL`; plain HTTP is accepted only for localhost. The
client checks `/api/v1/health`, searches `/api/v1/courses/search`, imports the
exact selected candidate, and requests a schema-v2 ZIP from `/api/v1/graphics`.

During a round the network service is not used. Both the compact hole graphic
and expanded zoom/pan/rotate/measure view render `render-model.json` locally.
Measurement and dogleg points stay in the model's local-metre coordinate space,
so their distances do not change under viewport transforms.
