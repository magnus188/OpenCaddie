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

- Approved concept reference: `.context/club-rack-reference.png`
- Pre-refinement baseline: `.context/club-rack-final-dark-heads.png`
- Refined dark/English/100% capture: `.context/club-rack-refined-dark.png`
- Refined light/Norwegian/150% capture: `.context/club-rack-refined-light-nb-150.png`
- Normalized before/after comparison: `.context/design-qa/club-rack-refinement-comparison.png`
- Additional fixtures: `.context/club-rack-refined-empty.png`, `.context/club-rack-refined-inactive.png`, `.context/club-rack-refined-overflow.png`, and `.context/club-rack-refined-dark-en-imperial.png`
- Text-scale matrix: dark English and light Norwegian at 80%, 100%, and 150%.
- Viewport: 800 x 480 logical pixels, captured at 1600 x 960 (`@2x`); comparison inputs normalized to exactly 800 x 480.
- Primary state: `BagScreen`, dark palette, English, metric units, 100% text scale, Active enabled, 7 iron selected at 145 m.

## Before/After Comparison

The normalized comparison shows the full approved refinement at one scale. The fixed bag slot is gone and the rack now uses the complete 800 px width. The previous rear/cavity artwork has been replaced only for irons and wedges by transparent, upright striking-face variants with clearly legible horizontal grooves and the same short-shaft composition. The selected club remains fully green with its restrained glow and underline.

The quick editor removes the duplicate carry/status summary, recommendation pill, slider, and cramped action column. Club identity, the one-unit numeric stepper, and Active switch form one clear upper row. Save Changes receives two layout units; Details and More receive one each. More presents 56 px Reorder Bag and destructive Remove Club rows. Reordering now uses a direct long-press and horizontal drag, with one Done control and a live position indicator.

## Required Fidelity Surfaces

- Typography and content: Inter remains legible across both languages and all tested text scales. Norwegian uses “Aktiv” and “Inaktiv”; narrow rack names intentionally ellipsize at 150% without overlapping adjacent content.
- Spacing and touch: every new interactive control is at least 48 px high, action gaps are 12 px, and no control clips at 800 x 480. The action sheet rows are 56 px high.
- Carry editing: tapping the number opens the numeric keyboard and lifts the quick editor so the field remains visible; Done commits the draft; ± moves in one-unit steps; blank input restores the last value; 0 clamps to 1; and 999 clamps to 350 m or 400 yd. The details-sheet carry field also remains above the keyboard.
- Colors and states: selected artwork remains green; neutral faces retain brushed metal; inactive clubs remain visible with reduced artwork opacity and an amber Inactive label.
- Image quality: `iron-face.png` and `wedge-face.png` have transparent, tightly cropped edges, correct face orientation, visible grooves at device size, and no chroma-key halo in either theme. All previous artwork files remain bundled and untouched.
- State handling: first-club selection, empty/Add Club state, overflow scrolling, Active persistence, More dismissal, remove confirmation, drag target boundaries, and unsaved-change guards were exercised in the live app. During a drag, the held card scales to 108%, gains a raised shadow/green outline, and adjacent cards animate into their new slots before drop persistence.

## Comparison History

- P1: the iron and wedge showed the back/cavity instead of the striking face, so their grooves were absent or visually incorrect.
- Fix: generated dedicated iron-face and wedge-face assets with front striking surfaces and crisp horizontal score lines; `ClubArtwork.qml` alone redirects those two types.
- P1: the bag illustration consumed the first rack slot and reduced the useful scrolling area.
- Fix: removed only its rendering and anchored the horizontal list from edge to edge; the bag assets remain in the repository.
- P2: the slider made exact one-unit carry changes cumbersome and repeated the carry value in two places.
- Fix: introduced a direct-entry number stepper with 48 px ± buttons, numeric keyboard support, restoration, and clamping.
- P2: recommendation copy and the vertical action stack made the editor dense.
- Fix: renamed the switch to Active, removed the status pill, widened primary actions, and moved reorder/remove into a modal More sheet.
- Follow-up P2: left/right reorder buttons were slower than direct manipulation and did not preview the resulting order.
- Fix: replaced movement buttons with long-press drag-and-drop, a local live-order model, animated displaced cards, edge auto-scroll, and a lifted drag card; the final order is persisted on drop.
- Follow-up P1: the on-screen keyboard covered the quick carry field.
- Fix: the quick editor now animates upward while its numeric field has focus; both quick and details carry inputs remain fully visible.

## Findings

No actionable P0, P1, or P2 differences remain.

## Verification

- [x] Full desktop CMake build.
- [x] All five CTest suites.
- [x] `opencaddie_qmllint`.
- [x] Direct entry, keyboard Done, ±, blank restoration, metric/imperial lower and upper clamping, and draft-only persistence.
- [x] Active save/restore, Details, More, remove confirmation, live drag reflow/boundaries, and unsaved-change guard.
- [x] Quick-editor and details-sheet carry fields remain visible with the numeric keyboard open.
- [x] Empty, inactive, and 11-club overflow fixtures.
- [x] Metric and imperial units.
- [x] Dark English and light Norwegian at 80%, 100%, and 150%.
- [x] Same-input before/after comparison at normalized 800 x 480.

final result: passed
