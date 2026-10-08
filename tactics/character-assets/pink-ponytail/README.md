# Pink ponytail girl — starter artwork

Latest design: `starter-sheet-v2.png` — lighter pastel pink hair, shorter
bangs and white shoes. Edited with the built-in image generation tool;
the exact edit prompt is in `edit-prompt-v2.txt`. The original draft is retained.
Both versions still require the production conversion steps below.

Original character: rose-pink high ponytail, ivory sleeveless top, plum shorts,
dark ankle boots. Generated with the built-in image generation tool.

`starter-sheet-v1.png` contains 20 proposed idle/walk poses in a 5-column,
4-row layout. This is an art draft, not a ROM-importable sprite sheet.
The two front-facing rows do not reliably provide opposite directions;
correct the direction silhouettes before defining animation frames.

## Local style references

- `assets/us/sprites_sora/sor1fl00.png`: Sora front diagonal poses.
- `assets/us/sprites_sora/sor1bl00.png`: Sora rear diagonal poses.
- `assets/us/sprites_evt/kair_f00.png`: female scale; 32×49 cells,
  anchor (16,43), three columns, native sprite approximately 43 pixels tall.
- `assets/us/sprites_evt/nami_walk_b.png`: female walk proportions.

The asset manifests are in `config/assets/sprites_*.yaml`. The existing
encoder in `tools/sprite_sheet.py` uses indexed PNGs and GBA OBJ pieces.

## Production conversion checklist

1. Redraw/clean the selected poses on a native pixel grid at reference scale.
2. Correct front/back diagonal views and make matching left/right frames.
3. Reduce to a shared 16-entry GBA palette, including transparent index 0,
   and round opaque colors to the GBA 5-bit RGB channels.
4. Place frames in uniform cells with a fixed foot anchor. Do not assume the
   generated image has exact frame boundaries or a uniform pixel grid.
5. Define idle/walk frame timings and valid 8-pixel OBJ pieces in a new
   custom manifest. Keep ponytail pixels inside the declared pieces.
6. Add attack, hurt, jump, climb and other poses needed by the chosen native
   character controller before making this a playable recruit.
7. Wire a separate palette and sprite/animation tables, then verify in mGBA.

No existing character assets, manifests or ROM behavior were replaced.
The prompt is saved in `generation-prompt.txt` for subsequent characters.
