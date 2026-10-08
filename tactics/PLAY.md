# Play KH Tactics 0.41

Apply the [release patch](ALADDIN-0.41.md) to the supported US game, or follow
[build instructions](BUILD.md), then open the resulting ROM in mGBA. Gameplay
uses the original 2.5D field engine: procedural rooms retain native sprites,
scenery, height, collision, doors, chests, climbing and jumping.

Choose your party at room entry, deploy with A, then use Select to open Commands.
Move, Attack, Skills, Party, End Turn and Suspend have contextual menus. B backs
out before confirmation. Party actions can be taken in any order; End Turn
begins the enemy phase and then refreshes the party's turn resources.

## Controls

| Button | Action |
| --- | --- |
| Select | Open Commands |
| D-pad | Commit one short movement step; adjacent directions allow diagonals |
| L + D-pad | Open a walking preview; D-pad selects a destination, A confirms, B cancels |
| R + D-pad while grounded | Face any of eight directions without spending resources |
| L + Select | Cycle living deployed heroes while grounded |
| L / R | Select a card in the five-card hand |
| A | Play the selected card from the controlled hero's position |
| R + B | Cycle eligible Fire/Cure targets, or Cloud/Aladdin sword targets |
| L + A | Stock a card; stock three, release L, then A executes a sleight |
| L + B | Cancel stock without exhausting cards |
| L + R | Reload discarded cards for one action |
| B | Native jump; combine with a direction for a moving jump |
| D-pad while attached to stairs | Climb/descend one level for one movement |
| L + Up/Down while attached | Open a height-route preview |
| R during attached height preview | Toggle the landing-floor cursor for descent plus walking |
| B while attached, outside a preview | Drop using native physics for one action |
| Select while hanging from a ledge | Open Climb/Drop; A confirms, B closes |
| Start | Immediately end the whole party turn |
| Start + Select while idle | Suspend; next boot resumes automatically |
| Select after defeat or run clear | Begin a new run with a new seed |

Holding a direction commits one command. Release and press again to make the
next step. In Commands → Move, R opens Jump confirmation while grounded. In
Skills, Down opens Reload when nothing is stocked; with stocked cards, it
clears the stock first. Commands → Party highlights a deployed hero with
Up/Down; A takes control and B returns without switching.

## Party and turn information

Sora occupies the leader slot. Two companion slots can hold unlocked Donald,
Goofy, Cloud, Rally or Aladdin. During PARTY SETUP, L/R selects a slot and
Up/Down changes its character. Choosing an already deployed companion swaps
the two slots. A/Start deploys and controls the highlighted living hero.
Original character cards identify the party; they are used for assembly and
recruitment rather than combat summons. Resuming an active battle skips setup.

| Hero | HP | Distinct abilities |
| --- | --- | --- |
| Sora | 80 | Original Keyblade swings and field sleights |
| Donald | 56 | Ranged attack magic; Cure and healing sleights gain eight recovery |
| Goofy | 72 | Attack-card shield spin; stronger party Guard |
| Cloud | 72 | Targeted sword slash: 12 + value + power, search radius 64 |
| Rally | 64 | Close strike; Cure and healing sleights gain four recovery |
| Aladdin | 60 | Targeted sword skirmish: 9 + value + power, search radius 48; a hit restores one movement |

Each hero has three movement points and one action per turn. Health, movement,
actions, facing, power and sleight upgrades follow identity when switched or
benched. Switching never refills resources. Aladdin's recovery is capped at
three movement; misses and card breaks grant none. Targeted sword searches
use Manhattan distance strictly below their radius and a 24-pixel height limit.

The normal HUD uses 18 pixels at the top and 16 at the bottom of the 160-pixel
screen. Its strip lists living heroes who can still act, marks the controlled
hero with X, then shows the enemy phase. During enemy resolution, ENEMIES comes
first. This represents free party ordering, not fixed initiative. Charged
attacks add one warning row. Commands use a 40-pixel top panel; detailed menus,
assembly and rewards use 56 pixels. The phase strip remains visible in detailed
Attack, Jump, Party, Reload and End Turn confirmations.

End Turn confirmation shows each deployed hero's HP, movement, action and
expected incoming damage. The forecast uses current positions; later enemies
can retarget if an earlier attack knocks someone out. Sora falling ends the
run. Knocked-out companions cannot take turns; Cure, chest healing or the rest
at a world transition can revive them.

## Movement, height and interactions

Walking previews outline affordable destinations on native terrain. Original
number sprites show route costs. A segment costs one movement for 16 pixels
horizontally, eight pixels in depth, or a combined diagonal. Collision,
occupancy, solid props and current floor height constrain the route. A
revalidates it before commitment. Native physics executes each segment;
completely blocked steps and segments that never start are refunded.

Walking into stairs attaches the original climbing controller. Party switching
waits until landing. End Turn can refresh movement while attached. L+Up/Down
selects up to three height segments; diamonds show affordable heights. R
switches to a floor cursor at the predicted landing so descent and subsequent
walking can be composed into one route. B cancels without dropping or spending
resources. Execution checks the landing again before walking onward.

A standing jump costs one action. A moving jump also costs one movement;
directional input stops after 32 pixels of weighted travel, while the original
controller completes ascent and landing. Jump confirmation shows direction,
cost and a landing diamond when resolved, or an explicit stair attachment,
ledge catch or unresolved forecast. An unresolved forecast does not guarantee
a landing. Original jump-pad physics and native collision remain authoritative.
Walking, ascent, jumping and prop departure retain their separate controls;
attached descent can continue directly into walking.

Face Sora or Rally toward a chest or enemy with R+D-pad, then play a Key card.
Sword confirmation shows damage **on a successful hit**; scenery, facing and
the original animated hitbox still determine contact. Walk through open doors
to travel. Sword swings beside them do not reset the generated room.

## Cards, sleights and rewards

The shared deck starts with twelve cards and draws a hand of up to five.
Playing discards a card and draws a replacement; Reload returns discarded cards
for one action. Zero cards bypass the value check with weaker melee damage.
Insufficient nonzero values break and still spend the card/action.

Fire targets a nearby enemy within its 128-pixel search radius and 24 pixels
of height. Cure targets the eligible member missing the most health, including
knocked-out friends, within 96 pixels and 24 pixels of height. R+B changes the
eligible target; original digits preview actual damage or capped recovery.
Target selection resets on room entry or resume. Guard reduces incoming damage;
Goofy's Guard protects the party through the complete enemy phase.

Three stocked cards preview their combined effect before commitment. The most
frequent type chooses melee, Fire, party Cure or full Guard; ties favor Key,
then Fire, Cure and Guard. Three matching cards unlock TRIPLE KEY/FIRAGA
(+6 damage), CURAGA (+8 party healing) or AEGIS (full Guard). Personal matching
sleight enhancements add four. A sleight spends one action, exhausts its first
card for the run, and discards the other two. Stock survives suspend/resume.
These are custom field tactics recipes.

Every second newly cleared room and every world boss offers a personal reward.
L/R selects a deployed hero, Up/Down selects power or a Key/Fire/Cure sleight
upgrade, and A confirms. Power adds one damage and is capped at eight. Each
sleight enhancement can be granted once per hero. Revisited clears do not
repeat rewards. Pending personal rewards can be suspended with Start+Select.

Traverse Town's optional room-9 Cloud challenge offers his original character
card instead of a personal upgrade. Jafar offers Aladdin's original character
card. Up from POWER selects recruitment; A unlocks the ally. Choose them in a
subsequent room's party setup. The regular reward leaves the ally locked.

Strike chests with a Key card. Choose one of three seeded cards with L/R or
Left/Right and confirm with A. It joins the deck and all deployed heroes recover
12 HP. At the 24-card cap, confirmation grants healing only. Finish the chest
choice before issuing commands or suspending.

## Worlds and bosses

Each world has twelve generated rooms with side branches and reward rooms.
Clear room 7 and leave through its far door to advance through Traverse Town,
Agrabah and Castle Oblivion. Ordinary doors preserve movement/actions and Guard.
World transitions fully heal all recruited heroes, revive the bench and refresh
turn resources while preserving recruitment and upgrades.

| Encounter | Telegraph and response |
| --- | --- |
| Guard Armor, 40 HP | Original multipart body charges a slam. Hand breaks at 26/13 HP reduce reach from 80 to 64 to 48 pixels and damage from 10 to 8 to 6. Move out or Guard. |
| Jafar, 48 HP | Original sorcerer/lamp poses charge a single-target spell: 96 projected pixels, 32 height, 10 damage. Leave its range or Guard. |
| Marluxia, 56 HP | Original windup/scythe poses telegraph a party sweep: 96 horizontal pixels, 16 projected depth, 24 height. Damage rises from 10 to 14 at 28 HP. |
| Cloud, 40 HP | Optional challenger telegraphs a sword sweep: 64 horizontal pixels, 16 projected depth, 24 height. Damage rises from 12 to 16 at 20 HP. |

Regular encounters mix original Shadow, Red Nocturne, Darkball and Black Fungus
roles. Their warning and damage previews update with party positions. Boss
phases are custom tactics rules; Jafar uses his sorcerer form.

## Suspend and verification

Two checksummed 1,024-byte SRAM slots preserve the seed, roster, party positions,
health/budgets/facing, deck/stock, enemy positions and partial damage, visited
rooms, chests, rewards and attached stair state. A damaged newest slot falls
back to the older valid slot. Defeat and run clear invalidate suspends.

Format 13 stores six heroes and imports formats 8–12 automatically. Older saves
initialize previously unrecorded roster resources; Aladdin remains locked.
Format 7 and earlier are incompatible. Current-build checks include an actual
recorded format-12 resume, native upgrade save and second reboot.

See [0.41 notes](ALADDIN-0.41.md) and [release audit](RELEASE-AUDIT.md) for evidence
and verification scope. Focused fixtures and completed campaigns have different
verification scopes; no universal guarantee for every generated seed is claimed.
