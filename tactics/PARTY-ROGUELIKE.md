# Party tactics and recruitment requirements

User direction, 2026-10-07. These requirements extend the full polish goal.

Each room encounter is a round; each world boss ends a set of rounds. This is
an implementation assumption until corrected by the user.

- Control one selected deployed character at a time; switch freely among
  eligible members while preserving individual position, movement and action.
- Give every character its own moveset and original sprite animations. Donald
  uses magic/healing, Goofy shield attacks/protection, Sora Keyblade/sleights.
- Add a recruitable roster, starting with Cloud, whose original battle sprites
  and summon card are present. Fight him before recruitment. Confirm the actual
  asset inventory before adding Aerith, Tifa or other candidates.
- Summon cards belong to round-start party assembly, not in-combat summon
  attacks. Select a deployed party from unlocked character cards before each
  encounter, then commit deployment and start tactical play.
- Show the selected character's reachable movement tiles on the original 2.5D
  geometry, including occupied spaces, height, climb and jump connections.
- Offer boss rewards: a recruit/summon card for appropriate character bosses,
  or a regular roguelike powerup. Every other encounter offers a sleight unlock
  or small upgrade assigned to a party member.
- Persist roster, deployment, unlocks and character upgrades in suspend saves;
  validate all assembly/reward transitions and full-run reset/recruitment.

## Current implementation (0.41)

The six-hero roster contains Sora, Donald, Goofy, Cloud, Rally and Aladdin. Each
has independent health, turn resources, facing and personal upgrades. Sora uses
original sword contact, Donald ranged magic/stronger healing, Goofy spin/party
protection, Cloud targeted sword strikes, Rally close strikes/healing, and
Aladdin targeted skirmishes that recover movement after a resolved hit.

Original character cards assemble the party at room start. Cloud is fought in
an optional Traverse Town challenge and recruited instead of a personal upgrade.
Jafar offers Aladdin's original character card as an alternative boss reward.
Personal power/sleight rewards occur every second unique room clear and at bosses;
revisited encounters do not duplicate rewards. Sora remains leader, but any living
deployed hero can be controlled. Benching and switching preserve resources.

Walking, stairs and jump confirmations display native terrain-space previews.
Attached descent can continue into a walking route. The compact HUD lists ready
party members followed by the enemy phase; detailed menus appear contextually.
Six-hero format-13 saves import older formats and preserve roster/deployment,
health, resources, upgrades, facing and partial encounters.

Current controls and exact mechanics are in [PLAY.md](PLAY.md). Version-scoped
verification and remaining gates are in [RELEASE-AUDIT.md](RELEASE-AUDIT.md).
Historical implementation/test milestones remain in [PLAN.md](PLAN.md).
