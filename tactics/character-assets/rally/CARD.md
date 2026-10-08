# Rally assembly card

Original artwork generated with the built-in imagegen tool on 2026-10-08.
Identity reference: tracked pink-ponytail/starter-sheet-v2.png. Requested one
square GBA pixel-art emerald/gold recruitment card, Rally bust with high pink
ponytail/black tie/white sleeveless top, pale-gold star, transparency, no text.
Source is retained unchanged in card-source-v1.png. Deterministic compilation
with `python3 tools/import_rally_card.py` crops its alpha bounds, samples32x32,
quantizes15 opaque colors and emits512 bytes of4bpp OBJ tiles plus16-entry
palette and one native sprite definition. Manifest records the source hash.

Native assembly borrows the same combat-card OBJ banks as other recruits.
The card is freed on cycling/deployment/exit; no extra persistent RAM or
in-battle summon action. The original field sprite remains unchanged.
Current-ROM native Rally setup/commands/deploy/suspend/reset20 checks pass;
setup capture inspected in build/tactics-us/rally-card-menu-evidence.
Complete campaigns on0.32 predate this presentation-only change; their exact
ROM hashes remain distinct and are not new-ROM campaign evidence.
