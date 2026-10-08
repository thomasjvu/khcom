# Maho and Rally combat drafts

Rally is the pink-ponytail character in `pink-ponytail/`.
Both sheets were generated with the built-in image tool using their existing
character sheets as identity references. Exact prompts are saved beside PNGs.

## Layout

Each sheet has five rows and six columns:

1. Front-right attack: ready, windup, strike preparation/strike,
   extended strike, follow-through, recovery.
2. Back-right attack sequence.
3. Front-right magic: ready, raise implement, charge, release, sustain, recover.
4. Back-right magic sequence.
5. Six separate spell-effect concepts.

Maho: `maho/combat-sheet-v1.png`, star-tipped wand, cyan/gold charge,
projectile and impact effects.

Rally: `pink-ponytail/rally-combat-sheet-v1.png`, pink/white pompom attacks,
white/plum megaphone casts, pink sound waves and rally sparkles.

## Intended timing for production

Attack frames: 100, 100, 70, 70, 100, 160 ms.
Cast frames: 100, 140, 200, 100, 180, 160 ms.
These are proposed timings, not implemented animation metadata.
Charge, projectile, impact and fade art represent different effect phases;
they should be separate animations rather than one looping six-frame effect.

## Import status

These are pose/effect art drafts, not playable ROM animations.
Some effects are attached to character poses; extract/redraw them as separate
OBJ effects so the actor and spell can use independent palettes and timing.
Check weapon-hand consistency, native pixel readability, left-facing views,
frame separation, indexed palette, OBJ bounds and foot anchors before encoding.
Rally's sheet also has visible fringe/noise around silhouettes requiring cleanup.
Follow the conversion checklist in `pink-ponytail/README.md`.
