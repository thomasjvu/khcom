# Aladdin 0.41 development patch

Defeating Jafar offers Aladdin's original character card alongside personal power
and sleight upgrades. Recruit him, then choose him during a subsequent room's
party setup. His original event and friend sprites animate movement and combat.
His targeted Key skill deals 9 + card value + personal power damage and restores
one movement after a successful hit, capped at three. Misses and card breaks
grant no movement. Fire, Cure, Guard and shared sleights remain available.

Recruit-card previews sit beside the party without covering its sprites. The
compact status and party/enemy phase strips remain visible, with detailed controls
only while a menu is open. Party members act in your chosen order.

Format-13 suspend saves store all six identities. Existing format-8 through
format-12 saves import automatically; Aladdin remains locked with fresh resources.
Recorded native format-12 import, native rewrite and reboot preserve old hero,
deck, health and control state.

Patch: `build/release/kh-tactics-0.41-aladdin-dev.bps` (99,656 bytes).
Apply to the US base SHA-1 `10729bd884f8fdca7a310b6d606c52e46657aa48` using
[build instructions](BUILD.md). Applied ROM SHA-256:
`1f52bcd88993a08ef318a9ba72ea19b7c410e181a113d8911e1a040597ca519d`.
Patch application matches the tested build byte for byte.

290 scoped current-ROM native checks pass with verified frontend exits. These
include all six heroes' sleight area/height boundaries, Donald healing/revival,
Goofy spin/Guard, party menus, Aladdin hit/miss/break/recovery caps, both Jafar
reward paths, full-roster corrupt-save fallback and native legacy import. Focused
fixtures explicitly set cards, health, unlocks or positions; they are separate
from the input-only complete campaign.

One complete input-only campaign passes at frame 314381 with stable terminal state, all 36
rooms, nine chests, Cloud/Aladdin recruitment and deployment, suspend/reboot and
composed descent. The three-seed regression is running and has not been certified.
The exact evidence list is `build/release/aladdin-0.41-manifest.json`.
This is a development package; final release audit remains open.
