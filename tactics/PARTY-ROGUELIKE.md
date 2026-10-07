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

## Current implementation

Sora, Donald and Goofy are individually controllable with separate movement
and action budgets. Donald uses ranged magic and stronger healing; Goofy uses
shield spins and protection. Cloud uses a targeted sword slash and his original
battle animations. Character power and matching-sleight upgrades apply in combat.

Round setup shows original character cards. L/R selects a deployed slot;
Up/Down changes an unlocked companion; A/Start confirms. Sora remains required.
Companion swaps preserve each character's health and spent turn resources.
Summon cards appear in setup and recruitment; combat Guard uses the original
Guard Armor enemy card. Walking previews show reachable terrain tiles. Climb
and jump connections still need integration into the reachable overlay.

Traverse Town's optional room-9 Cloud challenge offers his recruitment card or
a personal power/sleight upgrade. Every second unique room clear and world boss
also offers a personal upgrade; revisits cannot duplicate rewards. Cloud can
replace Donald or Goofy in either companion slot after recruitment.

Suspend format 10 stores roster identities, deployment, personal upgrades,
bench health and turn resources. Recorded format-8 and format-9 saves migrate.
Sixteen native deployment checks verify locked-character rejection, controller
selection, original art, Cloud damage, bench health/action and reboot recovery.
Both Cloud challenge reward choices have fifteen native fixture checks.

A fresh-SRAM input-only run using the starter party completes all three worlds,
including suspend/reset with the complete roster comparison: victory at frame
108366, stable terminal PASS at 108486, 14 kills and 899 movement commands.
Evidence: `build/tactics-us/cloud-party-map-suspend-full-run-evidence`.
Fixtures are distinct from full-run proof. An additional fresh input-only replay fights and recruits Cloud, deploys him,
saves at frame 13411 and verifies the complete resumed roster at 13741. It
completes all three worlds with 15 kills and 777 movement commands, stable PASS
at frame 92877. Evidence: `cloud-recruit-resume-full-run-evidence`. Cloud and
Goofy are KO later in the run; this proves route completion and persistence,
not balanced companion survival or a Cloud-led combat strategy.

World transitions now rest all recruited heroes, including the bench: full
health, revival, three movement points and one action. Rest preserves identities,
recruitment and upgrades, requires assembly phase, and does not occur on ordinary
room travel. Host checks and native build pass. A fresh input-only Cloud-route replay
verifies suspend/reset and full deployed HP (80,72,72) at both later world
entries. It wins at frame 93534 with Sora HP80 and reaches stable PASS at
93654: 15 kills, 769 movement commands. Evidence:
`world-rest-cloud-full-run-evidence`. A two-run replay using native retry to
advance the seed is in progress in `two-seed-rest-cloud-evidence`.

## Remaining scope

- Verify more party compositions, companion survival and character-led combat.
- Integrate height traversal into reachable movement previews and verify physical
  procedural-room connectivity across seeds and optional branches.
- Improve character loadouts and tactical guidance. Capped power and owned
  sleights have readable warnings; seven native checks verify rejection preserves
  the pending reward and an available choice applies once. Screenshots and log:
  `build/tactics-us/reward-availability-evidence`.
- Add further recruitable characters after checking original assets. Aerith's
  field sprites (`gEarF00`, `gEarB00`, `gEarF01`, `gEarB01`, `gEarisPalette`)
  and portrait card (`gCardNpcEx06`) exist in the decomp. No Tifa assets have
  been located; neither character is currently recruitable.
- Broaden encounter/content coverage and package a verified current BPS patch.
  The existing 0.16 patch predates party assembly and recruitment.
