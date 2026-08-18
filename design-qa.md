# Design QA: Cubist Welcome Artwork

## Evidence

- Source visual truth: `/Users/magnustrandokken/.codex/generated_images/01a006c2-ef95-7c03-9dc8-ef4360070b0d/exec-948811f3-b914-47db-b446-724b02988efa.png`
- Existing-page reference: `.context/front-page-current.png`
- Dark implementation: `.context/home-course-art/dark-v4.png`
- Light implementation: `.context/home-course-art/light-v4.png`
- Combined comparison: `.context/home-course-art/qa-comparison-v4.png`
- Viewport: 800 x 480 logical pixels, captured at 1600 x 960 (`@2x`).
- Artwork slot: 270 x 458 logical pixels; implementation crop is 540 x 916.
- Source normalization: the 1024 x 1536 source was center-cropped to 906 x 1536 and downsampled to 540 x 916 before comparison.
- State: `WelcomeScreen`, English, no active round, simulator hardware; both dark and light palettes checked.

## Full-view Comparison

The combined comparison shows that the title, CTA, menu rows, dividers, status bar, spacing, and 800 x 480 page structure remain unchanged. The selected cubist course occupies only the existing right-hand artwork region and stays clear of the menu.

## Focused Artwork Comparison

The lower pair in the combined comparison places the normalized source and rendered artwork crop together. Course geometry, creek, bunkers, tee, green, dashed route, cubist framing, scale, and crop are preserved. No additional focused region was needed because the artwork is the only changed visual element and is fully legible in the 540 x 916 crop.

## Required Fidelity Surfaces

- Fonts and typography: unchanged; Inter family, weights, sizing, wrapping, and antialiasing match the existing screen.
- Spacing and layout rhythm: unchanged; the image fills the established 270 x 458 slot without overlap or persistent-control clipping.
- Colors and visual tokens: production pixels are restricted to `#0B0D0C`, `#181B19`, `#167B43`, `#2FCB63`, `#E0C27A`, `#2BA7D7`, and `#F7F8F2`; dark and light backgrounds both retain sufficient separation.
- Image quality and asset fidelity: the 540 x 916 RGBA asset is sharp at device size, has transparent corners, and shows no visible chroma-key halo in either theme.
- Copy and content: unchanged.

## Comparison History

- Initial P2: soft chroma-key removal made the sand hazards partially transparent, causing dark-mode color drift and inconsistent light-mode rendering.
- Fix: regenerated the alpha matte with a hard key, preserved opaque artwork pixels, and rebuilt the resource.
- Post-fix evidence: `dark-v3.png` and `light-v3.png` show opaque sand and consistent geometry.
- Follow-up P2: generated greens approximated the app palette rather than using its actual tokens.
- Fix: quantized every visible artwork pixel to the seven OpenCaddie theme colors and rebuilt the resource.
- Post-fix evidence: `dark-v4.png`, `light-v4.png`, and `qa-comparison-v4.png` show the exact palette in both themes.

## Findings

No actionable P0, P1, or P2 differences remain.

## Follow-up Polish

- P3: a small upper-right green facet retains intentional painterly speckling from the generated cubist source.

## Implementation Checklist

- [x] Bundle the production artwork in the QML resource module.
- [x] Replace only the Welcome screen's decorative course rendering.
- [x] Build the native app and pass `opencaddie_qmllint`.
- [x] Capture and inspect dark and light 800 x 480 states.

final result: passed

---

# Design QA: Club Rack Bag Screen

## Evidence

- Source visual truth: `.context/club-rack-reference.png`
- Implementation: `src/ui/qml/screens/BagScreen.qml` and `src/ui/qml/components/ClubArtwork.qml`
- Dark implementation capture: `.context/club-rack-final-dark-heads.png`
- Light/Norwegian/150% capture: `.context/club-rack-light-nb-150-heads.png`
- Same-input comparison: `.context/design-qa/club-rack-comparison.png`
- Viewport: 800 x 480 logical pixels, captured at 1600 x 960 (`@2x`).
- Normalization: the 1619 x 971 source and 1600 x 960 implementation were both normalized to exactly 800 x 480 before comparison. The rack focus uses the same 800 px scale and the same 240 px crop.
- State: `BagScreen`, dark palette, English, metric units, 100% text scale, recommendations enabled, 7 iron selected at 145 m.

## Full-view Comparison

The upper pair in the combined comparison verifies the complete 800 x 480 screen. The implementation preserves the production top bar and touch-safe quick editor while matching the selected concept's dark equipment rack, bag-at-left composition, six visible clubs, green active state, carry labels, and bottom divider.

## Focused Rack Comparison

The lower pair compares the same rack region at the same scale. The final artwork uses upright three-quarter profile heads with a short shaft above each hosel. The driver, cavity-back iron, wedge, and mallet putter remain visually distinct. The selected 7 iron uses full green colorization, a restrained green glow, green copy, and a green underline. All six clubs fit the initial viewport; larger bags remain horizontally scrollable.

## Required Fidelity Surfaces

- Typography and content: Inter typography, English names, metric carries, and the 7 iron / 145 m state are legible and consistent. The Norwegian 150% stress capture uses intentional ellipsis in narrow rack labels without clipping controls.
- Spacing and layout rhythm: the 204 px rack, compact editor, dividers, and touch targets remain within the fixed 800 x 480 viewport. The narrowed bag slot allows the full putter delegate to remain visible.
- Colors and states: neutral clubs use charcoal, black, and brushed steel; only the active club receives the selected green treatment. Disabled opacity remains independent of selection.
- Image quality: the production PNGs are transparent, tightly fit to their portrait slots, mipmapped, and free of visible magenta keying halos in both light and dark captures.
- Interaction coverage: selection, carry adjustment, recommendations, save, details, remove, and reorder controls remain present; add/edit uses the details sheet and unsaved changes remain guarded.

## Comparison History

- Initial P1: full-length club assets made the heads too small and visually generic in the horizontal rack.
- Fix: retained the original full-club resources and introduced a separate generated head-artwork set.
- Follow-up P1: the first head-only generation used oversized face-on product views instead of the mock's profile-with-short-shaft pose.
- Fix: regenerated every club from the selected mock and existing equipment sources with an upright profile head plus a short shaft segment.
- Follow-up P2: transparent square padding reduced the apparent head size, the bag slot delayed the first club, and the putter clipped at the right edge.
- Fix: tightly cropped each transparent asset, reduced the bag slot to 112 px, fit all six delegates, and added a dedicated cropped bag-with-clubs illustration while preserving the prior bag asset.
- Post-fix evidence: `.context/design-qa/club-rack-comparison.png` shows the final normalized full view and rack crop together.

## Findings

No actionable P0, P1, or P2 differences remain.

## Follow-up Polish

- P3: the production top-bar action and richer details/reorder controls differ from the speculative mock because the implemented screen preserves the app's shared components and the approved functional scope.

## Verification

- [x] Full desktop CMake build.
- [x] All five CTest suites.
- [x] `opencaddie_qmllint`.
- [x] Dark English/metric capture at 100% text scale.
- [x] Light Norwegian/metric capture at 150% text scale.
- [x] Same-input visual comparison at normalized 800 x 480.

final result: passed
