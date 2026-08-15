# OpenCaddie brand mark

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="opencaddie-lockup.svg">
  <img src="opencaddie-lockup-ink.svg" alt="OpenCaddie" width="316">
</picture>

## The mark

An **open C**: a 290° arc with a ball resting in its mouth.

- **C** is Caddie.
- The arc is **open** — the gap is the whole point. Open source, open data, open to
  the player's judgement. The mark refuses to close the loop, because the app refuses
  to hit the shot for you.
- The **ball** sits on the lip of the opening. It is the only solid form in the mark:
  everything else is advice, the ball is the decision. It is also the single dot of
  certainty a GPS gives you — your position, on a course you already carry offline.
- Read a second way, the arc is a **green seen from above** and the ball is on its
  edge — the same top-down semantic geometry the app renders on every hole.

The gradient runs deep green at the bottom of the arc to fairway green at the mouth:
the light comes from where the shot is going.

## Philosophy

Three commitments the mark has to carry:

1. **Local-first.** Nothing in the mark depends on a network, a photo, or a texture.
   One arc, one circle, two colours. It renders identically on a 16px favicon and on
   a Compute Module in direct sunlight.
2. **Instrument, not app.** The geometry is exact — a true circle, a stroke that is
   exactly 1/5 of the radius, caps that are perfect semicircles. This is a measuring
   device. It should feel drawn with a compass, not sketched.
3. **Quiet under pressure.** Golf UI is read at arm's length, mid-round, in glare.
   The mark never competes with the map. That is why there is no flag, no club, no
   swing arc, no ball dimples — the four clichés of golf branding, all declined.

## Geometry

Drawn on a 64-unit grid. Circle centre `(31, 32)`, radius `20`, stroke `8`, round
caps. The gap spans ±35° about the 3 o'clock axis. Ball centre `(51, 32)`, radius `6`,
sitting on the arc's centreline so it half-overlaps the stroke's outer edge.

Optical bounding box is `50 × 48`. **Clearspace** on all sides is half the mark's
height (24 units at grid scale). In the lockup, the wordmark's cap height is 60% of
the mark's height, set in Inter — `Open` at 500, `Caddie` at 700, tracking −1%.

Minimum sizes: **16px** for the mark, **96px** wide for the lockup. Below 16px, drop
the gradient for a flat `#2FCB63` or `currentColor`.

## Colour

| Role | Token | Hex |
| --- | --- | --- |
| Arc, dark end | `Theme.greenDeep` | `#167B43` |
| Arc, light end / ball | `Theme.fairway` | `#2FCB63` |
| Ink (mono on light) | `Theme.text` light | `#172017` |
| Paper | `Theme.background` light | `#F6F7F0` |
| Tile | `Theme.surface` dark → `Theme.focusBackground` | `#181B19` → `#0B0D0C` |

Every value is already a `Theme.qml` token — the logo and the product share one
palette, no brand-only colours.

## Files

| File | Use |
| --- | --- |
| `opencaddie-mark.svg` | Primary mark, gradient, 64-grid |
| `opencaddie-mark-mono.svg` | Single-colour, inherits `currentColor` — favicons, print, knockout |
| `opencaddie-lockup.svg` | Mark + wordmark, horizontal — light text, for dark backgrounds |
| `opencaddie-lockup-ink.svg` | Same lockup in ink, for light backgrounds |
| `opencaddie-icon.svg` | 512px app tile on the dark surface gradient |
| `opencaddie-icon-512.png` | Rasterised tile for stores, docs, and READMEs |

The lockup's wordmark is live text referencing Inter (bundled in
`src/ui/assets/fonts/`). **Outline it** before using the lockup anywhere outside this
repo.

## In the product

| Surface | Source |
| --- | --- |
| Boot splash | `src/ui/qml/components/SplashOverlay.qml`, built on `BrandLockup.qml` |
| In-app mark artwork | `src/ui/assets/brand/mark.svg` (viewBox cropped to the artwork so QML `Image` bounds equal the visual bounds) |
| macOS Dock and Finder icon | `packaging/macos/icon.svg` → `packaging/macos/OpenCaddie.icns` |
| Window icon (Linux, EGLFS) | `QGuiApplication::setWindowIcon` in `src/ui/main.cpp` |

Regenerate the macOS icon after editing its SVG:

```sh
./tools/make-macos-icon.sh   # needs rsvg-convert: brew install librsvg
```

The splash runs for about 1.3 s, is dismissible by tap, and is skipped entirely
when the app is launched with `--screenshot` so captured screens stay deterministic.

### Don't

Recolour outside the palette · rotate the mark (the gap belongs at 3 o'clock) ·
detach or resize the ball · close the gap · add a flag, tee, club or dimples ·
place the gradient mark on mid-green backgrounds · stack the lockup vertically
without regenerating clearspace.

---

## Design prompt

Paste this into Claude (or any design tool) to regenerate, extend, or brief out the
system — icons, a wordmark, a splash, merch, a second mark.

> **Brief: OpenCaddie visual identity**
>
> OpenCaddie is an open-source, local-first golf computer that runs on a Raspberry Pi
> with an 800×480 touchscreen, carried in a bag and read at arm's length in direct
> sunlight. It holds offline OpenStreetMap-derived course maps, gives live GNSS
> distances, plans layups, and keeps score. It has no account, no cloud, and no
> connection during a round. It is an instrument, not a lifestyle app.
>
> **Philosophy.** Three commitments, in priority order:
> 1. *Local-first* — the identity must survive with no gradients, no photography, no
>    texture, no network. One or two flat colours, always.
> 2. *Instrument, not app* — geometry is exact and constructible with a compass and
>    ruler. State the construction (centres, radii, stroke ratios, angles) as part of
>    the design; a shape you cannot specify numerically is not finished.
> 3. *Quiet under pressure* — this is read mid-round, in glare, one-handed. Nothing
>    may compete with the course map, which is the real hero of the product.
>
> **Concept to hold.** The core mark is an **open C** — an arc of roughly 290°, gap at
> 3 o'clock, with a solid ball resting in the mouth. The arc is advice; the ball is the
> player's decision, and it is the only closed form. The openness is literal: open
> source, open data, and a tool that deliberately does not close the loop for you.
> Any extension must preserve the open gap and the single solid counterpoint.
>
> **Palette** (from the app's own `Theme.qml` — introduce no brand-only colours):
> fairway `#2FCB63`, deep green `#167B43`, ink `#172017`, paper `#F6F7F0`,
> dark surface `#181B19`, near-black `#0B0D0C`. Accents that exist in-product but are
> reserved for data, not branding: sand `#E0C27A`, water `#2BA7D7`, amber `#D48A35`,
> danger `#D94D3E`.
>
> **Typography.** Inter. `Open` at weight 500, `Caddie` at 700, tracking −1%, no other
> weights in the lockup. Cap height 60% of mark height, clearspace half the mark height.
>
> **Hard constraints.**
> - Must be legible as a 16px monochrome favicon and as a 512px app tile.
> - Must work in light mode, dark mode, and knocked out white on deep green.
> - Deliver as hand-written SVG on a 64-unit grid with exact coordinates — not traced,
>   not raster, no filters, no embedded fonts.
> - Sufficient contrast against both `#F6F7F0` and `#101211`.
>
> **Explicitly declined.** Flagsticks. Golf clubs. Swing arcs. Ball dimples. Tee pegs.
> Rolling hills. Anything a stock golf logo would do. Also avoid: circles with a gap at
> 12 o'clock (power button), a circle with a bottom-right tail (magnifying glass), and
> tapering arcs (loading spinner).
>
> **Deliver:** the mark, a monochrome variant using `currentColor`, a horizontal
> lockup, and a 512px app tile — plus one paragraph defending why the geometry means
> what you say it means.
