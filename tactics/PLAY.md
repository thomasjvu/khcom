# Play KH Tactics: native party build

Build with `configure.py --tactics` and open `build/tactics-us/kh_tactics.gba`
in mGBA. The ROM uses CoM's native 2.5D field engine, original actors, world
tiles, card pictures, value digits, chests and doors.

| Button | Action |
| --- | --- |
| D-pad | Commit a short movement step; adjacent directions allow diagonals |
| L + D-pad | Open a projected walking route preview; D-pad moves the cursor, A confirms, B cancels |
| Select | Switch Sora → Donald → Goofy; each retains movement/action budgets |
| L / R | Select the previous/next card in the five-card hand |
| L + A | Stock the selected card; stock three, then release L and press A for a sleight |
| L + B | Return stocked cards to the deck without exhaustion |
| A | Play the selected card from the active member's position |
| B | Commit a full-height native jump; combine with a direction for a moving jump |
| D-pad on stairs | Climb/descend one 16-pixel level for one movement point |
| B on stairs | Drop using native physics, spending the action |
| D-pad / B while hanging | Climb / drop using native ledge physics |
| L + R | Reload discarded cards, spending the active member's action |
| Start | End the whole party turn; every enemy resolves one decision |
| Start + Select | Suspend while idle; the next boot automatically resumes |
| Select after defeat/run clear | Start a new run with a new seed |

Each member has three movement points and one action per turn. A jump consumes
an action and a directional jump also consumes movement. Holding a direction
commits one command; release and press again for the next. A completely blocked
step refunds its movement point. Sora has 80 HP, Donald 56 and Goofy 72. Sora
falling ends the run. Knocked-out friends are skipped by selection until
Cure or a chest revives them. The footer shows all three health pools and a NEXT damage estimate for
each member before ending the turn. This estimate uses current positions;
later enemies can retarget if an earlier attack knocks out a member.

Walking previews show movement costs using the original card-value digits on
native field surfaces. A route charges one point per 16-pixel horizontal or
8-pixel vertical segment, or one combined 16-by-8-pixel diagonal. Diagonal
segments use native diagonal movement and check the floor footprint at quarter
intervals. Routes beyond the remaining budget cannot be
confirmed. The original player controller walks each segment; it stops if
collision prevents progress and refunds segments that never started. Preview
walking stays on the current floor level, allows safe upper ledges above void,
and avoids occupied destinations and solid props. Preview costs update as
native prop colliders change.
Entering stairs through a movement step attaches the native climbing
controller. Holding a direction still commits only one segment. You can end
a turn while attached to recover movement; Select is disabled until landing
so another party member cannot inherit the stair controller’s target.
A B press commits the full native ascent even if released immediately. A moving
jump spends one action and one movement point; its directional input stops at
32 pixels of weighted world-space travel (horizontal distance plus twice depth).
Native collision and landing remain authoritative. With no movement left, B
performs a stationary jump. Catching stairs hands off to the budgeted stair
controller and clears the airborne flag.

Use the existing B jump/climb controls to cross other height changes; jump and climb
edges are not yet part of the route cursor.

The shared deck starts with twelve cards. The hand contains up to five cards;
playing a card discards it and draws a replacement. L+R reloads discarded cards.
When three cards are stocked, the HUD previews their combined value and effect
(KEY, FIR, CUR, or GRD) before A commits the sleight. Inspecting this preview
does not spend cards or an action.

A sleight combines three card values. The most frequent type chooses the
result (ties favor Kingdom Key, then Fire, Cure and Guard): an area melee
attack, an area Fire attack, party-wide Cure, or full Guard. It costs one
action. The first stocked card is exhausted for the run; the other two enter
discard. Stocked cards use original card pictures and survive suspend/resume.

Kingdom Key uses the original sword hitbox, Fire strikes the nearest enemy within
range and height limits, Cure heals the most injured nearby member (including knocked-out friends), and Guard reduces the next enemy phase's
damage. Donald gets a Cure/Fire bonus; Goofy gets stronger Guard. Donald's cast
and Goofy's guard use original animation assets. Zero cards bypass the value
check with weaker melee damage. Values below an enemy's threshold are broken.

Walk through doors to explore twelve generated rooms per world, including side
branches. Door travel and world advancement preserve the current party turn.
Backtracking gives no free movement, actions or Guard reset. Strike chests with
a Kingdom Key card to open them. Choose among three seeded cards with L/R or
Left/Right, then confirm with A. The selected card joins the deck and every
party member heals 12 HP. At the 24-card limit, A grants healing only. Finish
the reward choice before using field commands or suspend. Confirmed cards,
healing and opened chests survive suspend/resume; an unconfirmed choice has
not been saved. Chest flags and room enemy
counts persist. World groups mix Shadow, ranged Red Nocturne, Darkball and
Black Fungus. Room 7 includes a Large Body guardian with 40/48/56 HP. Within
96 pixels it spends a decision charging, then on its next decision strikes
all party members within a 64-pixel Manhattan radius and 24 pixels of height.
Move out during the warning or play Guard. Incoming damage uses original
value digits above threatened characters as well as the NEXT footer. The
warning is based on current positions; it updates as the party moves.

Clear the room-7 encounter and leave through its far
door to advance from Traverse Town to Agrabah and Castle Oblivion.

Suspend records exact party positions/budgets, enemies and their HP/positions,
the shared deck, individual HP, every visited room encounter and seed in
two checksummed 1,024-byte SRAM slots. Partially damaged enemies keep their
health and positions when you backtrack. A damaged
latest slot falls back to the older valid slot. Defeat/run clear invalidates
suspends. This build uses save format 8 to preserve enemy identity, charged attacks and the active
stair controller’s target/facing. Format 7 and earlier saves are incompatible.

This is still a development build. Physical room reachability guarantees, climb/jump routes and
full area/range overlays, reward choices, canonical bosses, complete
input-only run testing and hardware validation remain unfinished. Native room
creation gathers the party at the entry door, preserving the selected member,
health, remaining movement/actions and Guard. Only ending a turn renews budgets.

Open doors belong to the generated run. Sword swings beside them keep the room intact; walk through the doorway to travel.

While attached to native stairs, hold L and press Up or Down to preview the
next sixteen-pixel vertical segment. The original value digit marks the
vertical destination. A confirms for one movement point; B cancels without
dropping. Outside a preview, B retains the normal drop action. These are
single-segment previews, not combined walking/climbing routes.

Selecting Cure names its intended party member in the HUD before play. It chooses the nearby member missing the most HP, including knocked-out friends; height and range still constrain healing.

Selecting Fire projects the expected HP loss above its nearest valid enemy using original number sprites. The HUD says FIRE NO TARGET when none is in range. The projection accounts for card-value breaks, Donald’s bonus and remaining enemy HP.

Fire displays FIRE CARD BREAK when its selected value will be broken by the current floor’s enemy threshold. Playing it still spends the action and card; the projected HP loss is zero.

Donald and Goofy use original walking sprites while moving and return to idle
when stopped. Casting and Guard keep their action poses. Original airborne poses follow
native jump rise/fall states. Active party sprites stay at their physical
positions; dedicated climb poses remain unfinished.
