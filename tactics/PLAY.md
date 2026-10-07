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
| R + B with Fire or Cure selected | Cycle eligible enemies or party members without spending cards or turn budgets |
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
Cure, a chest, or the rest after a world boss revives them. The footer shows all three health pools and a NEXT damage estimate for
each member before ending the turn. This estimate uses current positions;
later enemies can retarget if an earlier attack knocks out a member.

Walking previews show movement costs using the original card-value digits on
native field surfaces. A route charges one point per 16-pixel horizontal or
8-pixel vertical segment, or one combined 16-by-8-pixel diagonal. Diagonal
segments use native diagonal movement and check the floor footprint at quarter
intervals. The planner exposes only routes within the remaining movement
budget; a destination without an affordable route cannot be confirmed.
The original player controller walks each segment; it stops if
collision prevents progress and refunds segments that never started. Preview
walking stays on the current floor level, allows safe upper ledges above void,
and avoids occupied destinations and solid props. Preview costs update as
native prop colliders change. Unchanged previews are cached to keep cursor
input responsive; A always revalidates collision before committing movement.
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
branches. Ordinary door travel preserves the current party turn.
Backtracking gives no free movement, actions or Guard reset. Completing a world
boss and entering the next world fully heals all recruited characters, including
benched heroes, and refreshes movement/actions. Strike chests with
a Kingdom Key card to open them. Choose among three seeded cards with L/R or
Left/Right, then confirm with A. The selected card joins the deck and every
party member heals 12 HP. At the 24-card limit, A grants healing only. Finish
the reward choice before using field commands or suspend. Confirmed cards,
healing and opened chests survive suspend/resume; an unconfirmed choice has
not been saved. Chest flags and room enemy
counts persist. World groups mix Shadow, ranged Red Nocturne, Darkball and
Black Fungus. Regular cohorts use two enemy palettes per room, with room seeds varying the pair. Optional props use a limited palette budget so party, card art and value digits can remain visible. Traverse Town room 7 has a solo Guard Armor with 40 HP, rendered
using its original seven animated sprite components. A charged slam raises its hands and crouches; impact lowers the hands before returning to idle. Its warned slam reaches
80 pixels for 10 damage. At 26 HP the far hand breaks, reducing reach to 64 pixels and damage to 8. At 13 HP both hands are gone, leaving a 48-pixel body strike for 6 damage. The warning names the current attack and reach. These are custom tactics phases. Agrabah now has a solo sorcerer Jafar with 48 HP, using his original field and lamp artwork. He charges a single-target spell reaching 96 projected pixels and 32 pixels of height for 10 damage; equally reachable targets prefer Sora, then Donald, then Goofy. Moving out of reach or playing Guard defends against it. This is a custom tactics form, rather than a port of the original giant Genie battle. Castle Oblivion has solo humanoid Marluxia with 56 HP, using original battle idle, windup and scythe attack animations. Within 112 projected pixels and 32 pixels of height he spends a decision charging. His next decision sweeps all living members within 96 horizontal pixels, 16 pixels of projected depth and 24 pixels of height. The sweep deals 10 damage, increasing to 14 at 28 HP or less. Move out during the warning or play Guard. Incoming damage uses original
value digits above threatened characters as well as the NEXT footer. The
warning is based on current positions; it updates as the party moves.

Clear the room-7 encounter and leave through its far
door to advance from Traverse Town to Agrabah and Castle Oblivion.

Suspend records exact party positions/budgets, enemies and their HP/positions,
the shared deck, individual HP, every visited room encounter and seed in
two checksummed 1,024-byte SRAM slots. Partially damaged enemies keep their
health and positions when you backtrack. A damaged
latest slot falls back to the older valid slot. Defeat/run clear invalidates
suspends. This source writes save format 10 (including bench health and budgets) and reads formats 8/9 to preserve enemy identity, charged attacks and the active
stair controller’s target/facing. Format 8 initializes the starter roster; format 7 and earlier saves are incompatible.

This is still a development build. Physical room reachability guarantees, climb/jump routes and
full area/range overlays, richer rewards, additional boss moves, complete
input-only run testing and hardware validation remain unfinished. Native room
creation gathers the party at the entry door, preserving the selected member,
health, remaining movement/actions and Guard. Only ending a turn renews budgets.

Open doors belong to the generated run. Sword swings beside them keep the room intact; walk through the doorway to travel.

While attached to native stairs, hold L and press Up or Down to preview the
next sixteen-pixel vertical segment. Further Up/Down presses move the selected
height, allowing a route of up to three segments within the remaining movement
budget. Original value digits show the total cost; reachable diamonds mark the
affordable heights. A confirms and the original controller animates each
segment; B cancels without dropping. Outside a preview, B retains the normal
drop action. Climb routes still use a separate preview from walking routes.

Selecting Cure names its intended party member in the HUD before play. It chooses the nearby member missing the most HP, including knocked-out friends; height and range still constrain healing.

Selecting Fire projects the expected HP loss above its nearest valid enemy using original number sprites. The HUD says FIRE NO TARGET when none is in range. The projection accounts for card-value breaks, Donald’s bonus and remaining enemy HP.

Fire displays FIRE CARD BREAK when its selected value will be broken by the current floor’s enemy threshold. Playing it still spends the action and card; the projected HP loss is zero.

Donald and Goofy use original walking sprites while moving and return to idle
when stopped. Casting and Guard keep their action poses. Original airborne poses follow
native jump rise/fall states. Active party sprites stay at their physical
positions; dedicated climb poses remain unfinished.

Living Donald and Goofy cast original-game shadows on their current standing
surface. During jumps, the body rises while the shadow stays on the ground.

Fire defaults to the nearest enemy within 128 world-space pixels and 24 pixels
of height. R+B selects another eligible enemy; the projected damage digits
follow that choice, and A attacks the same enemy. An invalid choice falls back
to the nearest eligible enemy. Target choice resets on room entry or resume.

Cure normally chooses the nearby member missing the most HP. R+B chooses a
specific nearby member, including a knocked-out friend; the named HUD target
and actual healing use the same choice. A member outside 96 world-space
pixels or 24 pixels of height cannot remain selected. Cure target choice is
transient and resets on room entry or resume.

Cure projects original value digits above its chosen recipient to show actual
HP recovery, capped at missing HP. Donald’s casting bonus is included. A
full-health target shows zero; a knocked-out target shows revival HP.

With three cards stocked, attack sleights project damage digits above each
enemy they will hit. The preview uses the attack’s area and height limits and
caps displayed damage at the enemy’s remaining HP.

Attached-stair previews label A as CLIMB or DESCEND according to the chosen
direction; B cancels the preview without dropping. Budget and blocked-route
warnings take priority over the confirm prompt.

Three matching card types unlock named field recipes: TRIPLE KEY and FIRAGA
add 6 damage to the usual attack sleight; CURAGA adds 8 party healing. AEGIS
labels the existing full-Guard sleight and grants the same protection as mixed
Guard stocks. These are custom field rules, not reproductions of the original
battle recipes. Mixed stocks retain their majority-type effect. The first
card is still exhausted and the action cost remains one.

Cure and Curaga sleights preview capped healing above all three party
positions, including revival HP for knocked-out friends.

When the active member has spent their action, the HUD says ACT SPENT START
TURN. Start ends the party turn; you may still use remaining movement or
switch to another available member before doing so.

Donald plays attack cards as ranged magic (128-pixel range, 24-pixel height);
R+B cycles targets. Goofy plays attack cards as a shield spin hitting every
enemy within 48 world pixels and 24 pixels of height. Original number sprites
preview damage. Sora retains native Keyblade swings. These are the first distinct
character attacks; party assembly, recruitable character cards and upgrades are
being implemented according to `PARTY-ROGUELIKE.md`.

At room entry, ROUND SETUP shows the starter cards. L/R selects the starting
hero; A or Start confirms. Confirmation changes control without spending the
hero's action. Combat and movement wait for confirmation. Resuming an active
battle preserves it and skips setup. This is the initial starter-leader screen;
custom roster deployment and recruitment remain unfinished.

Every second newly cleared encounter offers a character reward, and room-7
bosses always offer one. L/R chooses Sora, Donald or Goofy; Up/Down chooses
POWER PLUS 1 or a personal Key/Fire/Cure sleight enhancement; A confirms. Power
adds one to that hero's card damage. Sleight enhancements add four to matching
sleight damage or healing. Each enhancement can be granted once per hero; power
is capped at eight. Revisited cleared rooms cannot grant another reward.
Pending rewards may be suspended with Start+Select. Finish the reward before
leaving the room. Recruit-card rewards and custom roster deployment remain work.

Walking preview now outlines all reachable destinations on the original
terrain within the selected hero's remaining movement. The cursor and numbered
route use the same collision, occupancy and cost rules. Spent movement removes
these outlines. This local walking overlay covers the current surface; stair
and jump connections still use their separate controls and previews.

Traverse Town's optional room 9 now has a solo 40-HP Cloud challenge. His warned
sword sweep spans 64 horizontal pixels, 16 projected depth and 24 height. It
deals 12 damage, rising to 16 at 20 HP or below. Defeat offers a normal power/
sleight reward or Cloud's original summon card. At the reward, Up from POWER
selects recruitment; A unlocks Cloud in the saved roster. Choosing regular loot
leaves him locked. This unlock does not yet make Cloud deployable: character-slot
mapping and the recruited party-selection UI still require implementation.

Recruited Cloud is now deployable. In ROUND SETUP, L/R chooses a slot; Up/Down
cycles its unlocked companion card. Sora remains required in slot 0. Donald,
Goofy and Cloud can occupy either companion slot; choosing an already deployed
hero swaps the companions. A/Start confirms and controls the selected living
hero. Select cycles living deployed heroes during combat.

Health, movement, actions, power and sleight upgrades follow character identity
when benched or swapped. Switching slots does not refill resources. Ending a
turn refreshes movement/actions for the roster. Cloud's attack card is a targeted
sword slash within 64 world pixels and 24 pixels of height, for 12 plus card
value and personal power. R+B cycles targets. Donald retains magic/healing
bonuses and Goofy shield/protection behavior in either slot.

Summon cards appear in setup and recruitment. Combat Guard now uses the original
Guard Armor enemy-card artwork, rather than Goofy's summon card. Format-8/9
saves migrate with default health/budgets for previously unrecorded benched
heroes and preserve saved deployed party state.

Attached stair previews outline affordable vertical destinations. Up/Down moves
the selected height; A commits the displayed movement cost; B cancels the
preview. With no movement left, no reachable outlines appear. The final descent
marker includes the supporting floor and original controller's landing offset.
The HUD shows OUT OF REACH for an unaffordable or blocked destination, with
ROUTE X instead of a misleading zero cost. Selecting the origin spends nothing.

On the ground, R + D-pad faces the active character in any of eight directions
without moving or spending movement/action points. It preserves the selected
card. Cancel a movement preview with B before facing. Use this to aim Sora's
Keyblade at a chest or nearby enemy without walking past it.

Current source preserves each hero's facing when switching, benching, traveling
between rooms, or resuming a format-11 suspend. Older saves migrate with each
hero facing up; attached stair saves retain their recorded climb direction.
The packaged 0.19 development patch predates per-hero facing persistence.
