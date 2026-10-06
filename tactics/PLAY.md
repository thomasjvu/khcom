# Play the native field prototype

Build with `configure.py --tactics` and open `build/tactics-us/kh_tactics.gba`
in mGBA. This version boots directly into a generated Traverse Town room.
The original full sprites and world art render through CoM's field engine.

| Button | Action |
| --- | --- |
| D-pad | Commit a short movement step; adjacent directions allow diagonal steps |
| A | Use the original sword animation to hit a field enemy, chest or breakable |
| B | Jump; combine with a direction for a moving jump |
| D-pad while hanging | Continue climbing a ledge |
| B while hanging | Drop from the ledge |
| Start | End the turn and let enemies act |
| Select after defeat/run clear | Start a new run with a new seed |

Each turn allows three movement steps and one attack/jump. Holding a direction
commits one movement command; release and press again to move farther.
Original collision and height checks apply. Jumping with a direction also
spends one movement point. A ledge climb continues the committed jump.

Walk through open doors to explore the generated room graph. Optional rooms
contain chests; strike a chest to open it and restore 12 HP, once per room.
Enemies are seeded when entering a room, remain still while you choose actions,
and act during the enemy phase. A sword hit currently defeats a field enemy;
enemy contact deals 4 HP damage at most once per enemy per phase. HP persists
between rooms. Clear room 7 and leave through its far door to advance worlds.
After Traverse Town, Agrabah and Castle Oblivion, the run-clear state appears.

This is a field integration prototype. Card selection, card values and breaks,
sleights, tactical intent previews, bosses, reward choices and suspend saves
from the old flat-board prototype are not yet connected to this version.
The old board renderer is not part of the current ROM build.
