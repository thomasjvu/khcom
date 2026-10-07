# Play KH Tactics: native party build

Build with `configure.py --tactics` and open `build/tactics-us/kh_tactics.gba`
in mGBA. The ROM uses CoM's native 2.5D field engine, original actors, world
tiles, card pictures, value digits, chests and doors.

| Button | Action |
| --- | --- |
| D-pad | Commit a short movement step; adjacent directions allow diagonals |
| Select | Switch Sora → Donald → Goofy; each retains movement/action budgets |
| L / R | Select the previous/next card in the five-card hand |
| A | Play the selected card from the active member's position |
| B | Jump; combine with a direction for a moving jump |
| D-pad / B while hanging | Climb / drop using native ledge physics |
| L + R | Reload discarded cards, spending the active member's action |
| Start | End the whole party turn; every enemy resolves one decision |
| Start + Select | Suspend while idle; the next boot automatically resumes |
| Select after defeat/run clear | Start a new run with a new seed |

Each member has three movement points and one action per turn. A jump consumes
an action and a directional jump also consumes movement. Holding a direction
commits one command; release and press again for the next. A completely blocked
step refunds its movement point. The party currently shares one 80-HP pool.

The shared deck starts with twelve cards. The hand contains up to five cards;
playing a card discards it and draws a replacement. L+R reloads discarded cards.
Kingdom Key uses the original sword hitbox, Fire strikes the nearest enemy within
range and height limits, Cure heals, and Guard reduces the next enemy phase's
damage. Donald gets a Cure/Fire bonus; Goofy gets stronger Guard. Donald's cast
and Goofy's guard use original animation assets. Zero cards bypass the value
check with weaker melee damage. Values below an enemy's threshold are broken.

Walk through doors to explore twelve generated rooms per world, including side
branches. Strike chests with a Kingdom Key card to open them: each restores
12 HP and adds one seeded card, up to a 24-card deck. Chest flags and room enemy
counts persist. Clear the tougher room-7 encounter and leave through its far
door to advance from Traverse Town to Agrabah and Castle Oblivion.

Suspend records exact party positions/budgets, enemies and their HP/positions,
the shared deck, room progress and seed in two checksummed SRAM slots. A damaged
latest slot falls back to the older valid slot. Defeat/run clear invalidates
suspends. Save files from earlier flat-board or field prototypes are incompatible.

This is still a development build. Physical route guarantees, cursor/path and
intent previews, independent member HP, persistent partial enemy damage when
backtracking, sleights, reward choices, distinct boss sprites/AI, complete
input-only run testing and hardware validation remain unfinished. Native room
creation resets party positions/budgets and surviving enemy HP on room entry.
