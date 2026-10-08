# Rally action artwork

`action-source-v1.png` was generated with the built-in imagegen tool using
`../pink-ponytail/starter-sheet-v2.png` as the identity reference. The original
pink ponytail, ivory top, plum shorts and white shoes remain. The new sheet
contains five poses in each of two matching front/rear views: strike windup,
forward punch, recovery, rising jump and falling/landing preparation.

Run `python3 tools/import_rally.py` to compile all 30 frames. Action poses use
one shared scale across the sheet, a 32×64 OBJ cell and foot anchor (16,56).
They map to the existing 15 opaque colors plus transparent index 0. The 20
original idle/walk tiles and palette remain byte-identical. Right-facing
banks mirror the corresponding front/rear artwork. `--actions PATH` allows a
replacement source sheet without modifying the approved walking source.

Native action frames are 20–22 (front) and 25–27 (rear). Rising/falling frames
are 23/24 and 28/29. Native attacks play once and hold recovery; idle and walk
still loop. The original controller owns launch, collision, landing and
resource costs. No persistent RAM or save-format fields were added.

Run `python3 tools/test_rally_assets.py` to check the tile encoding, palette,
frame bounds, fixed feet and animation bindings. The native input-only
`tools/tactics_rally_action_smoke.py` checks actual frame selection, attack
loop flags, both strike sequences and full-height jump/landing behavior.
Dedicated hurt/climb/casting art and Rally's own card remain unfinished.
